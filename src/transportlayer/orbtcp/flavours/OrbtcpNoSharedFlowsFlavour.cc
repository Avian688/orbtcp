//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "OrbtcpNoSharedFlowsFlavour.h"

namespace inet {
namespace tcp {

#define MIN_REXMIT_TIMEOUT     0.2   // 1s
#define MAX_REXMIT_TIMEOUT     240   // 2 * MSL (RFC 1122)

Register_Class(OrbtcpNoSharedFlowsFlavour);

simsignal_t OrbtcpNoSharedFlowsFlavour::txRateSignal = cComponent::registerSignal("txRate");
simsignal_t OrbtcpNoSharedFlowsFlavour::tauSignal = cComponent::registerSignal("tau");
simsignal_t OrbtcpNoSharedFlowsFlavour::uSignal = cComponent::registerSignal("u");
simsignal_t OrbtcpNoSharedFlowsFlavour::USignal = cComponent::registerSignal("U");
simsignal_t OrbtcpNoSharedFlowsFlavour::additiveIncreaseSignal = cComponent::registerSignal("additiveIncrease");
simsignal_t OrbtcpNoSharedFlowsFlavour::sharingFlowsSignal = cComponent::registerSignal("sharingFlows");
simsignal_t OrbtcpNoSharedFlowsFlavour::bottleneckBandwidthSignal = cComponent::registerSignal("bottleneckBandwidth");
simsignal_t OrbtcpNoSharedFlowsFlavour::avgRttSignal = cComponent::registerSignal("avgRtt");
simsignal_t OrbtcpNoSharedFlowsFlavour::queueingDelaySignal = cComponent::registerSignal("queueingDelay");
simsignal_t OrbtcpNoSharedFlowsFlavour::estimatedRttSignal = cComponent::registerSignal("estimatedRtt");
simsignal_t OrbtcpNoSharedFlowsFlavour::smoothedEstimatedRttSignal = cComponent::registerSignal("smoothedEstimatedRtt");
simsignal_t OrbtcpNoSharedFlowsFlavour::avgEstimatedRttSignal = cComponent::registerSignal("avgEstimatedRtt");
simsignal_t OrbtcpNoSharedFlowsFlavour::alphaSignal = cComponent::registerSignal("alpha");
simsignal_t OrbtcpNoSharedFlowsFlavour::measuringInflightSignal = cComponent::registerSignal("measuringInflight");
simsignal_t OrbtcpNoSharedFlowsFlavour::txBytesSignal = cComponent::registerSignal("txBytes");

simsignal_t OrbtcpNoSharedFlowsFlavour::testRttSignal = cComponent::registerSignal("testRtt");

simsignal_t OrbtcpNoSharedFlowsFlavour::sndUnaSignal = cComponent::registerSignal("sndUna");
simsignal_t OrbtcpNoSharedFlowsFlavour::sndMaxSignal = cComponent::registerSignal("sndMax");

simsignal_t OrbtcpNoSharedFlowsFlavour::recoveryPointSignal = cComponent::registerSignal("recoveryPoint");

OrbtcpNoSharedFlowsFlavour::OrbtcpNoSharedFlowsFlavour() : OrbtcpFamily(),
    state((OrbtcpStateVariables *&)TcpAlgorithm::state)
{
}

OrbtcpNoSharedFlowsFlavour::~OrbtcpNoSharedFlowsFlavour() {
    cancelEvent(reactTimer);
    delete reactTimer;
}

void OrbtcpNoSharedFlowsFlavour::initialize()
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

    state->alpha = conn->getTcpMain()->par("alpha");
    if(state->alpha > 0){
        state->useHpccAlpha = false;
    }
    else{
        state->useHpccAlpha = true;
    }
    reactTimer = new cMessage("React Timer");
    updateWindow = true;
    //updateNext = false;
}

void OrbtcpNoSharedFlowsFlavour::established(bool active)
{
    //state->snd_cwnd = state->B * state->T.dbl();
    state->snd_cwnd = 7300; //5 packets
    dynamic_cast<OrbtcpConnection*>(conn)->changeIntersendingTime(0.000001); //do not pace intial packets as RTT is unknown
    state->ssthresh = 7300;
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

void OrbtcpNoSharedFlowsFlavour::rttMeasurementComplete(simtime_t tSent, simtime_t tAcked)
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

void OrbtcpNoSharedFlowsFlavour::receivedDataAck(uint32_t firstSeqAcked, IntDataVec intData)
{
    TcpTahoeRenoFamily::receivedDataAck(firstSeqAcked);
    EV_INFO << "\nORBTCPInfo ___________________________________________" << endl;
    EV_INFO << "\nORBTCPInfo - Received Data Ack" << endl;
    if (state->lossRecovery && state->sack_enabled) {
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

void OrbtcpNoSharedFlowsFlavour::receivedDuplicateAck(uint32_t firstSeqAcked, IntDataVec intData)
{
    bool isHighRxtLost = dynamic_cast<TcpPacedConnection*>(conn)->checkIsLost(state->snd_una+state->snd_mss);
    bool rackLoss = dynamic_cast<TcpPacedConnection*>(conn)->checkRackLoss();
    if ((rackLoss && !state->lossRecovery) || state->dupacks == state->dupthresh || (isHighRxtLost && !state->lossRecovery)) {
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
            if (state->recoveryPoint == 0 || seqGE(state->snd_una, state->recoveryPoint)) { // HighACK = snd_una
                state->recoveryPoint = state->snd_max; // HighData = snd_max
                state->lossRecovery = true;
                dynamic_cast<TcpPacedConnection*>(conn)->setSackedHeadLost();
                dynamic_cast<TcpPacedConnection*>(conn)->updateInFlight();
                //std::cout << "\n Entering Loss recovery - dup acks > dupthresh at simTime: " << simTime().dbl() << endl;
                EV_DETAIL << " recoveryPoint=" << state->recoveryPoint;
                dynamic_cast<TcpPacedConnection*>(conn)->doRetransmit();
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
    else if (state->dupacks > state->dupthresh) {
        EV_INFO << "dupAcks > DUPTHRESH(=" << state->dupthresh << ": Fast Recovery: inflating cwnd by SMSS, new cwnd=" << state->snd_cwnd << "\n";
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

double OrbtcpNoSharedFlowsFlavour::measureInflight(IntDataVec intData)
{
    TcpPacedConnection::RateSample rs = dynamic_cast<TcpPacedConnection*>(conn)->getRateSample();

    double u = 0;
    double tau;
    double bottleneckAverageRtt;
    double bottleneckBandwidth;
    double bottleneckTxRate;
    double totalQueueingDelay = 0;

    uint32_t bottleneckTxBytes;
    double bottleneckRtt;
    double bottleneckSharingFlows;
    double bottleneckInitPhaseFlows;
    uint32_t bottleneckQueueing;
    bool bottleneckIsPastAck = false;
    std::vector<bool> currPathId(16);

    bool sharedHop = false;
    if(intData.size() == state->L.size()){
        for(int i = 0; i < intData.size(); i++){ //Start at front of queue. First item is first hop etc.
            IntMetaData* intDataEntry = intData.at(i);
            double uPrime = 0;
            sharedHop = true;
            bool isPastAck = false;
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
                if(intDataEntry->getHopId() == state->L.at(i)->getHopId() && intDataEntry->getAverageRtt() > 0) {
                    //std::bitset<1> b = (a1 ^= a2);
                    totalQueueingDelay +=(double)intDataEntry->getRxQlen()/(double)intDataEntry->getB();
                    //txRate is bytes observed at router between previous and current ACK packet subtracted from the timestamp of the previous and current ack. Equals estimated rate.
                    double hopTxRate = (intDataEntry->getTxBytes() - state->L.at(i)->getTxBytes())/(intDataEntry->getTs().dbl() - state->L.at(i)->getTs().dbl());
                    uPrime = (std::min(intDataEntry->getQLen(), state->L.at(i)->getQLen())/(intDataEntry->getB()*intDataEntry->getAverageRtt()))+(hopTxRate/intDataEntry->getB());
                    if(intDataEntry->getTs().dbl() < state->L.at(i)->getTs().dbl()) {
                        isPastAck = true;
                    }

                    if(uPrime > u) {
                        u = uPrime;
                        tau = intDataEntry->getTs().dbl() - state->L.at(i)->getTs().dbl();

                        bottleneckSharingFlows = intDataEntry->getNumOfFlows();
                        bottleneckInitPhaseFlows = intDataEntry->getNumOfFlowsInInitialPhase();
                        bottleneckAverageRtt = intDataEntry->getAverageRtt();
                        bottleneckRtt = intDataEntry->getTs().dbl() - state->L.at(i)->getTs().dbl();
                        bottleneckQueueing = (std::min(intDataEntry->getQLen(), state->L.at(i)->getQLen())/(intDataEntry->getB()*intDataEntry->getAverageRtt()));
                        bottleneckTxRate = hopTxRate;
                        bottleneckIsPastAck = isPastAck;
                        bottleneckTxBytes = intDataEntry->getTxBytes() - state->L.at(i)->getTxBytes();
                        if(bottleneckAverageRtt <= 0){
                            bottleneckAverageRtt = estimatedRtt.dbl();
                            EV_DEBUG << "bottleneckAverageRtt is lower or equal to 0!\n";
                        }
                        bottleneckBandwidth = intDataEntry->getB();
                    }
                }
            }
        }
    }

    if(bottleneckIsPastAck || pathChanged){
        return 0;
    }

    if(pathId.empty()) {
        pathId = currPathId;
    }
    else if((pathId != currPathId)){
        //updateNext = true;
        pathId = currPathId;
        pathChanged = true;
        //state->L = std::vector<IntMetaData*>(); //reset
        state->L = intData;
        return 0;
    }

    state->sharingFlows = bottleneckSharingFlows;
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

    if(state->useHpccAlpha){
        state->alpha = tau/bottleneckAverageRtt;
    }

    if(state->u == 0){
        state->u = u;
    }

    state->u = (1-state->alpha)*state->u+state->alpha*u;
    conn->emit(alphaSignal, state->alpha);
    conn->emit(USignal, state->u);

    state->additiveIncrease = (((bottleneckBandwidth) * rtt.dbl()) * state->additiveIncreasePercent);

    conn->emit(testRttSignal, bottleneckRtt);
    conn->emit(bottleneckBandwidthSignal, bottleneckBandwidth);
    conn->emit(avgRttSignal, bottleneckAverageRtt);
    conn->emit(additiveIncreaseSignal, state->additiveIncrease);
    conn->emit(ssthreshSignal, state->ssthresh);
    return state->u;
}

uint32_t OrbtcpNoSharedFlowsFlavour::computeWnd(double u, bool updateWc)
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
        conn->emit(txRateSignal, state->txRate);
    }

    return targetW;
}

size_t OrbtcpNoSharedFlowsFlavour::getConnId()
{
    return connId;
}

simtime_t OrbtcpNoSharedFlowsFlavour::getRtt()
{
    return state->srtt;
}

simtime_t OrbtcpNoSharedFlowsFlavour::getEstimatedRtt()
{
    return smoothedEstimatedRtt;
}

void OrbtcpNoSharedFlowsFlavour::processRexmitTimer(TcpEventCode &event) {
    TcpPacedFamily::processRexmitTimer(event);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
                   << ", ssthresh=" << state->ssthresh << "\n";

    state->afterRto = true;
    dynamic_cast<OrbtcpConnection*>(conn)->cancelPaceTimer();
    sendData(false);
}

void OrbtcpNoSharedFlowsFlavour::processTimer(cMessage *timer, TcpEventCode& event)
{
    if(timer == reactTimer){
        updateWindow = true;
        pathChanged = false;
        conn->scheduleAt(simTime() + state->srtt.dbl(), reactTimer);
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


} // namespace tcp
} // namespace inet

