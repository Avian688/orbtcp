//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <algorithm> // min,max

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
simsignal_t OrbtcpFlavour::avgEstimatedRttSignal = cComponent::registerSignal("avgEstimatedRtt");
simsignal_t OrbtcpFlavour::alphaSignal = cComponent::registerSignal("alpha");
simsignal_t OrbtcpFlavour::measuringInflightSignal = cComponent::registerSignal("measuringInflight");

simsignal_t OrbtcpFlavour::testRttSignal = cComponent::registerSignal("testRtt");

simsignal_t OrbtcpFlavour::sndUnaSignal = cComponent::registerSignal("sndUna");
simsignal_t OrbtcpFlavour::sndMaxSignal = cComponent::registerSignal("sndMax");

simsignal_t OrbtcpFlavour::recoveryPointSignal = cComponent::registerSignal("recoveryPoint");

OrbtcpFlavour::OrbtcpFlavour() : OrbtcpFamily(),
    state((OrbtcpStateVariables *&)TcpAlgorithm::state)
{
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

    state->alpha = conn->getTcpMain()->par("alpha");
    if(state->alpha > 0){
        state->useHpccAlpha = false;
    }
    else{
        state->useHpccAlpha = true;
    }
}

void OrbtcpFlavour::established(bool active)
{
    //state->snd_cwnd = state->B * state->T.dbl();
    state->snd_cwnd = 7300; //5 packets
    dynamic_cast<OrbtcpConnection*>(conn)->changeIntersendingTime(0.000001); //do not pace intial packets as RTT is unknown
    state->ssthresh = 1215752192;
    connId = std::hash<std::string>{}(conn->localAddr.str() + "/" + std::to_string(conn->localPort) + "/" + conn->remoteAddr.str() + "/" + std::to_string(conn->remotePort));
    initPackets = true;
    EV_DETAIL << "OrbTCP initial CWND is set to " << state->snd_cwnd << "\n";
    if (active) {
        // finish connection setup with ACK (possibly piggybacked on data)
        EV_INFO << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";
        if (!sendData(false)) // FIXME - This condition is never true because the buffer is empty (at this time) therefore the first ACK is never piggyback on data
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

    if(state->srtt == 1){
        state->srtt = newRTT;
    }

    simtime_t& srtt = state->srtt;
    simtime_t& rttvar = state->rttvar;

    simtime_t err = newRTT - srtt;

    srtt += g * err;
    rttvar += g * (fabs(err) - rttvar);

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
    estimatedRtt = rtt - state->queueingDelay;

    if(estimatedRtt <= 0){
        estimatedRtt = rtt;
    }
    conn->emit(rttSignal, newRTT);
    conn->emit(estimatedRttSignal, newRTT-state->queueingDelay);
    conn->emit(srttSignal, srtt);
    conn->emit(rttvarSignal, rttvar);
    conn->emit(rtoSignal, rto);
}

void OrbtcpFlavour::receivedDataAck(uint32_t firstSeqAcked, IntDataVec intData)
{
    EV_INFO << "\nORBTCPInfo ___________________________________________" << endl;
    EV_INFO << "\nORBTCPInfo - Received Data Ack" << endl;

    TcpTahoeRenoFamily::receivedDataAck(firstSeqAcked);

    if(firstSeqAcked > state->lastUpdateSeq) {
        double uVal = measureInflight(intData);
        if(uVal > 0){
            state->snd_cwnd = computeWnd(uVal, true);
        }
        conn->emit(cwndSignal, state->snd_cwnd);
        state->lastUpdateSeq = state->snd_nxt;
    }
    else {
        double uVal = measureInflight(intData);
        if(uVal > 0){
            state->snd_cwnd = computeWnd(uVal, false);
        }
        conn->emit(cwndSignal, state->snd_cwnd);
    }
    if(state->snd_cwnd > 0){
        double pace = state->srtt.dbl()/((double) (state->snd_cwnd*1.2)/(double)state->snd_mss);
        dynamic_cast<OrbtcpConnection*>(conn)->changeIntersendingTime(pace);
    }
    state->L = intData;

    // Check if recovery phase has ended
    if (state->sack_enabled && state->lossRecovery) {
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
                // update of scoreboard (B.1) has already be done in readHeaderOptions()
                //conn->setPipe();
            }
            conn->emit(recoveryPointSignal, state->recoveryPoint);
        }
        sendData(false);

        conn->emit(sndUnaSignal, state->snd_una);
        conn->emit(sndMaxSignal, state->snd_max);
}

void OrbtcpFlavour::receivedDuplicateAck(uint32_t firstSeqAcked, IntDataVec intData)
{
    TcpTahoeRenoFamily::receivedDuplicateAck();

    bool isHighRxtLost = dynamic_cast<TcpPacedConnection*>(conn)->checkIsLost(dynamic_cast<TcpPacedConnection*>(conn)->getHighestRexmittedSeqNum());
    if (state->dupacks == state->dupthresh || isHighRxtLost) {
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
                dynamic_cast<TcpPacedConnection*>(conn)->setSackedHeadLost();
                dynamic_cast<TcpPacedConnection*>(conn)->updateInFlight();
                //std::cout << "\n Entering Loss recovery - dup acks > dupthresh at simTime: " << simTime().dbl() << endl;
                state->lossRecovery = true;
                EV_DETAIL << " recoveryPoint=" << state->recoveryPoint;

                dynamic_cast<TcpPacedConnection*>(conn)->retransmitNext(false);
            }
        }

        conn->emit(cwndSignal, state->snd_cwnd);

        EV_DETAIL << " set cwnd=" << state->snd_cwnd << ", ssthresh=" << state->ssthresh << "\n";

        // Fast Retransmission: retransmit missing segment without waiting
        // for the REXMIT timer to expire
        sendData(false);
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
        //
        // Reno: For each additional duplicate ACK received, increment cwnd by SMSS.
        // This artificially inflates the congestion window in order to reflect the
        // additional segment that has left the network
        //
        //state->snd_cwnd += state->snd_mss;
        //state->snd_cwnd += state->dupacks * state->snd_mss;
        EV_DETAIL << "Reno on dupAcks > DUPTHRESH(=" << state->dupthresh << ": Fast Recovery: inflating cwnd by SMSS, new cwnd=" << state->snd_cwnd << "\n";

        conn->emit(cwndSignal, state->snd_cwnd);

        // Note: Steps (A) - (C) of RFC 3517, page 7 ("Once a TCP is in the loss recovery phase the following procedure MUST be used for each arriving ACK")
        // should not be used here!

        // RFC 3517, pages 7 and 8: "5.1 Retransmission Timeouts
        // (...)
        // If there are segments missing from the receiver's buffer following
        // processing of the retransmitted segment, the corresponding ACK will
        // contain SACK information.  In this case, a TCP sender SHOULD use this
        // SACK information when determining what data should be sent in each
        // segment of the slow start.  The exact algorithm for this selection is
        // not specified in this document (specifically NextSeg () is
        // inappropriate during slow start after an RTO).  A relatively
        // straightforward approach to "filling in" the sequence space reported
        // as missing should be a reasonable approach."
    }

    if(firstSeqAcked > state->lastUpdateSeq) {
        //std::cout << "\n UPDATING CWND GIVEN DUP ACK" << endl;
        double uVal = measureInflight(intData);
        if(uVal > 0){
            state->snd_cwnd = computeWnd(uVal, true);
        }
        conn->emit(cwndSignal, state->snd_cwnd);
        state->lastUpdateSeq = state->snd_nxt;
    }
    else {
        double uVal = measureInflight(intData);
        if(uVal > 0){
            state->snd_cwnd = computeWnd(uVal, false);
        }
        conn->emit(cwndSignal, state->snd_cwnd);
    }
    state->L = intData;

    if(state->snd_cwnd > 0){
        double pace = state->srtt.dbl()/((double) (state->snd_cwnd*1.2)/(double)state->snd_mss);
        dynamic_cast<OrbtcpConnection*>(conn)->changeIntersendingTime(pace);
    }

    sendData(false);
}

double OrbtcpFlavour::measureInflight(IntDataVec intData)
{
    double u = 0;
    double tau;
    double bottleneckAverageRtt;
    double bottleneckBandwidth;
    double bottleneckTxRate;
    double totalQueueingDelay = 0;

    double bottleneckRtt;
    double bottleneckSharingFlows;
    double bottleneckInitPhaseFlows;
    bool bottleneckIsPastAck = false;
    std::vector<bool> currPathId(16);
    for(int i = 0; i < intData.size(); i++){ //Start at front of queue. First item is first hop etc.
        double uPrime = 0;
        IntMetaData* intDataEntry = intData.at(i);
        if(state->L.size() == intData.size()){ //TODO replace with check to ensure the hops are the same, maybe hopID? Look at paper/rfc

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

            if(intDataEntry->getAverageRtt() > 0) {
                initPackets = false;
            }
            else{
                return 0;
            }

            if(!initPackets){
                //std::bitset<1> b = (a1 ^= a2);
                totalQueueingDelay +=(double)intDataEntry->getRxQlen()/(double)intDataEntry->getB();
                //txRate is bytes observed at router between prev and current ACK packet subtracted from the timestamp of the prev and current ack. Equals estimated rate.
                double hopTxRate = (intDataEntry->getTxBytes() - state->L.at(i)->getTxBytes())/(intDataEntry->getTs().dbl() - state->L.at(i)->getTs().dbl());
                uPrime = (intDataEntry->getQLen())/(intDataEntry->getB()*intDataEntry->getAverageRtt())+(hopTxRate/intDataEntry->getB());
                if(intDataEntry->getTs().dbl() < state->L.at(i)->getTs().dbl()) {
                  //std::cout << "\n CURRENT ACK HOP TIMESTAMP IS OUT OF ORDER " << endl;
                    isPastAck = true;
                }

                if(uPrime > u) {
                    u = uPrime;
                    tau = intDataEntry->getTs().dbl() - state->L.at(i)->getTs().dbl();

                    bottleneckSharingFlows = intDataEntry->getNumOfFlows();
                    bottleneckInitPhaseFlows = intDataEntry->getNumOfFlowsInInitialPhase();
                    bottleneckAverageRtt = intDataEntry->getAverageRtt();
                    bottleneckRtt = intDataEntry->getTs().dbl() - state->L.at(i)->getTs().dbl();
                    bottleneckTxRate = hopTxRate;
                    bottleneckIsPastAck = isPastAck;
                    if(bottleneckAverageRtt <= 0){
                        bottleneckAverageRtt = estimatedRtt.dbl();
                        EV_DEBUG << "bottleneckAverageRtt is lower or equal to 0!\n";
                    }
                    bottleneckBandwidth = intDataEntry->getB();
                }
            }
        }
        else{

            int hopId = intDataEntry->getHopId();
            std::vector<bool> bitArray(16);

            std::vector<bool> tempBitArray(16);
            int bitSize = 16;

            for (size_t i = 0; i < bitSize; ++i) {
                bitArray[(bitSize-1) - i] = (hopId >> i) & 1;
            }

            for (size_t i = 0; i < bitArray.size(); ++i) {
                tempBitArray[i] = bitArray[i] ^ currPathId[i];  // XOR the corresponding bits
            }
            currPathId = tempBitArray;

            if(intDataEntry->getAverageRtt() > 0) {
                initPackets = false;
            }
            else{
                return 0;
            }

            double hopTxRate = intDataEntry->getTxBytes()/intDataEntry->getAverageRtt();
            totalQueueingDelay +=(double)(intDataEntry->getRxQlen())/(double)intDataEntry->getB();
            uPrime = (intDataEntry->getQLen()/(intDataEntry->getB()*intDataEntry->getAverageRtt()))+(hopTxRate/intDataEntry->getB());
            if(uPrime > u) {
                u = uPrime;
                tau = intDataEntry->getTs().dbl();
                bottleneckSharingFlows = intDataEntry->getNumOfFlows();
                bottleneckInitPhaseFlows = intDataEntry->getNumOfFlowsInInitialPhase();
                bottleneckAverageRtt = intDataEntry->getAverageRtt();
                bottleneckTxRate = hopTxRate;
                if(bottleneckAverageRtt <= 0){
                    bottleneckAverageRtt = estimatedRtt.dbl();
                    EV_DEBUG << "bottleneckAverageRtt is lower or equal to 0!\n";
                }
            }
        }
    }

    if(pathId.empty()) {
        pathId = currPathId;
    }
    else if(pathId != currPathId){
        pathId = currPathId;
        return state->u;
    }

    if(bottleneckIsPastAck){
        return state->u;
    }

    state->sharingFlows = bottleneckSharingFlows;
    state->initialPhaseSharingFlows = bottleneckInitPhaseFlows;

    state->txRate = bottleneckTxRate;
    state->bottBW = bottleneckBandwidth;
    state->queueingDelay = totalQueueingDelay;

    conn->emit(queueingDelaySignal, state->queueingDelay);
    conn->emit(avgEstimatedRttSignal, bottleneckAverageRtt);
    conn->emit(txRateSignal, bottleneckTxRate);
    conn->emit(uSignal, u);
    conn->emit(tauSignal, tau);
    conn->emit(sharingFlowsSignal, state->sharingFlows);

    if(state->useHpccAlpha){
        state->alpha = tau/bottleneckAverageRtt;
    }

    if(state->u == 0){
        state->u = u;
    }

    state->u = (1-state->alpha)*state->u+state->alpha*u;
    conn->emit(alphaSignal, state->alpha);
    conn->emit(USignal, state->u);

    state->ssthresh = ((((bottleneckBandwidth)/state->sharingFlows) * estimatedRtt.dbl()) * 0.95);
    if(!state->initialPhase || state->snd_cwnd > state->ssthresh){ //slow start - more aggressive till max allowed share is reached
        state->additiveIncrease = ((((bottleneckBandwidth)/state->sharingFlows) * rtt.dbl()) * state->additiveIncreasePercent);
        state->ssthresh = 0;
        state->initialPhase = false;
    }
    else{
        if(state->initialPhaseSharingFlows > 1){
            state->additiveIncrease = ((((bottleneckBandwidth)/state->initialPhaseSharingFlows) * rtt.dbl()) * state->additiveIncreasePercent);
        }
        else{
            state->additiveIncrease = (bottleneckBandwidth * rtt.dbl()) * state->additiveIncreasePercent/2;
        }
    }

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
        state->prevWnd = targetW;
    }
    return targetW;
}

size_t OrbtcpFlavour::getConnId()
{
    return connId;
}

simtime_t OrbtcpFlavour::getRtt()
{
    return estimatedRtt;
}

void OrbtcpFlavour::processRexmitTimer(TcpEventCode &event) {
    TcpPacedFamily::processRexmitTimer(event);


    conn->emit(cwndSignal, state->snd_cwnd);

    EV_INFO << "Begin Slow Start: resetting cwnd to " << state->snd_cwnd
                   << ", ssthresh=" << state->ssthresh << "\n";

    state->afterRto = true;
    dynamic_cast<OrbtcpConnection*>(conn)->cancelPaceTimer();

    dynamic_cast<OrbtcpConnection*>(conn)->retransmitNext(true);
    sendData(false);
}

} // namespace tcp
} // namespace inet

