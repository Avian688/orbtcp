//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//

#ifndef QUEUEING_QUEUE_PINTQUEUE_H_
#define QUEUEING_QUEUE_PINTQUEUE_H_

#include <cstdint>
#include <vector>

#include "inet/queueing/queue/PacketQueue.h"

namespace inet {
namespace queueing {

/**
 * OrbCC queue using HPCC-PINT-style local utilization and a single,
 * logarithmically encoded bottleneck record.
 */
class PintQueue : public PacketQueue
{
  protected:
    static simsignal_t avgRttSignal;
    static simsignal_t numberOfFlowsSignal;
    static simsignal_t effectiveNumberOfFlowsSignal;
    static simsignal_t numOfFlowsInInitialPhaseSignal;
    static simsignal_t persistentQueueingDelaySignal;
    static simsignal_t bandwidthSignal;
    static simsignal_t txBytesSignal;
    static simsignal_t pintLocalUtilizationSignal;
    static simsignal_t pintDecodedUtilizationSignal;
    static simsignal_t pintEncodedPowerSignal;

    cMessage *measurementTimer = nullptr;

    simtime_t avgRtt;
    simtime_t measurementInterval;
    simtime_t fixedAvgRtt;
    simtime_t pintInitialRtt;
    simtime_t lastPintUpdate;

    double fallbackBandwidthBitsPerSecond = 0;
    double sumRttByCwnd = 0;
    double sumRttSquareByCwnd = 0;
    double pintUtilization = 0;
    double alpha = 0.03;
    double pintLogBase = 1.05;

    uint64_t txBytes = 0;
    uint64_t flowSketchSeed = 0;

    int numberOfFlows = 1;
    int numberOfInitialPhaseFlows = 0;
    int flowCardinalityBits = 0;
    int pintBits = 8;
    int pintMaxConcurrentFlows = 512;

    bool hasPintSample = false;

    std::vector<uint64_t> activeFlowBitmap;
    std::vector<uint64_t> initialPhaseFlowBitmap;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void processMeasurementTimer();
    virtual void scheduleMeasurementTimer();

    virtual double getLinkBandwidthBytesPerSecond() const;

    virtual void markFlow(std::vector<uint64_t>& bitmap, uint64_t flowId);
    virtual double estimateFlowCount(const std::vector<uint64_t>& bitmap) const;
    virtual void resetFlowCounters();

    virtual double updatePintUtilization(uint64_t packetBytes, uint64_t queueBytes,
            double bandwidthBytesPerSecond);
    virtual uint16_t encodePintUtilization(double utilization);
    virtual double decodePintUtilization(uint16_t power) const;

    static uint64_t mixHash(uint64_t value);
    static uint32_t updatePathDigest(uint32_t digest, uint32_t hopId);

  public:
    virtual ~PintQueue();

    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual Packet *pullPacket(cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif
