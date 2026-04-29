//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include <algorithm>

#include "OrbtcpNewFlavour.h"

namespace inet {
namespace tcp {

Register_Class(OrbtcpNewFlavour);

double OrbtcpNewFlavour::measureInflight(IntDataVec intData)
{
    double u = 0;
    double tau = 0;
    double bottleneckAverageRtt = 0;
    double bottleneckBandwidth = 0;
    double bottleneckTxRate = 0;
    double totalQueueingDelay = 0;

    uint32_t bottleneckTxBytes = 0;
    double bottleneckRtt = 0;
    double bottleneckSharingFlows = 1;
    double bottleneckInitPhaseFlows = 0;
    bool bottleneckIsPastAck = false;
    std::vector<bool> currPathId(16);

    if (state->L.empty()) {
        for (int i = 0; i < intData.size(); i++) {
            IntMetaData *intDataEntry = intData.at(i);
            int hopId = intDataEntry->getHopId();
            std::vector<bool> bitArray(16);
            std::vector<bool> tempBitArray(16);
            int bitSize = 16;

            for (size_t j = 0; j < bitSize; ++j)
                bitArray[(bitSize - 1) - j] = (hopId >> j) & 1;

            for (size_t j = 0; j < bitArray.size(); ++j)
                tempBitArray[j] = currPathId[j] ^ bitArray[j];

            currPathId = tempBitArray;
            bottleneckAverageRtt = std::max(bottleneckAverageRtt, intDataEntry->getAverageRtt());
        }

        if (pathId.empty())
            pathId = currPathId;
        state->L = intData;

        if (!initReactTimer->isScheduled() && bottleneckAverageRtt > 0)
            conn->scheduleAt(simTime() + bottleneckAverageRtt, initReactTimer);
        return 0;
    }

    if (intData.size() == state->L.size()) {
        for (int i = 0; i < intData.size(); i++) {
            IntMetaData *intDataEntry = intData.at(i);
            double uPrime = 0;
            bool isPastAck = false;
            int hopId = intDataEntry->getHopId();
            std::vector<bool> bitArray(16);
            std::vector<bool> tempBitArray(16);
            int bitSize = 16;

            for (size_t j = 0; j < bitSize; ++j)
                bitArray[(bitSize - 1) - j] = (hopId >> j) & 1;

            for (size_t j = 0; j < bitArray.size(); ++j)
                tempBitArray[j] = currPathId[j] ^ bitArray[j];

            currPathId = tempBitArray;

            if (intDataEntry->getHopId() == state->L.at(i)->getHopId() && intDataEntry->getAverageRtt() > 0) {
                totalQueueingDelay += (double)intDataEntry->getRxQlen() / (double)intDataEntry->getB();

                const double deltaTs = intDataEntry->getTs().dbl() - state->L.at(i)->getTs().dbl();
                if (deltaTs == 0)
                    continue;

                const double hopTxRate = (intDataEntry->getTxBytes() - state->L.at(i)->getTxBytes()) / deltaTs;
                uPrime =
                    (std::min(intDataEntry->getQLen(), state->L.at(i)->getQLen()) / (intDataEntry->getB() * intDataEntry->getAverageRtt())) +
                    (hopTxRate / intDataEntry->getB());

                if (deltaTs < 0)
                    isPastAck = true;

                if (uPrime > u) {
                    u = uPrime;
                    tau = deltaTs;

                    const double effectiveSharingFlows = intDataEntry->getEffectiveNumOfFlows();
                    bottleneckSharingFlows = effectiveSharingFlows > 0 ? effectiveSharingFlows : intDataEntry->getNumOfFlows();
                    bottleneckInitPhaseFlows = intDataEntry->getNumOfFlowsInInitialPhase();
                    bottleneckAverageRtt = intDataEntry->getAverageRtt();
                    bottleneckRtt = deltaTs;
                    bottleneckTxRate = hopTxRate;
                    bottleneckIsPastAck = isPastAck;
                    bottleneckTxBytes = intDataEntry->getTxBytes() - state->L.at(i)->getTxBytes();
                    if (bottleneckAverageRtt <= 0) {
                        bottleneckAverageRtt = estimatedRtt.dbl();
                        EV_DEBUG << "bottleneckAverageRtt is lower or equal to 0!\n";
                    }
                    bottleneckBandwidth = intDataEntry->getB();
                }
            }
        }
    }

    if (!initReactTimer->isScheduled() && bottleneckAverageRtt > 0)
        conn->scheduleAt(simTime() + bottleneckAverageRtt, initReactTimer);
    if (bottleneckIsPastAck || pathChanged || bottleneckBandwidth <= 0)
        return 0;

    if (pathId.empty()) {
        pathId = currPathId;
    }
    else if (pathId != currPathId) {
        pathId = currPathId;
        pathChanged = true;
        state->L = intData;
        return 0;
    }

    state->sharingFlows = std::max(1.0, bottleneckSharingFlows);
    state->initialPhaseSharingFlows = bottleneckInitPhaseFlows;
    state->txRate = bottleneckTxRate;
    state->bottBW = bottleneckBandwidth;
    state->queueingDelay = totalQueueingDelay;

    conn->emit(queueingDelaySignal, state->queueingDelay);
    conn->emit(avgEstimatedRttSignal, bottleneckAverageRtt);
    conn->emit(uSignal, u);
    conn->emit(tauSignal, tau);
    conn->emit(sharingFlowsSignal, state->sharingFlows);
    conn->emit(txBytesSignal, bottleneckTxBytes);

    if (state->useHpccAlpha)
        state->alpha = tau / bottleneckAverageRtt;

    if (state->u == 0)
        state->u = u;

    state->u = (1 - state->alpha) * state->u + state->alpha * u;
    conn->emit(alphaSignal, state->alpha);
    conn->emit(USignal, state->u);

    if (!firstRTT) {
        const double sharedRttRef =
            bottleneckAverageRtt > 0 ? bottleneckAverageRtt
            : (smoothedEstimatedRtt > 0 ? smoothedEstimatedRtt.dbl()
            : (estimatedRtt > 0 ? estimatedRtt.dbl() : rtt.dbl()));

        if (!state->initialPhase) {
            state->additiveIncrease =
                ((((bottleneckBandwidth) / state->sharingFlows) * rtt.dbl()) * state->additiveIncreasePercent);
            state->ssthresh = 0;
        }
        else {
            const double startupCompetingFlows =
                std::max(1.0, static_cast<double>(state->sharingFlows + state->initialPhaseSharingFlows));
            const double startupTargetWnd = state->eta * (bottleneckBandwidth / startupCompetingFlows) * rtt.dbl();

            state->ssthresh = static_cast<uint32_t>(std::max(0.0, startupTargetWnd));
            state->additiveIncrease = static_cast<uint32_t>(std::max(0.0, startupTargetWnd - state->snd_cwnd));
            state->endInitialPhase = true;
        }
    }

    conn->emit(testRttSignal, bottleneckRtt);
    conn->emit(bottleneckBandwidthSignal, bottleneckBandwidth);
    conn->emit(avgRttSignal, bottleneckAverageRtt);
    conn->emit(additiveIncreaseSignal, state->additiveIncrease);
    conn->emit(ssthreshSignal, state->ssthresh);

    return state->u;
}

} // namespace tcp
} // namespace inet
