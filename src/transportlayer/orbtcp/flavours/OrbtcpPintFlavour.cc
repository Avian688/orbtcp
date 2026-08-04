//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//

#include "OrbtcpPintFlavour.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "../../../common/PintFlowCount.h"
#include "../../../common/PintQueueingDelay.h"

namespace inet {
namespace tcp {

Register_Class(OrbtcpPintFlavour);

namespace {

uint32_t clampWindow(double window)
{
    if (!std::isfinite(window) || window <= 0)
        return 0;

    return static_cast<uint32_t>(std::min(window,
            static_cast<double>(std::numeric_limits<uint32_t>::max())));
}

} // namespace

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

void OrbtcpPintFlavour::updateRttTelemetry(const IntDataVec& intData)
{
    state->queueingDelay = 0;
    if (intData.empty()) {
        conn->emit(queueingDelaySignal, state->queueingDelay);
        return;
    }

    state->queueingDelay =
            pint::decodeQueueingDelay(intData.front().getQueueingDelayCode());
    conn->emit(queueingDelaySignal, state->queueingDelay);
}

void OrbtcpPintFlavour::processRexmitTimer(TcpEventCode& event)
{
    state->initialPhase = false;
    state->endInitialPhase = false;
    OrbtcpFlavour::processRexmitTimer(event);
}

void OrbtcpPintFlavour::rackLossDetected()
{
    state->initialPhase = false;
    state->endInitialPhase = false;
    OrbtcpFlavour::rackLossDetected();
}

double OrbtcpPintFlavour::measureInflight(IntDataVec intData)
{
    if (intData.empty())
        return 0;

    const IntMetaData& pintData = intData.front();
    const double utilization = pintData.getPintUtilization();
    const double bottleneckBandwidth = pintData.getB();
    if (!std::isfinite(utilization) || utilization <= 0 ||
            !std::isfinite(bottleneckBandwidth) || bottleneckBandwidth <= 0)
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

    const uint32_t totalFlowCount = pint::decodeFlowCount(
            pintData.getPintTotalFlowCountCode());
    if (totalFlowCount == 0)
        return 0;

    state->sharingFlows = totalFlowCount;
    state->initialPhaseSharingFlows = pint::decodeFlowCount(
            pintData.getPintInitialFlowCountCode());
    state->bottBW = static_cast<uint32_t>(bottleneckBandwidth);
    state->queueingDelay =
            pint::decodeQueueingDelay(pintData.getQueueingDelayCode());
    state->txRate = utilization * bottleneckBandwidth;
    state->u = utilization;
    state->alpha = 1;

    bottleneckId = pintData.getHopId();
    pathHopMetrics.clear();

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
            simtime_t startupRtt = smoothedEstimatedRtt;
            if (startupRtt <= SIMTIME_ZERO)
                startupRtt = rtt > SIMTIME_ZERO ? rtt : state->srtt;
            if (startupRtt <= SIMTIME_ZERO)
                startupRtt = state->T;

            const int initialPhaseFlows =
                    std::max(1, state->initialPhaseSharingFlows);
            // N sets the fair-rate ceiling; S divides one link-wide 5% startup budget.
            const double fairWindow = state->eta *
                    (bottleneckBandwidth / state->sharingFlows) *
                    startupRtt.dbl();
            const double remainingWindow = std::max(0.0,
                    fairWindow - state->snd_cwnd);
            const double startupIncreaseBudget =
                    (bottleneckBandwidth / initialPhaseFlows) *
                    startupRtt.dbl() * state->additiveIncreasePercent;

            state->ssthresh = clampWindow(fairWindow);
            state->endInitialPhase = utilization >= state->eta;
            state->additiveIncrease = state->endInitialPhase ? 0 :
                    clampWindow(std::min(remainingWindow,
                            startupIncreaseBudget));
        }
    }

    adjustAdditiveIncrease();

    conn->emit(testRttSignal, feedbackInterval);
    conn->emit(bottleneckBandwidthSignal, bottleneckBandwidth);
    conn->emit(additiveIncreaseSignal, state->additiveIncrease);
    conn->emit(ssthreshSignal, state->ssthresh);

    return state->u;
}

uint32_t OrbtcpPintFlavour::computeWnd(double u, bool updateWc)
{
    // Keep ACK-burst feedback from applying the per-RTT increase more than once.
    if (!updateWc)
        return state->snd_cwnd;

    const double currentWindow = state->snd_cwnd;
    double targetWindow = u >= state->eta ?
            currentWindow / (u / state->eta) + state->additiveIncrease :
            currentWindow + state->additiveIncrease;

    if (state->initialPhase && state->ssthresh > 0)
        targetWindow = std::min(targetWindow,
                static_cast<double>(state->ssthresh));

    uint32_t targetWnd = clampWindow(targetWindow);
    const bool cwndLimited = isCwndLimited();
    targetWnd = limitCwndGrowth(targetWnd, cwndLimited);
    conn->emit(cwndLimitedSignal, cwndLimited);

    updateWindow = false;
    state->prevWnd = targetWnd;
    if (state->initialPhase && state->ssthresh > 0 &&
            targetWnd >= state->ssthresh)
        state->endInitialPhase = true;
    if (state->endInitialPhase) {
        state->initialPhase = false;
        state->endInitialPhase = false;
    }
    conn->emit(txRateSignal, state->txRate);

    return targetWnd;
}

} // namespace tcp
} // namespace inet
