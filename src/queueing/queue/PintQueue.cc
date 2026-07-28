//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//

#include "PintQueue.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "inet/common/PacketEventTag.h"
#include "inet/common/TimeTag.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader_m.h"

#include "../../common/IntTag_m.h"

namespace inet {
namespace queueing {

Define_Module(PintQueue);

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
    flowCardinalityBits = par("flowCardinalityBits");
    flowSketchSeed = static_cast<uint64_t>(par("flowSketchSeed").intValue());
    pintBits = par("pintBits");
    pintLogBase = par("pintLogBase");
    pintMaxConcurrentFlows = par("pintMaxConcurrentFlows");

    if (pintInitialRtt <= SIMTIME_ZERO)
        throw cRuntimeError("pintInitialRtt must be positive");
    if (fallbackBandwidthBitsPerSecond <= 0)
        throw cRuntimeError("fallbackBandwidth must be positive");
    if (flowCardinalityBits <= 0)
        throw cRuntimeError("PINT flow counter size must be positive");
    if (pintBits <= 0 || pintBits > 16)
        throw cRuntimeError("pintBits must be in the range [1, 16]");
    if (pintLogBase <= 1 || pintMaxConcurrentFlows <= 0)
        throw cRuntimeError("PINT logarithmic encoding parameters are invalid");

    avgRtt = fixedAvgRtt > SIMTIME_ZERO ? fixedAvgRtt : pintInitialRtt;
    measurementInterval = avgRtt;
    lastPintUpdate = SIMTIME_ZERO;

    const int bitmapWords = (flowCardinalityBits + 63) / 64;
    activeFlowBitmap.assign(bitmapWords, 0);
    initialPhaseFlowBitmap.assign(bitmapWords, 0);

    measurementTimer = new cMessage("PINT measurement timer");

    cSimpleModule::emit(bandwidthSignal, fallbackBandwidthBitsPerSecond);
    cSimpleModule::emit(avgRttSignal, avgRtt);
}

void PintQueue::handleMessage(cMessage *message)
{
    if (message == measurementTimer)
        processMeasurementTimer();
    else {
        scheduleMeasurementTimer();
        auto packet = check_and_cast<Packet *>(message);
        pushPacket(packet, packet->getArrivalGate());
    }
}

void PintQueue::processMeasurementTimer()
{
    const double bandwidthBytesPerSecond = getLinkBandwidthBytesPerSecond();

    if (fixedAvgRtt > SIMTIME_ZERO)
        avgRtt = fixedAvgRtt;
    else if (sumRttByCwnd > 0 && sumRttSquareByCwnd > 0)
        avgRtt = SimTime(sumRttSquareByCwnd / sumRttByCwnd);

    if (avgRtt <= SIMTIME_ZERO)
        avgRtt = pintInitialRtt;

    numberOfFlows = std::max(1, static_cast<int>(std::lround(estimateFlowCount(activeFlowBitmap))));
    numberOfInitialPhaseFlows =
            std::max(0, static_cast<int>(std::lround(estimateFlowCount(initialPhaseFlowBitmap))));

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
    resetFlowCounters();

    measurementInterval = avgRtt;
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

void PintQueue::resetFlowCounters()
{
    std::fill(activeFlowBitmap.begin(), activeFlowBitmap.end(), 0);
    std::fill(initialPhaseFlowBitmap.begin(), initialPhaseFlowBitmap.end(), 0);
}

double PintQueue::updatePintUtilization(uint64_t packetBytes, uint64_t queueBytes,
        double bandwidthBytesPerSecond)
{
    const double rttSeconds = avgRtt > SIMTIME_ZERO ?
            avgRtt.dbl() : pintInitialRtt.dbl();
    const double serializationTime = packetBytes / bandwidthBytesPerSecond;
    double tau = hasPintSample ?
            (simTime() - lastPintUpdate).dbl() : serializationTime;

    if (tau <= 0)
        tau = serializationTime;
    tau = std::clamp(tau, std::min(serializationTime, rttSeconds), rttSeconds);

    const double weight = tau / rttSeconds;
    const double sample = queueBytes / (bandwidthBytesPerSecond * rttSeconds) +
            packetBytes / (bandwidthBytesPerSecond * tau);
    pintUtilization = (1 - weight) * pintUtilization + weight * sample;

    lastPintUpdate = simTime();
    hasPintSample = true;
    return pintUtilization;
}

uint16_t PintQueue::encodePintUtilization(double utilization)
{
    const uint32_t maxPower = pintBits == 16 ?
            std::numeric_limits<uint16_t>::max() : (1U << pintBits) - 1;
    const double scaledUtilization =
            std::max(1.0, std::ceil(utilization * pintMaxConcurrentFlows));
    const double exactPower = std::log(scaledUtilization) / std::log(pintLogBase);

    if (exactPower >= maxPower)
        return static_cast<uint16_t>(maxPower);

    const uint16_t lowerPower = static_cast<uint16_t>(std::floor(exactPower));
    const uint16_t upperPower = static_cast<uint16_t>(std::ceil(exactPower));
    if (lowerPower == upperPower)
        return lowerPower;

    const double lowerValue = std::pow(pintLogBase, lowerPower);
    const double upperValue = std::pow(pintLogBase, upperPower);
    const double upperProbability =
            (scaledUtilization - lowerValue) / (upperValue - lowerValue);
    return getRNG(0)->doubleRand() < upperProbability ? upperPower : lowerPower;
}

double PintQueue::decodePintUtilization(uint16_t power) const
{
    return std::pow(pintLogBase, power) / pintMaxConcurrentFlows;
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
    const uint64_t packetBytesAtQueue = packet->getByteLength();
    cNamedObject packetPushStartedDetails("atomicOperationStarted");
    emit(packetPushStartedSignal, packet, &packetPushStartedDetails);
    EV_INFO << "Pushing packet" << EV_FIELD(packet) << EV_ENDL;

    auto ipv4Header = packet->removeAtFront<Ipv4Header>();
    if (ipv4Header->getTotalLengthField() < packet->getDataLength())
        packet->setBackOffset(B(ipv4Header->getTotalLengthField()) -
                ipv4Header->getChunkLength());

    if (ipv4Header->getProtocolId() == 6) {
        auto tcpHeader = packet->removeAtFront<tcp::TcpHeader>();
        if (packet->getDataLength() > b(0)) {
            auto intTag = tcpHeader->addTagIfAbsent<IntTag>();
            const uint64_t flowId = static_cast<uint64_t>(intTag->getConnId());

            if (intTag->getRtt() > SIMTIME_ZERO && intTag->getCwnd() > 0) {
                const double weight = static_cast<double>(packetBytesAtQueue) /
                        intTag->getCwnd();
                sumRttByCwnd += intTag->getRtt().dbl() * weight;
                sumRttSquareByCwnd += intTag->getRtt().dbl() *
                        intTag->getRtt().dbl() * weight;
            }

            markFlow(activeFlowBitmap, flowId);
            if (intTag->getInitialPhase())
                markFlow(initialPhaseFlowBitmap, flowId);

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
    const uint64_t packetBytesAtQueue = packet->getByteLength();
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
    const double localUtilization = updatePintUtilization(
            packetBytesAtQueue, queueBytes, bandwidthBytesPerSecond);
    txBytes += packetBytesAtQueue;
    cSimpleModule::emit(txBytesSignal, txBytes);
    cSimpleModule::emit(pintLocalUtilizationSignal, localUtilization);

    auto ipv4Header = packet->removeAtFront<Ipv4Header>();
    if (ipv4Header->getTotalLengthField() < packet->getDataLength())
        packet->setBackOffset(B(ipv4Header->getTotalLengthField()) -
                ipv4Header->getChunkLength());

    if (ipv4Header->getProtocolId() == 6) {
        auto tcpHeader = packet->removeAtFront<tcp::TcpHeader>();
        if (packet->getByteLength() > 0 && tcpHeader->findTag<IntTag>()) {
            auto& intDataVector =
                    tcpHeader->addTagIfAbsent<IntTag>()->getIntDataForUpdate();
            if (!intDataVector.empty()) {
                IntMetaData& intData = intDataVector.front();
                const int hopId = getParentModule()->getParentModule()->getId();

                const uint16_t power = encodePintUtilization(localUtilization);
                const double decodedUtilization = decodePintUtilization(power);
                const double localFairShare =
                        bandwidthBytesPerSecond / std::max(1, numberOfFlows);
                const double currentFairShare = intData.getPintValid() ?
                        intData.getB() /
                        std::max(1, intData.getNumOfFlows()) :
                        std::numeric_limits<double>::infinity();

                intData.setPathDigest(updatePathDigest(
                        intData.getPathDigest(), static_cast<uint32_t>(hopId)));
                intData.setAccumulatedQueueingDelay(
                        intData.getAccumulatedQueueingDelay() +
                        queueBytes / bandwidthBytesPerSecond);

                // Equal quantized utilizations retain the tighter OrbCC fair share.
                if (!intData.getPintValid() ||
                        power > intData.getPintPower() ||
                        (power == intData.getPintPower() &&
                        localFairShare < currentFairShare)) {
                    intData.setPintValid(true);
                    intData.setPintPower(power);
                    intData.setPintUtilization(decodedUtilization);
                    intData.setHopId(hopId);
                    intData.setTs(simTime());
                    intData.setB(bandwidthBytesPerSecond);
                    intData.setQLen(queueBytes);
                    intData.setRxQlen(queueBytes);
                    intData.setTxBytes(0);
                    intData.setAverageRtt(0);
                    intData.setNumOfFlows(numberOfFlows);
                    intData.setEffectiveNumOfFlows(numberOfFlows);
                    intData.setNumOfFlowsInInitialPhase(
                            numberOfInitialPhaseFlows);
                }

                cSimpleModule::emit(pintDecodedUtilizationSignal, decodedUtilization);
                cSimpleModule::emit(pintEncodedPowerSignal, static_cast<long>(power));
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
