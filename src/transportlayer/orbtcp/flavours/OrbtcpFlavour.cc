//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <algorithm> // min,max
#include <cmath>

#include "inet/transportlayer/tcp/Tcp.h"
#include "OrbtcpFlavour.h"

namespace inet {
namespace tcp {

#define MIN_REXMIT_TIMEOUT     0.2   // 1s
#define MAX_REXMIT_TIMEOUT     240   // 2 * MSL (RFC 1122)

Register_Class(OrbtcpFlavour);

simsignal_t OrbtcpFlavour::txRateSignal = cComponent::registerSignal("txRate");
simsignal_t OrbtcpFlavour::tauSignal = cComponent::registerSignal("tau");
simsignal_t OrbtcpFlavour::uSignal = cComponent::registerSignal("u");
simsignal_t OrbtcpFlavour::USignal = cComponent::registerSignal("U");
simsignal_t OrbtcpFlavour::additiveIncreaseSignal = cComponent::registerSignal("additiveIncrease");
simsignal_t OrbtcpFlavour::sharingFlowsSignal = cComponent::registerSignal("sharingFlows");
simsignal_t OrbtcpFlavour::bottleneckBandwidthSignal = cComponent::registerSignal("bottleneckBandwidth");
simsignal_t OrbtcpFlavour::avgRttSignal = cComponent::registerSignal("avgRtt");
simsignal_t OrbtcpFlavour::queueingDelaySignal = cComponent::registerSignal("queueingDelay");
simsignal_t OrbtcpFlavour::estimatedRttSignal = cComponent::registerSignal("estimatedRtt");
simsignal_t OrbtcpFlavour::smoothedEstimatedRttSignal = cComponent::registerSignal("smoothedEstimatedRtt");
simsignal_t OrbtcpFlavour::avgEstimatedRttSignal = cComponent::registerSignal("avgEstimatedRtt");
simsignal_t OrbtcpFlavour::alphaSignal = cComponent::registerSignal("alpha");
simsignal_t OrbtcpFlavour::measuringInflightSignal = cComponent::registerSignal("measuringInflight");
simsignal_t OrbtcpFlavour::txBytesSignal = cComponent::registerSignal("txBytes");

simsignal_t OrbtcpFlavour::testRttSignal = cComponent::registerSignal("testRtt");

simsignal_t OrbtcpFlavour::sndUnaSignal = cComponent::registerSignal("sndUna");
simsignal_t OrbtcpFlavour::sndMaxSignal = cComponent::registerSignal("sndMax");

simsignal_t OrbtcpFlavour::recoveryPointSignal = cComponent::registerSignal("recoveryPoint");

OrbtcpFlavour::OrbtcpFlavour() : OrbtcpFamily(),
    state((OrbtcpStateVariables *&)TcpAlgorithm::state)
{
}

OrbtcpFlavour::~OrbtcpFlavour() {
    cancelEvent(reactTimer);
    delete reactTimer;

    cancelEvent(initReactTimer);
    delete initReactTimer;
}

void OrbtcpFlavour::initialize()
{
    OrbtcpFamily::initialize();
    state->B = conn->getTcpMain()->par("bandwidth");
    state->subFlows = conn->getTcpMain()->par("subFlows");
    state->sharingFlows = conn->getTcpMain()->par("sharingFlows");
    state->initialPhaseSharingFlows = 1;
    state->additiveIncreasePercent = conn->getTcpMain()->par("additiveIncreasePercent");
    state->eta = conn->getTcpMain()->par("eta");
    state->T = conn->getTcpMain()->par("basePropagationRTT");
    state->queueingDelay = 0;
    state->additiveIncrease = 1;
    state->prevWnd = 10000;
    state->initialPhase = false;
    state->L.clear();
    firstRTT = true;
    initPackets = false;
    pathChanged = false;
    pathId.clear();
    bottleneckId = -1;

    state->alpha = conn->getTcpMain()->par("alpha");
    if(state->alpha > 0){
        state->useHpccAlpha = false;
    }
    else{
        state->useHpccAlpha = true;
    }
    reactTimer = new cMessage("React Timer");
    initReactTimer = new cMessage("Init React Timer");
    updateWindow = false;
    //updateNext = false;
}

void OrbtcpFlavour::established(bool active)
{
    //state->snd_cwnd = state->B * state->T.dbl();
    state->snd_cwnd = 7300; //5 packets
    dynamic_cast<OrbtcpConnection*>(conn)->changeIntersendingTime(0.000001); //do not pace intial packets as RTT is unknown
    state->ssthresh = 73000;
    connId = std::hash<std::string>{}(conn->localAddr.str() + "/" + std::to_string(conn->localPort) + "/" + conn->remoteAddr.str() + "/" + std::to_string(conn->remotePort));
    initPackets = true;
    EV_DETAIL << "OrbTCP initial CWND is set to " << state->snd_cwnd << "\n";
    if (active) {
        // finish connection setup with ACK (possibly piggybacked on data)
        EV_INFO << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";
        sendData(false);
        conn->sendAck();
    }
}

void OrbtcpFlavour::rttMeasurementComplete(simtime_t tSent, simtime_t tAcked)
{
    //
    // Jacobson's algorithm for estimating RTT and adaptively setting RTO.
    //
    // Note: this implementation calculates in doubles. An impl. which uses
    // 500ms ticks is available from old tcpmodule.cc:calcRetransTimer().
    //

    // update smoothed RTT estimate (srtt) and variance (rttvar)
    const double g = 0.125; // 1 / 8; (1 - alpha) where alpha == 7 / 8;
    simtime_t newRTT = tAcked - tSent;
    estimatedRtt = newRTT - state->queueingDelay;

    if(state->srtt == 0){
        state->srtt = newRTT;
    }

    if(smoothedEstimatedRtt == 0){
        smoothedEstimatedRtt = newRTT;
    }

    simtime_t& srtt = state->srtt;
    simtime_t& rttvar = state->rttvar;

    simtime_t estErr = estimatedRtt - smoothedEstimatedRtt;
    simtime_t err = newRTT - srtt;

    srtt += g * err;
    rttvar += g * (fabs(err) - rttvar);

    smoothedEstimatedRtt += g * estErr;

    // assign RTO (here: rexmit_timeout) a new value
    simtime_t rto = srtt + 4 * rttvar;

    if (rto > MAX_REXMIT_TIMEOUT)
        rto = MAX_REXMIT_TIMEOUT;
    else if (rto < MIN_REXMIT_TIMEOUT)
        rto = MIN_REXMIT_TIMEOUT;

    state->rexmit_timeout = rto;

    dynamic_cast<TcpPacedConnection*>(conn)->setMinRtt(std::min(srtt, dynamic_cast<TcpPacedConnection*>(conn)->getMinRtt()));

    // record statistics
    EV_DETAIL << "Measured RTT=" << (newRTT * 1000) << "ms, updated SRTT=" << (srtt * 1000)
              << "ms, new RTO=" << (rto * 1000) << "ms\n";

    rtt = newRTT;

    if(estimatedRtt <= 0 || smoothedEstimatedRtt <= 0){
        estimatedRtt = rtt;
        smoothedEstimatedRtt = srtt;
    }

    conn->emit(rttSignal, newRTT);
    conn->emit(estimatedRttSignal, estimatedRtt);
    conn->emit(smoothedEstimatedRttSignal, smoothedEstimatedRtt);
    conn->emit(srttSignal, srtt);
    conn->emit(rttvarSignal, rttvar);
    conn->emit(rtoSignal, rto);
}

void OrbtcpFlavour::receivedDataAck(uint32_t firstSeqAcked, IntDataVec intData)
{
    TcpTahoeRenoFamily::receivedDataAck(firstSeqAcked);
    EV_INFO << "\nORBTCPInfo ___________________________________________" << endl;
    EV_INFO << "\nORBTCPInfo - Received Data Ack" << endl;
    bool wasInLossRecovery = state->lossRecovery && state->sack_enabled;
    if (wasInLossRecovery) {
        // RFC 3517, page 7: "Once a TCP is in the loss recovery phase the following procedure MUST
        // be used for each arriving ACK:
        //
        // (A) An incoming cumulative ACK for a sequence number greater than
        // RecoveryPoint signals the end of loss recovery and the loss
        // recovery phase MUST be terminated.  Any information contained in
        // the scoreboard for sequence numbers greater than the new value of
        // HighACK SHOULD NOT be cleared when leaving the loss recovery
        // phase."

        if (seqGE(state->snd_una, state->recoveryPoint)) {
            EV_INFO << "Loss Recovery terminated.\n";
            state->lossRecovery = false;
        }
        // RFC 3517, page 7: "(B) Upon receipt of an ACK that does not cover RecoveryPoint the
        // following actions MUST be taken:
        // (B.1) Use Update () to record the new SACK information conveyed
        // by the incoming ACK.
        //
        // (B.2) Use SetPipe () to re-calculate the number of octets still
        // in the network."
        else {
            dynamic_cast<TcpPacedConnection*>(conn)->doRetransmit();
            //sendData(false);
            // update of scoreboard (B.1) has already be done in readHeaderOptions()
            //conn->setPipe();
        }
        conn->emit(recoveryPointSignal, state->recoveryPoint);

        conn->emit(cwndSignal, state->snd_cwnd);
        if (!reactTimer->isScheduled())
            conn->scheduleAt(simTime() + state->srtt.dbl(), reactTimer);
        conn->emit(sndUnaSignal, state->snd_una);
        conn->emit(sndMaxSignal, state->snd_max);
        return;
    }

    double uVal = measureInflight(intData);

    if(uVal > 0) {
        if(!pathChanged){
            state->snd_cwnd = computeWnd(uVal, updateWindow);
        }
        state->L = intData;
    }

    state->lastUpdateSeq = state->snd_nxt;
    conn->emit(cwndSignal, state->snd_cwnd);

    if(state->snd_cwnd > 0){
        uint32_t maxWindow = std::max(state->snd_cwnd, dynamic_cast<TcpPacedConnection*>(conn)->getBytesInFlight());
        uint32_t nominalBandwidth = (maxWindow / state->srtt.dbl());
        double pace = 1/((1.2 *(double)nominalBandwidth)/(double)state->snd_mss);
        dynamic_cast<OrbtcpConnection*>(conn)->changeIntersendingTime(pace);
    }
    // Check if recovery phase has ended
    sendData(false);

    if(!reactTimer->isScheduled()){
        conn->scheduleAt(simTime() + state->srtt.dbl(), reactTimer);
    }

    conn->emit(sndUnaSignal, state->snd_una);
    conn->emit(sndMaxSignal, state->snd_max);
}

void OrbtcpFlavour::receivedDuplicateAck(uint32_t firstSeqAcked, IntDataVec intData)
{
    state->initialPhase = false;
    auto pacedConn = dynamic_cast<TcpPacedConnection*>(conn);

    if (shouldEnterLossRecoveryOnDuplicateAck()) {
        EV_INFO << "Reno on dupAcks == DUPTHRESH(=" << state->dupthresh << ": perform Fast Retransmit, and enter Fast Recovery:";

        if (state->sack_enabled) {
            // RFC 3517, page 6: "When a TCP sender receives the duplicate ACK corresponding to
            // DupThresh ACKs, the scoreboard MUST be updated with the new SACK
            // information (via Update ()).  If no previous loss event has occurred
            // on the connection or the cumulative acknowledgment point is beyond
            // the last value of RecoveryPoint, a loss recovery phase SHOULD be
            // initiated, per the fast retransmit algorithm outlined in [RFC2581].
            // The following steps MUST be taken:
            //
            // (1) RecoveryPoint = HighData
            //
            // When the TCP sender receives a cumulative ACK for this data octet
            // the loss recovery phase is terminated."

            // RFC 3517, page 8: "If an RTO occurs during loss recovery as specified in this document,
            // RecoveryPoint MUST be set to HighData.  Further, the new value of
            // RecoveryPoint MUST be preserved and the loss recovery algorithm
            // outlined in this document MUST be terminated.  In addition, a new
            // recovery phase (as described in section 5) MUST NOT be initiated
            // until HighACK is greater than or equal to the new value of
            // RecoveryPoint."
            if ((state->recoveryPoint == 0 || seqGE(state->snd_una, state->recoveryPoint)) && !state->lossRecovery) { // HighACK = snd_una
                state->recoveryPoint = state->snd_max; // HighData = snd_max
                state->lossRecovery = true;
                conn->emit(recoveryPointSignal, state->recoveryPoint);
                pacedConn->setSackedHeadLostIfRackDisabled();
                pacedConn->updateInFlight();
                setRecoveryCongestionWindow();
                //std::cout << "\n Entering Loss recovery - dup acks > dupthresh at simTime: " << simTime().dbl() << endl;
                EV_DETAIL << " recoveryPoint=" << state->recoveryPoint;
                pacedConn->doRetransmit();
                //conn->rescheduleAt(simTime() + state->srtt.dbl(), reactTimer);

                //sendData(false);
            }
        }

        // Fast Retransmission: retransmit missing segment without waiting
        // for the REXMIT timer to expire
        // Do not restart REXMIT timer.
        // Note: Restart of REXMIT timer on retransmission is not part of RFC 2581, however optional in RFC 3517 if sent during recovery.
        // Resetting the REXMIT timer is discussed in RFC 2582/3782 (NewReno) and RFC 2988.

        if (state->sack_enabled) {
            if (state->lossRecovery) {
                EV_INFO << "Retransmission sent during recovery, restarting REXMIT timer.\n";
                restartRexmitTimer();
            }
        }

    }
    else if (state->lossRecovery && state->dupacks > state->dupthresh)
        EV_DETAIL << "Additional duplicate ACK during RACK recovery; cwnd remains "
                  << state->snd_cwnd << "\n";

    if (state->lossRecovery) {
        conn->emit(cwndSignal, state->snd_cwnd);
        if (state->snd_cwnd > 0) {
            uint32_t maxWindow = std::max(state->snd_cwnd, pacedConn->getBytesInFlight());
            uint32_t nominalBandwidth = maxWindow / state->srtt.dbl();
            double pace = 1 / ((1.2 * (double)nominalBandwidth) / state->snd_mss);
            dynamic_cast<OrbtcpConnection *>(conn)->changeIntersendingTime(pace);
        }
        if (!reactTimer->isScheduled())
            conn->scheduleAt(simTime() + state->srtt.dbl(), reactTimer);
        return;
    }

    double uVal = measureInflight(intData);
    if(uVal > 0) {
        if(!pathChanged){
            state->snd_cwnd = computeWnd(uVal, updateWindow);
        }
        state->L = intData;
    }
    conn->emit(cwndSignal, state->snd_cwnd);
    //state->lastUpdateSeq = state->snd_nxt;
    if(state->snd_cwnd > 0){
        uint32_t maxWindow = std::max(state->snd_cwnd, dynamic_cast<TcpPacedConnection*>(conn)->getBytesInFlight());
        uint32_t nominalBandwidth = (maxWindow / state->srtt.dbl());
        double pace = 1/((1.2 *(double)nominalBandwidth)/(double)state->snd_mss);
        dynamic_cast<OrbtcpConnection*>(conn)->changeIntersendingTime(pace);
    }

    sendData(false);

    if(!reactTimer->isScheduled()){
        conn->scheduleAt(simTime() + state->srtt.dbl(), reactTimer);
    }
}

double OrbtcpFlavour::measureInflight(IntDataVec intData)
{
    if(intData.empty())
        return 0;

    // The first INT sample is only a baseline: a rate requires two queue
    // counter/timestamp observations. Store it explicitly instead of relying
    // on the initial U value to make the caller retain it.
    if(state->L.size() != intData.size()){
        state->L = intData;
        return 0;
    }

    double u = 0;
    double tau = 0;
    double bottleneckAverageRtt = 0;
    double bottleneckBandwidth = 0;
    double bottleneckTxRate = 0;
    double totalQueueingDelay = 0;

    uint32_t bottleneckTxBytes = 0;
    double bottleneckRtt = 0;
    double bottleneckSharingFlows = 1;
    int currentBottleneckId = -1;
    bool foundBottleneck = false;
    std::vector<bool> currPathId(16);

    if(intData.size() == state->L.size()){
        for(int i = 0; i < intData.size(); i++){ //Start at front of queue. First item is first hop etc.
            const IntMetaData *intDataEntry = &intData.at(i);
            double uPrime = 0;
            int hopId = intDataEntry->getHopId();
            std::vector<bool> bitArray(16);
            std::vector<bool> tempBitArray(16);
            int bitSize = 16;

            for (size_t i = 0; i < bitSize; ++i) {
                bitArray[(bitSize-1) - i] = (hopId >> i) & 1;
            }

            for (size_t i = 0; i < bitArray.size(); ++i) {
                tempBitArray[i] = currPathId[i] ^ bitArray[i];  // XOR the corresponding bits
            }

            currPathId = tempBitArray;
            if(i < state->L.size()) {
                if(intDataEntry->getHopId() == state->L.at(i).getHopId() && intDataEntry->getAverageRtt() > 0) {
                    const double sampleInterval = intDataEntry->getTs().dbl() - state->L.at(i).getTs().dbl();
                    const double bandwidth = intDataEntry->getB();
                    if(sampleInterval <= 0 || bandwidth <= 0)
                        continue;

                    //std::bitset<1> b = (a1 ^= a2);
                    totalQueueingDelay +=(double)intDataEntry->getRxQlen()/bandwidth;
                    //txRate is bytes observed at router between previous and current ACK packet subtracted from the timestamp of the previous and current ack. Equals estimated rate.
                    double hopTxRate = (intDataEntry->getTxBytes() - state->L.at(i).getTxBytes())/sampleInterval;
                    uPrime = (std::min(intDataEntry->getQLen(), state->L.at(i).getQLen())/(bandwidth*intDataEntry->getAverageRtt()))+(hopTxRate/bandwidth);

                    if(std::isfinite(uPrime) && uPrime > u) {
                        u = uPrime;
                        tau = sampleInterval;

                        bottleneckSharingFlows = intDataEntry->getNumOfFlows();
                        bottleneckAverageRtt = intDataEntry->getAverageRtt();
                        bottleneckRtt = sampleInterval;
                        bottleneckTxRate = hopTxRate;
                        bottleneckTxBytes = intDataEntry->getTxBytes() - state->L.at(i).getTxBytes();
                        bottleneckBandwidth = bandwidth;
                        currentBottleneckId = hopId;
                        foundBottleneck = true;
                    }
                }
            }
        }
    }

    // A first or stale INT sample cannot define a rate interval yet. Do not
    // commit uninitialized bottleneck state; the caller retains this sample
    // as the baseline for the next ACK.
    if(!foundBottleneck)
        return 0;

    if(!initReactTimer->isScheduled()){
        conn->scheduleAt(simTime() + bottleneckAverageRtt, initReactTimer);
    }
    if(pathChanged){
        return 0;
    }

    if(pathId.empty()) {
        pathId = currPathId;
    }
    else if((pathId != currPathId)){
        //updateNext = true;
        pathId = currPathId;
        pathChanged = true;
        bottleneckId = -1;
        //state->L = IntDataVec(); //reset
        state->L = intData;
        return 0;
    }

    state->sharingFlows = std::max(1.0, bottleneckSharingFlows);
    bottleneckId = currentBottleneckId;


    state->txRate = bottleneckTxRate;
    state->bottBW = bottleneckBandwidth;
    state->queueingDelay = totalQueueingDelay;

    conn->emit(queueingDelaySignal, state->queueingDelay);
    conn->emit(avgEstimatedRttSignal, bottleneckAverageRtt);
    conn->emit(uSignal, u);
    conn->emit(tauSignal, tau);
    conn->emit(sharingFlowsSignal, state->sharingFlows);
    conn->emit(txBytesSignal, bottleneckTxBytes);

    if(state->useHpccAlpha){
        state->alpha = std::clamp(tau/bottleneckAverageRtt, 0.0, 1.0);
    }

    if(state->u == 0 || !std::isfinite(state->u)){
        state->u = u;
    }

    state->u = (1-state->alpha)*state->u+state->alpha*u;
    conn->emit(alphaSignal, state->alpha);
    conn->emit(USignal, state->u);

    if(!firstRTT){
        if(!state->initialPhase){ //slow start - more aggressive till max allowed share is reached
            state->additiveIncrease = ((((bottleneckBandwidth)/state->sharingFlows) * rtt.dbl()) * state->additiveIncreasePercent);
            state->ssthresh = 0;
        }
        else{
            state->ssthresh = (((bottleneckBandwidth/(state->sharingFlows+state->initialPhaseSharingFlows)) * smoothedEstimatedRtt.dbl()) * state->eta);
            double initAI = state->additiveIncreasePercent;
//            if(state->eta/state->sharingFlows < initAI){
//                initAI = state->eta/state->sharingFlows;
//            }

            //state->additiveIncrease = ((((bottleneckBandwidth)/std::max(1, state->initialPhaseSharingFlows)) * rtt.dbl()) * initAI);

            state->additiveIncrease = state->ssthresh > state->snd_cwnd ?
                    state->ssthresh - state->snd_cwnd : 0;

            //if(state->snd_cwnd >= state->ssthresh){
            state->endInitialPhase = true;
            //}
        }
    }

    adjustAdditiveIncrease();

    conn->emit(testRttSignal, bottleneckRtt);
    conn->emit(bottleneckBandwidthSignal, bottleneckBandwidth);
    conn->emit(avgRttSignal, bottleneckAverageRtt);
    conn->emit(additiveIncreaseSignal, state->additiveIncrease);
    conn->emit(ssthreshSignal, state->ssthresh);

    return state->u;
}

uint32_t OrbtcpFlavour::computeWnd(double u, bool updateWc)
{
    uint32_t targetW;
    if(u >= state->eta) {
        targetW = (state->prevWnd/(u/state->eta)) + state->additiveIncrease;
    }
    else {
        targetW = state->prevWnd + state->additiveIncrease;
    }

    if(updateWc) {
        updateWindow = false;
        state->prevWnd = targetW;
        if(state->endInitialPhase){
            state->initialPhase = false;
        }
        conn->emit(txRateSignal, state->txRate);
    }

    return targetW;
}

size_t OrbtcpFlavour::getConnId()
{
    return connId;
}

int OrbtcpFlavour::getBottleneckId() const
{
    return bottleneckId;
}

simtime_t OrbtcpFlavour::getRtt()
{
    return state->srtt;
}

simtime_t OrbtcpFlavour::getEstimatedRtt()
{
    return smoothedEstimatedRtt;
}

void OrbtcpFlavour::rackLossDetected()
{
    bool wasInRecovery = state->lossRecovery;
    TcpPacedFamily::rackLossDetected();
    if (!wasInRecovery && state->lossRecovery) {
        conn->emit(recoveryPointSignal, state->recoveryPoint);
        conn->emit(cwndSignal, state->snd_cwnd);
    }
}

void OrbtcpFlavour::processRexmitTimer(TcpEventCode &event) {
    TcpPacedFamily::processRexmitTimer(event);
    if (event == TCP_E_ABORT)
        return;

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
                   << ", ssthresh=" << state->ssthresh << "\n";

    state->afterRto = true;
    dynamic_cast<OrbtcpConnection*>(conn)->cancelPaceTimer();
    sendData(false);
}

void OrbtcpFlavour::processTimer(cMessage *timer, TcpEventCode& event)
{
    if(timer == reactTimer){
        updateWindow = true;
        pathChanged = false;
        conn->scheduleAt(simTime() + state->srtt.dbl(), reactTimer);
    }
    else if(timer == initReactTimer){
        if(firstRTT == true){
            firstRTT = false;
            state->initialPhase = true;
        }
    }
    else if (timer == rexmitTimer)
        processRexmitTimer(event);
    else if (timer == persistTimer)
        processPersistTimer(event);
    else if (timer == delayedAckTimer)
        processDelayedAckTimer(event);
    else if (timer == keepAliveTimer)
        processKeepAliveTimer(event);
    else
        throw cRuntimeError(timer, "unrecognized timer");
}

bool OrbtcpFlavour::getInitialPhase()
{
    if(state->initialPhase || firstRTT){
        return true;
    }
    return false;
}

} // namespace tcp
} // namespace inet
