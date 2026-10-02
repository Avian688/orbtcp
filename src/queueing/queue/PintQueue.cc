//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//

#include "PintQueue.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

#include "inet/common/PacketEventTag.h"
#include "inet/common/TimeTag.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader_m.h"

#include "../../common/IntTag_m.h"
#include "../../common/OmegaPriceTag.h"
#include "../../common/PintFlowCount.h"
#include "../../common/PintQueueingDelay.h"
#include "../../common/PintSenderTelemetry.h"

namespace inet {
namespace queueing {

Define_Module(PintQueue);

static Ptr<IntTag> getAttachedIntTagForUpdate(const Ptr<tcp::TcpHeader>& tcpHeader)
{
    // INET 4.5 may return the pre-detach region tag on the first COW lookup;
    // reacquire it so updates target the tag attached to this TCP header.
    tcpHeader->addTagIfAbsent<IntTag>();
    return tcpHeader->addTagIfAbsent<IntTag>();
}

simsignal_t PintQueue::avgRttSignal = cComponent::registerSignal("avgRttQueue");
simsignal_t PintQueue::numberOfFlowsSignal = cComponent::registerSignal("numberOfFlows");
simsignal_t PintQueue::effectiveNumberOfFlowsSignal = cComponent::registerSignal("effectiveNumberOfFlows");
simsignal_t PintQueue::numOfFlowsInInitialPhaseSignal = cComponent::registerSignal("numOfFlowsInInitialPhase");
simsignal_t PintQueue::persistentQueueingDelaySignal = cComponent::registerSignal("persistentQueueingDelay");
simsignal_t PintQueue::bandwidthSignal = cComponent::registerSignal("bandwidth");
simsignal_t PintQueue::txBytesSignal = cComponent::registerSignal("txBytes");
simsignal_t PintQueue::pintLocalUtilizationSignal = cComponent::registerSignal("pintLocalUtilization");
simsignal_t PintQueue::pintDecodedUtilizationSignal = cComponent::registerSignal("pintDecodedUtilization");
simsignal_t PintQueue::pintEncodedPowerSignal = cComponent::registerSignal("pintEncodedPower");

PintQueue::~PintQueue()
{
    if (priceTimer != nullptr)
        cancelAndDelete(priceTimer);
    if (measurementTimer != nullptr) {
        if (measurementTimer->isScheduled())
            cancelEvent(measurementTimer);
        delete measurementTimer;
    }
}

void PintQueue::initialize(int stage)
{
    PacketQueue::initialize(stage);

    if (stage != INITSTAGE_LOCAL)
        return;

    fallbackBandwidthBitsPerSecond = par("fallbackBandwidth").doubleValue();
    fixedAvgRtt = par("fixedAvgRTTVal");
    pintInitialRtt = par("pintInitialRtt");
    pintNoAverageRttInterval = par("pintNoAverageRttInterval");
    pintUseAverageRtt = par("pintUseAverageRtt");
    flowCountSketchEnabled = par("flowCountSketchEnabled");
    flowCardinalityBits = par("flowCardinalityBits");
    flowSketchSeed = static_cast<uint64_t>(par("flowSketchSeed").intValue());
    pintBits = par("pintBits");
    pintFlowCountBits = par("pintFlowCountBits");
    pintMaxFlowCount = par("pintMaxFlowCount");
    pintAutoScaleEncoding = par("pintAutoScaleEncoding");
    pintLogBase = par("pintLogBase");
    pintMaxUtilization = par("pintMaxUtilization");
    pintMaxConcurrentFlows = par("pintMaxConcurrentFlows");

    if (pintInitialRtt <= SIMTIME_ZERO)
        throw cRuntimeError("pintInitialRtt must be positive");
    if (pintNoAverageRttInterval <= SIMTIME_ZERO)
        throw cRuntimeError("pintNoAverageRttInterval must be positive");
    if (fallbackBandwidthBitsPerSecond <= 0)
        throw cRuntimeError("fallbackBandwidth must be positive");
    if (flowCountSketchEnabled && flowCardinalityBits <= 0)
        throw cRuntimeError("PINT flow counter size must be positive");
    if (pintBits < 0 || pintBits > 16)
        throw cRuntimeError("pintBits must be 0 (exact) or in the range [1, 16]");
    if (!pint::isValidFlowCountBits(pintFlowCountBits))
        throw cRuntimeError("pintFlowCountBits must be 0 (exact) or in the range [2, 16]");
    if (pintMaxFlowCount <= 0 ||
            pintMaxFlowCount > static_cast<int>(pint::FLOW_COUNT_MAX))
        throw cRuntimeError("pintMaxFlowCount must be in the range [1, 65535]");
    if (pintMaxConcurrentFlows <= 0)
        throw cRuntimeError("pintMaxConcurrentFlows must be positive");
    if (pintBits > 0 && !pintAutoScaleEncoding && pintLogBase <= 1)
        throw cRuntimeError("legacy pintLogBase must be greater than 1");
    if (pintBits > 0 && pintAutoScaleEncoding &&
            pintMaxUtilization <= 1.0 / pintMaxConcurrentFlows)
        throw cRuntimeError("pintMaxUtilization must exceed 1 / pintMaxConcurrentFlows");

    avgRtt = pintUseAverageRtt ?
            (fixedAvgRtt > SIMTIME_ZERO ? fixedAvgRtt : pintInitialRtt) :
            SIMTIME_ZERO;
    measurementInterval = pintUseAverageRtt ?
            avgRtt : pintNoAverageRttInterval;
    lastPintUpdate = SIMTIME_ZERO;

    if (flowCountSketchEnabled) {
        const int bitmapWords = (flowCardinalityBits + 63) / 64;
        activeFlowBitmap.assign(bitmapWords, 0);
        initialPhaseFlowBitmap.assign(bitmapWords, 0);
        measurementFlowBitmap.assign(bitmapWords, 0);
        initialPhaseMeasurementFlowBitmap.assign(bitmapWords, 0);
    }

    measurementTimer = new cMessage("PINT measurement timer");

    if (par("omegaPriceEnabled").boolValue()) {
        priceInterval = par("omegaPriceInterval");
        priceMinRtt = par("omegaPriceMinRtt").doubleValue();
        priceTarget = par("omegaPriceTarget");
        priceIntegralGain = par("omegaPriceIntegralGain");
        priceProportionalGain = par("omegaPriceProportionalGain");
        if (priceInterval <= SIMTIME_ZERO || !std::isfinite(priceMinRtt) ||
                priceMinRtt < priceInterval.dbl() || !std::isfinite(priceTarget) ||
                priceTarget <= 0 || priceTarget >= 1 ||
                !std::isfinite(priceIntegralGain) || priceIntegralGain <= 0 ||
                !std::isfinite(priceProportionalGain) || priceProportionalGain < 0)
            throw cRuntimeError("Invalid Omega queue price parameters");
        priceTimer = new cMessage("Omega aggregate price timer");
        scheduleAt(simTime() + priceInterval, priceTimer);
    }

    cSimpleModule::emit(bandwidthSignal, fallbackBandwidthBitsPerSecond);
    cSimpleModule::emit(avgRttSignal, avgRtt);
}

void PintQueue::handleParameterChange(const char *name)
{
    if (name == nullptr) {
        packetCapacity = par("packetCapacity");
        const int updatedPacketCapacity = par("runtimePacketCapacity");
        if (updatedPacketCapacity >= 0)
            packetCapacity = updatedPacketCapacity;
    }
    else if (!strcmp(name, "packetCapacity"))
        packetCapacity = par("packetCapacity");
    else if (!strcmp(name, "runtimePacketCapacity")) {
        const int updatedPacketCapacity = par("runtimePacketCapacity");
        if (updatedPacketCapacity >= 0)
            packetCapacity = updatedPacketCapacity;
    }
}

void PintQueue::handleMessage(cMessage *message)
{
    if (message == priceTimer) {
        const double capacity = getLinkBandwidthBytesPerSecond();
        const double offeredRate = priceOfferedBytes / priceInterval.dbl();
        linkPrice.update(offeredRate, capacity, queue.getByteLength(),
                priceInterval.dbl(), std::max(priceMinRtt, avgRtt.dbl()),
                priceTarget, priceIntegralGain, priceProportionalGain);
        priceOfferedBytes = 0;
        static const simsignal_t priceSignal = registerSignal("omegaLinkPrice");
        static const simsignal_t memorySignal = registerSignal("omegaPriceMemory");
        static const simsignal_t loadSignal = registerSignal("omegaOfferedLoad");
        cSimpleModule::emit(priceSignal, linkPrice.advertised);
        cSimpleModule::emit(memorySignal, linkPrice.memory);
        cSimpleModule::emit(loadSignal, offeredRate / capacity);
        // Keep ticking when idle: unused links must lose their old price.
        scheduleAt(simTime() + priceInterval, priceTimer);
    }
    else if (message == measurementTimer)
        processMeasurementTimer();
    else {
        auto packet = check_and_cast<Packet *>(message);
        pushPacket(packet, packet->getArrivalGate());
    }
}

void PintQueue::processMeasurementTimer()
{
    if (!telemetryActivitySinceLastTimer)
        return;

    telemetryActivitySinceLastTimer = false;
    const double bandwidthBytesPerSecond = getLinkBandwidthBytesPerSecond();

    if (pintUseAverageRtt) {
        if (fixedAvgRtt > SIMTIME_ZERO)
            avgRtt = fixedAvgRtt;
        else if (sumRttByCwnd > 0 && sumRttSquareByCwnd > 0)
            avgRtt = SimTime(sumRttSquareByCwnd / sumRttByCwnd);

        if (avgRtt <= SIMTIME_ZERO)
            avgRtt = pintInitialRtt;
    }

    if (flowCountSketchEnabled) {
        rotateFlowCounterBitmaps();
        numberOfFlows = std::max(1,
                static_cast<int>(std::lround(
                        estimateFlowCount(measurementFlowBitmap))));
        numberOfInitialPhaseFlows = std::max(0,
                static_cast<int>(std::lround(
                        estimateFlowCount(
                                initialPhaseMeasurementFlowBitmap))));
    }
    else {
        numberOfFlows = std::max(1, static_cast<int>(activeFlowIds.size()));
        numberOfInitialPhaseFlows =
                static_cast<int>(initialPhaseFlowIds.size());
    }

    const double queueingDelay = bandwidthBytesPerSecond > 0 ?
            static_cast<double>(queue.getByteLength()) / bandwidthBytesPerSecond : 0;

    cSimpleModule::emit(avgRttSignal, avgRtt);
    cSimpleModule::emit(numberOfFlowsSignal, numberOfFlows);
    cSimpleModule::emit(effectiveNumberOfFlowsSignal, numberOfFlows);
    cSimpleModule::emit(numOfFlowsInInitialPhaseSignal, numberOfInitialPhaseFlows);
    cSimpleModule::emit(persistentQueueingDelaySignal, queueingDelay);
    cSimpleModule::emit(bandwidthSignal, bandwidthBytesPerSecond * 8.0);

    sumRttByCwnd = 0;
    sumRttSquareByCwnd = 0;
    resetCompletedFlowCounters();

    measurementInterval = pintUseAverageRtt ?
            avgRtt : pintNoAverageRttInterval;
    scheduleMeasurementTimer();
}

void PintQueue::scheduleMeasurementTimer()
{
    if (!measurementTimer->isScheduled())
        scheduleAt(simTime() + measurementInterval, measurementTimer);
}

double PintQueue::getLinkBandwidthBytesPerSecond() const
{
    auto *networkInterface = dynamic_cast<NetworkInterface *>(getParentModule());
    auto *channel = networkInterface != nullptr ?
            networkInterface->getTxTransmissionChannel() : nullptr;
    if (channel != nullptr && channel->getNominalDatarate() > 0)
        return channel->getNominalDatarate() / 8.0;
    return fallbackBandwidthBitsPerSecond / 8.0;
}

void PintQueue::markFlow(std::vector<uint64_t>& bitmap, uint64_t flowId)
{
    const uint64_t hash = mixHash(flowId ^ flowSketchSeed ^ 0xd6e8feb86659fd93ULL);
    const size_t bit = hash % static_cast<uint64_t>(flowCardinalityBits);
    bitmap[bit / 64] |= 1ULL << (bit % 64);
}

double PintQueue::estimateFlowCount(const std::vector<uint64_t>& bitmap) const
{
    size_t setBits = 0;
    for (uint64_t word : bitmap)
        setBits += static_cast<size_t>(__builtin_popcountll(word));
    setBits = std::min(setBits, static_cast<size_t>(flowCardinalityBits));

    const size_t zeroBits = flowCardinalityBits - setBits;
    if (zeroBits == static_cast<size_t>(flowCardinalityBits))
        return 0;
    if (zeroBits == 0)
        return flowCardinalityBits * std::log(static_cast<double>(flowCardinalityBits));

    return -flowCardinalityBits *
            std::log(static_cast<double>(zeroBits) / flowCardinalityBits);
}

void PintQueue::resetCompletedFlowCounters()
{
    if (flowCountSketchEnabled) {
        std::fill(measurementFlowBitmap.begin(), measurementFlowBitmap.end(), 0);
        std::fill(initialPhaseMeasurementFlowBitmap.begin(),
                initialPhaseMeasurementFlowBitmap.end(), 0);
    }
    else {
        activeFlowIds.clear();
        initialPhaseFlowIds.clear();
    }
}

void PintQueue::rotateFlowCounterBitmaps()
{
    activeFlowBitmap.swap(measurementFlowBitmap);
    initialPhaseFlowBitmap.swap(initialPhaseMeasurementFlowBitmap);
}

int PintQueue::getTotalFlowCount() const
{
    return std::clamp(numberOfFlows, 1, 65535);
}

int PintQueue::getInitialPhaseFlowCount() const
{
    return std::clamp(numberOfInitialPhaseFlows, 0, 65535);
}

double PintQueue::updatePintUtilization(uint64_t payloadBytes, uint64_t queueBytes,
        double bandwidthBytesPerSecond, double flowRttSeconds, uint64_t flowId)
{
    const double rttSeconds = pintUseAverageRtt ?
            (avgRtt > SIMTIME_ZERO ? avgRtt.dbl() : pintInitialRtt.dbl()) :
            (flowRttSeconds > 0 ? flowRttSeconds :
                    pintNoAverageRttInterval.dbl());
    const double payloadServiceTime = payloadBytes / bandwidthBytesPerSecond;
    const double queueUtilization =
            queueBytes / (bandwidthBytesPerSecond * rttSeconds);

    if (!hasPintSample) {
        lastPintUpdate = simTime();
        hasPintSample = true;
        if (pintUseAverageRtt) {
            pintUtilization = queueUtilization;
            return pintUtilization;
        }

        perFlowPintUtilization[flowId] = {queueUtilization, simTime()};
        return queueUtilization;
    }

    double tau = (simTime() - lastPintUpdate).dbl();
    tau = std::max(tau, payloadServiceTime);

    const double sample = queueUtilization +
            payloadBytes / (bandwidthBytesPerSecond * tau);
    // Keep the full interval in the rate sample, including after idle periods.
    // Only the EWMA weight is capped: the reference RTT is its time scale.
    lastPintUpdate = simTime();
    if (pintUseAverageRtt) {
        const double gain = std::clamp(tau / rttSeconds, 0.0, 1.0);
        pintUtilization = (1 - gain) * pintUtilization + gain * sample;
        return pintUtilization;
    }

    auto [it, inserted] = perFlowPintUtilization.emplace(
            flowId, FlowUtilization{sample, simTime()});
    if (!inserted) {
        // This EWMA advances only on this flow's packets; use its own elapsed
        // time for smoothing, while the sample above still measures the link.
        const double flowTau = std::max((simTime() - it->second.updatedAt).dbl(), payloadServiceTime);
        const double gain = std::clamp(flowTau / rttSeconds, 0.0, 1.0);
        it->second.value = (1 - gain) * it->second.value + gain * sample;
        it->second.updatedAt = simTime();
    }
    return it->second.value;
}

uint16_t PintQueue::encodePintUtilization(double utilization)
{
    if (pintBits == 0)
        return 0;

    const uint32_t maxPower = pintBits == 16 ?
            std::numeric_limits<uint16_t>::max() : (1U << pintBits) - 1;
    const double minimumUtilization = 1.0 / pintMaxConcurrentFlows;
    const double clampedUtilization = pintAutoScaleEncoding ?
            std::max(minimumUtilization, utilization) :
            std::max(1.0, std::ceil(
                    utilization * pintMaxConcurrentFlows)) /
                    pintMaxConcurrentFlows;
    const double logBase = getPintUtilizationLogBase();
    const double exactPower = std::log(
            clampedUtilization / minimumUtilization) / std::log(logBase);

    if (exactPower >= maxPower)
        return static_cast<uint16_t>(maxPower);

    const uint16_t lowerPower = static_cast<uint16_t>(std::floor(exactPower));
    const uint16_t upperPower = static_cast<uint16_t>(std::ceil(exactPower));
    if (lowerPower == upperPower)
        return lowerPower;

    const double lowerValue = minimumUtilization *
            std::pow(logBase, lowerPower);
    const double upperValue = minimumUtilization *
            std::pow(logBase, upperPower);
    const double upperProbability =
            (clampedUtilization - lowerValue) / (upperValue - lowerValue);
    return getRNG(0)->doubleRand() < upperProbability ? upperPower : lowerPower;
}

double PintQueue::decodePintUtilization(uint16_t power) const
{
    if (pintBits == 0)
        return 0;

    return std::pow(getPintUtilizationLogBase(), power) /
            pintMaxConcurrentFlows;
}

double PintQueue::getPintUtilizationLogBase() const
{
    if (!pintAutoScaleEncoding)
        return pintLogBase;

    const uint32_t maxPower = pintBits == 16 ?
            std::numeric_limits<uint16_t>::max() : (1U << pintBits) - 1;
    const double minimumUtilization = 1.0 / pintMaxConcurrentFlows;
    const double rangeFittingBase = std::pow(
            pintMaxUtilization / minimumUtilization, 1.0 / maxPower);

    return rangeFittingBase;
}

uint64_t PintQueue::mixHash(uint64_t value)
{
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

uint32_t PintQueue::updatePathDigest(uint32_t digest, uint32_t hopId)
{
    if (digest == 0)
        digest = 2166136261U;
    digest ^= hopId;
    return digest * 16777619U;
}

void PintQueue::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    cNamedObject packetPushStartedDetails("atomicOperationStarted");
    emit(packetPushStartedSignal, packet, &packetPushStartedDetails);
    EV_INFO << "Pushing packet" << EV_FIELD(packet) << EV_ENDL;

    auto ipv4Header = packet->removeAtFront<Ipv4Header>();
    if (ipv4Header->getTotalLengthField() < packet->getDataLength())
        packet->setBackOffset(B(ipv4Header->getTotalLengthField()) -
                ipv4Header->getChunkLength());

    if (ipv4Header->getProtocolId() == 6) {
        auto tcpHeader = packet->removeAtFront<tcp::TcpHeader>();
        const uint64_t tcpPayloadBytes = packet->getByteLength();
        if (tcpPayloadBytes > 0) {
            // Count arrivals before drop/admission, including retransmissions.
            // Counting departures would hide overload behind the link capacity.
            if (priceTimer != nullptr)
                priceOfferedBytes += tcpPayloadBytes;
            auto intTag = getAttachedIntTagForUpdate(tcpHeader);
            if (!telemetryActivitySinceLastTimer) {
                telemetryActivitySinceLastTimer = true;
                scheduleMeasurementTimer();
            }
            const uint64_t flowId = static_cast<uint64_t>(intTag->getConnId());

            if (pintUseAverageRtt) {
                const double baseRtt =
                        pint::decodeBaseRtt(intTag->getPintBaseRttCode());
                const uint64_t cwndBytes =
                        pint::decodeCwnd(intTag->getPintCwndCode());
                if (baseRtt > 0 && cwndBytes > 0) {
                    const double weight =
                            static_cast<double>(tcpPayloadBytes) / cwndBytes;
                    sumRttByCwnd += baseRtt * weight;
                    sumRttSquareByCwnd += baseRtt * baseRtt * weight;
                }
            }

            if (flowCountSketchEnabled) {
                markFlow(activeFlowBitmap, flowId);
                if (intTag->getInitialPhase())
                    markFlow(initialPhaseFlowBitmap, flowId);
            }
            else {
                activeFlowIds.insert(flowId);
                if (intTag->getInitialPhase())
                    initialPhaseFlowIds.insert(flowId);
            }

            auto& intData = intTag->getIntDataForUpdate();
            if (intData.empty())
                intData.emplace_back();
        }

        packet->insertAtFront(tcpHeader);
    }

    ipv4Header->setTotalLengthField(ipv4Header->getChunkLength() +
            packet->getDataLength());
    packet->insertAtFront(ipv4Header);

    queue.insert(packet);
    if (buffer != nullptr)
        buffer->addPacket(packet);
    else if (packetDropperFunction != nullptr) {
        while (isOverloaded()) {
            auto packetToDrop = packetDropperFunction->selectPacket(this);
            queue.remove(packetToDrop);
            dropPacket(packetToDrop, QUEUE_OVERFLOW);
        }
    }
    ASSERT(!isOverloaded());
    if (collector != nullptr && getNumPackets() != 0)
        collector->handleCanPullPacketChanged(outputGate->getPathEndGate());

    cNamedObject packetPushEndedDetails("atomicOperationEnded");
    emit(packetPushEndedSignal, nullptr, &packetPushEndedDetails);
    updateDisplayString();
}

Packet *PintQueue::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    auto packet = check_and_cast<Packet *>(queue.front());
    EV_INFO << "Pulling packet" << EV_FIELD(packet) << EV_ENDL;

    if (buffer != nullptr) {
        queue.remove(packet);
        buffer->removePacket(packet);
    }
    else
        queue.pop();

    const simtime_t queueingTime = simTime() - packet->getArrivalTime();
    auto packetEvent = new PacketQueuedEvent();
    packetEvent->setQueuePacketLength(getNumPackets());
    packetEvent->setQueueDataLength(getTotalLength());
    insertPacketEvent(this, packet, PEK_QUEUED, queueingTime, packetEvent);
    increaseTimeTag<QueueingTimeTag>(packet, queueingTime, queueingTime);

    const uint64_t queueBytes = queue.getByteLength();
    const double bandwidthBytesPerSecond =
            getLinkBandwidthBytesPerSecond();

    auto ipv4Header = packet->removeAtFront<Ipv4Header>();
    if (ipv4Header->getTotalLengthField() < packet->getDataLength())
        packet->setBackOffset(B(ipv4Header->getTotalLengthField()) -
                ipv4Header->getChunkLength());

    if (ipv4Header->getProtocolId() == 6) {
        auto tcpHeader = packet->removeAtFront<tcp::TcpHeader>();
        const uint64_t tcpPayloadBytes = packet->getByteLength();
        double localUtilization = 0;
        if (tcpPayloadBytes > 0 && tcpHeader->findTag<OmegaPriceTag>()) {
            tcpHeader->addTagIfAbsent<OmegaPriceTag>(); // Detach COW region tags.
            auto price = tcpHeader->addTagIfAbsent<OmegaPriceTag>();
            if (!price->echoed) {
                if (price->hops == 0) {
                    price->sampledAt = simTime();
                    price->capacity = bandwidthBytesPerSecond;
                }
                price->hops++;
                price->capacity = std::min(price->capacity, bandwidthBytesPerSecond);
                if (priceTimer != nullptr) {
                    price->pricedHops++;
                    price->price += linkPrice.advertised;
                }
            }
        }
        if (tcpPayloadBytes > 0) {
            double flowRttSeconds = 0;
            uint64_t flowId = 0;
            if (!pintUseAverageRtt) {
                const auto intTag = tcpHeader->findTag<IntTag>();
                if (intTag != nullptr) {
                    flowRttSeconds = intTag->getRtt().dbl();
                    flowId = static_cast<uint64_t>(intTag->getConnId());
                }
            }
            localUtilization = updatePintUtilization(
                    tcpPayloadBytes, queueBytes, bandwidthBytesPerSecond,
                    flowRttSeconds, flowId);
            txBytes += tcpPayloadBytes;
            cSimpleModule::emit(txBytesSignal, txBytes);
            cSimpleModule::emit(pintLocalUtilizationSignal, localUtilization);
        }

        if (tcpHeader->findTag<IntTag>()) {
            auto intTag = getAttachedIntTagForUpdate(tcpHeader);
            auto& intDataVector = intTag->getIntDataForUpdate();
            if (!intDataVector.empty()) {
                IntMetaData& intData = intDataVector.front();
                pint::accumulatePacketQueueingDelay(intData, tcpPayloadBytes > 0,
                        queueingTime.dbl());

                // Split-delay MpORB ACKs preserve the forward accumulator;
                // legacy ACKs still sum both directions in the original field.
                // Neither ACK format replaces the forward bottleneck record.
                if (tcpPayloadBytes > 0) {
                    const int hopId = getParentModule()->getParentModule()->getId();
                    const uint16_t power = encodePintUtilization(localUtilization);
                    const double decodedUtilization = pintBits == 0 ?
                            localUtilization : decodePintUtilization(power);
                    const int totalFlowCount = getTotalFlowCount();
                    const bool initialPhase = intTag->getInitialPhase();
                    const int initialPhaseFlowCount = initialPhase ?
                            std::max(1, getInitialPhaseFlowCount()) : 0;
                    const uint32_t totalFlowCountCode =
                            pint::encodeFlowCount(totalFlowCount,
                                    pintFlowCountBits, pintMaxFlowCount);
                    const uint32_t initialFlowCountCode = initialPhase ?
                            pint::encodeFlowCount(initialPhaseFlowCount,
                                    pintFlowCountBits,
                                    pintMaxFlowCount) : 0;
                    const uint32_t decodedTotalFlowCount =
                            pint::decodeFlowCount(totalFlowCountCode,
                                    pintFlowCountBits, pintMaxFlowCount);
                    const double localFairShare =
                            bandwidthBytesPerSecond / decodedTotalFlowCount;
                    const bool hasBottleneckRecord = intData.getB() > 0;
                    const double currentFairShare = hasBottleneckRecord ?
                            intData.getB() /
                            std::max(1U, pint::decodeFlowCount(
                                    intData.getPintTotalFlowCountCode(),
                                    pintFlowCountBits,
                                    pintMaxFlowCount)) :
                            0;

                    intData.setPathDigest(updatePathDigest(
                            intData.getPathDigest(), static_cast<uint32_t>(hopId)));

                    // Equal utilization records retain the tighter OrbCC fair share.
                    if (!hasBottleneckRecord ||
                            decodedUtilization > intData.getPintUtilization() ||
                            (decodedUtilization == intData.getPintUtilization() &&
                            localFairShare < currentFairShare)) {
                        intData.setPintPower(power);
                        intData.setPintUtilization(decodedUtilization);
                        intData.setHopId(hopId);
                        intData.setTs(simTime());
                        intData.setB(bandwidthBytesPerSecond);
                        intData.setQLen(queueBytes);
                        intData.setRxQlen(0);
                        intData.setTxBytes(0);
                        intData.setAverageRtt(0);
                        intData.setPintTotalFlowCountCode(totalFlowCountCode);
                        if (initialPhase)
                            intData.setPintInitialFlowCountCode(initialFlowCountCode);
                        else
                            intData.setPintInitialFlowCountCode(0);
                    }

                    cSimpleModule::emit(pintDecodedUtilizationSignal, decodedUtilization);
                    cSimpleModule::emit(pintEncodedPowerSignal, static_cast<long>(power));
                }
            }
        }
        packet->insertAtFront(tcpHeader);
    }

    ipv4Header->setTotalLengthField(ipv4Header->getChunkLength() +
            packet->getDataLength());
    packet->insertAtFront(ipv4Header);
    emit(packetPulledSignal, packet);
    animatePullPacket(packet, outputGate);
    updateDisplayString();
    return packet;
}

} // namespace queueing
} // namespace inet
