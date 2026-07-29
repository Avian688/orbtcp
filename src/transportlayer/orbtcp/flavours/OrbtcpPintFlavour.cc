//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//

#include "OrbtcpPintFlavour.h"

#include <algorithm>
#include <cmath>

namespace inet {
namespace tcp {

Register_Class(OrbtcpPintFlavour);

void OrbtcpPintFlavour::initialize()
{
    OrbtcpFlavour::initialize();

    pintFeedbackProbability =
            conn->getTcpMain()->par("pintFeedbackProbability").doubleValue();
    if (pintFeedbackProbability < 0 || pintFeedbackProbability > 1)
        throw cRuntimeError("pintFeedbackProbability must be in the range [0, 1]");

    lastPathDigest = 0;
    hasPathDigest = false;
    lastPintFeedback = SIMTIME_ZERO;
}

double OrbtcpPintFlavour::measureInflight(IntDataVec intData)
{
    if (intData.empty())
        return 0;

    const IntMetaData& pintData = intData.front();
    if (!pintData.getPintValid())
        return 0;

    if (pintFeedbackProbability < 1 &&
            conn->getTcpMain()->getRNG(0)->doubleRand() >=
                    pintFeedbackProbability)
        return 0;

    const uint32_t currentPathDigest = pintData.getPathDigest();
    if (!hasPathDigest) {
        lastPathDigest = currentPathDigest;
        hasPathDigest = true;
    }
    else if (currentPathDigest != lastPathDigest) {
        lastPathDigest = currentPathDigest;
        pathChanged = true;
        bottleneckId = -1;
        pathHopMetrics.clear();
        lastPintFeedback = SIMTIME_ZERO;
        state->L = intData;
        return state->u;
    }

    const double utilization = pintData.getPintUtilization();
    const double bottleneckBandwidth = pintData.getB();
    if (!std::isfinite(utilization) || utilization <= 0 ||
            !std::isfinite(bottleneckBandwidth) || bottleneckBandwidth <= 0)
        return 0;

    const simtime_t feedbackInterval = lastPintFeedback > SIMTIME_ZERO ?
            simTime() - lastPintFeedback :
            (state->srtt > SIMTIME_ZERO ? state->srtt : state->T);
    lastPintFeedback = simTime();

    simtime_t startupDelay =
            state->srtt > SIMTIME_ZERO ? state->srtt : state->T;
    if (startupDelay <= SIMTIME_ZERO)
        startupDelay = SimTime(10, SIMTIME_MS);
    if (!initReactTimer->isScheduled())
        conn->scheduleAt(simTime() + startupDelay, initReactTimer);

    state->sharingFlows =
            std::max(1, pintData.getNumOfFlows());
    state->initialPhaseSharingFlows =
            std::max(0, pintData.getNumOfFlowsInInitialPhase());
    state->bottBW = static_cast<uint32_t>(bottleneckBandwidth);
    state->queueingDelay =
            std::max(0.0, pintData.getAccumulatedQueueingDelay());
    state->txRate = utilization * bottleneckBandwidth;
    state->u = utilization;
    state->alpha = 1;

    bottleneckId = pintData.getHopId();
    pathHopMetrics.clear();

    conn->emit(queueingDelaySignal, state->queueingDelay);
    conn->emit(uSignal, utilization);
    conn->emit(USignal, state->u);
    conn->emit(tauSignal, feedbackInterval);
    conn->emit(alphaSignal, state->alpha);
    conn->emit(sharingFlowsSignal, state->sharingFlows);

    if (!firstRTT) {
        if (!state->initialPhase) {
            state->additiveIncrease =
                    ((bottleneckBandwidth / state->sharingFlows) *
                    rtt.dbl()) * state->additiveIncreasePercent;
            state->ssthresh = 0;
        }
        else {
            state->ssthresh =
                    (bottleneckBandwidth /
                    (state->sharingFlows + state->initialPhaseSharingFlows)) *
                    smoothedEstimatedRtt.dbl() * state->eta;
            state->additiveIncrease = state->ssthresh > state->snd_cwnd ?
                    state->ssthresh - state->snd_cwnd : 0;
            state->endInitialPhase = true;
        }
    }

    adjustAdditiveIncrease();

    conn->emit(testRttSignal, feedbackInterval);
    conn->emit(bottleneckBandwidthSignal, bottleneckBandwidth);
    conn->emit(additiveIncreaseSignal, state->additiveIncrease);
    conn->emit(ssthreshSignal, state->ssthresh);

    return state->u;
}

} // namespace tcp
} // namespace inet
