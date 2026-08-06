//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//

package orbtcp.queueing.queue;

import inet.queueing.queue.PacketQueue;

simple PintQueue extends PacketQueue
{
    parameters:
        @class("inet::queueing::PintQueue");

        @signal[avgRttQueue];
        @statistic[avgRttQueue](record=vector; interpolationmode=sample-hold);

        @signal[numberOfFlows];
        @statistic[numberOfFlows](record=vector; interpolationmode=sample-hold);

        @signal[effectiveNumberOfFlows];
        @statistic[effectiveNumberOfFlows](record=vector; interpolationmode=sample-hold);

        @signal[numOfFlowsInInitialPhase];
        @statistic[numOfFlowsInInitialPhase](record=vector; interpolationmode=sample-hold);

        @signal[persistentQueueingDelay];
        @statistic[persistentQueueingDelay](record=vector; interpolationmode=sample-hold);

        @signal[bandwidth];
        @statistic[bandwidth](record=vector; interpolationmode=sample-hold);

        @signal[txBytes];
        @statistic[txBytes](record=vector; interpolationmode=sample-hold);

        @signal[pintLocalUtilization];
        @statistic[pintLocalUtilization](record=vector; interpolationmode=sample-hold);

        @signal[pintDecodedUtilization];
        @statistic[pintDecodedUtilization](record=vector; interpolationmode=sample-hold);

        @signal[pintEncodedPower];
        @statistic[pintEncodedPower](record=vector; interpolationmode=sample-hold);

        packetCapacity @mutable = default(100);
        double fallbackBandwidth @unit(bps) = default(100Mbps);
        double fixedAvgRTTVal @unit(s) = default(0s);
        double pintInitialRtt @unit(s) = default(10ms);
        double alpha = default(0.03);

        // Set false for an exact per-interval flow set, matching base OrbCC.
        bool flowCountSketchEnabled = default(true);
        // N and S each use active and completed-epoch bitmap banks.
        int flowCardinalityBits = default(4096);
        int flowSketchSeed = default(1337);

        // U encoding: zero carries the exact simulation value.
        int pintBits = default(8);
        // N/S encoding: zero carries the exact 16-bit count.
        int pintFlowCountBits = default(8);
        // Upper count represented by logarithmic modes; it must cover the workload.
        int pintMaxFlowCount = default(65535);

        // Set true for bit-width sweeps; false preserves the legacy 1.05 base.
        bool pintAutoScaleEncoding = default(false);
        double pintMaxUtilization = default(4);
        int pintMaxConcurrentFlows = default(512);
        // Used only when pintAutoScaleEncoding is false.
        double pintLogBase = default(1.05);

        dropperClass = default("inet::queueing::PacketAtCollectionEndDropper");
}
