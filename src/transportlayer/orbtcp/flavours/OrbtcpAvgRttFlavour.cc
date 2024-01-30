//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <algorithm> // min,max

#include "inet/transportlayer/tcp/Tcp.h"
#include "OrbtcpAvgRttFlavour.h"

namespace inet {
namespace tcp {

#define MIN_REXMIT_TIMEOUT     1.0   // 1s
#define MAX_REXMIT_TIMEOUT     240   // 2 * MSL (RFC 1122)

Register_Class(OrbtcpAvgRttFlavour);

simsignal_t OrbtcpAvgRttFlavour::txRateSignal = cComponent::registerSignal("txRate");
simsignal_t OrbtcpAvgRttFlavour::tauSignal = cComponent::registerSignal("tau");
simsignal_t OrbtcpAvgRttFlavour::uSignal = cComponent::registerSignal("u");
simsignal_t OrbtcpAvgRttFlavour::USignal = cComponent::registerSignal("U");
simsignal_t OrbtcpAvgRttFlavour::additiveIncreaseSignal = cComponent::registerSignal("additiveIncrease");
simsignal_t OrbtcpAvgRttFlavour::sharingFlowsSignal = cComponent::registerSignal("sharingFlows");
simsignal_t OrbtcpAvgRttFlavour::bottleneckBandwidthSignal = cComponent::registerSignal("bottleneckBandwidth");
simsignal_t OrbtcpAvgRttFlavour::avgRttSignal = cComponent::registerSignal("avgRtt");
simsignal_t OrbtcpAvgRttFlavour::queueingDelaySignal = cComponent::registerSignal("queueingDelay");
simsignal_t OrbtcpAvgRttFlavour::estimatedRttSignal = cComponent::registerSignal("estimatedRtt");

OrbtcpAvgRttFlavour::OrbtcpAvgRttFlavour() : OrbtcpFamily(),
    state((OrbtcpAvgRttStateVariables *&)TcpAlgorithm::state)
{
}

void OrbtcpAvgRttFlavour::initialize()
{
    OrbtcpFamily::initialize();
    state->B = conn->getTcpMain()->par("bandwidth");
    state->subFlows = conn->getTcpMain()->par("subFlows");
    state->sharingFlows = conn->getTcpMain()->par("sharingFlows");
    state->additiveIncreasePercent = conn->getTcpMain()->par("additiveIncreasePercent");
    state->eta = conn->getTcpMain()->par("eta");
    state->T = conn->getTcpMain()->par("basePropagationRTT");
    state->u = 0;
    state->queueingDelay = 0;
    //TODO add Par for number of N. Currently is 10 meaning 10 flows. Look at paper for wAI
    //state->additiveIncrease = ((state->B * state->T.dbl())*(1-state->eta))/state->sharingFlows;
    state->additiveIncrease = 1;
    //std::cout << "\n additiveIncrease factor: " << state->additiveIncrease << endl;
    //state->prevWnd = state->B * state->T.dbl();
    state->prevWnd = 10000;
}

void OrbtcpAvgRttFlavour::established(bool active)
{
    //state->snd_cwnd = state->B * state->T.dbl();
    state->snd_cwnd = 10000;
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

void OrbtcpAvgRttFlavour::rttMeasurementComplete(simtime_t tSent, simtime_t tAcked)
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

    // record statistics
    EV_DETAIL << "Measured RTT=" << (newRTT * 1000) << "ms, updated SRTT=" << (srtt * 1000)
              << "ms, new RTO=" << (rto * 1000) << "ms\n";

    rtt = newRTT;
    conn->emit(rttSignal, newRTT);
    conn->emit(srttSignal, srtt);
    conn->emit(rttvarSignal, rttvar);
    conn->emit(rtoSignal, rto);
}

void OrbtcpAvgRttFlavour::receivedDataAckInt(uint32_t firstSeqAcked, IntDataVec intData)
{
    EV_INFO << "\nORBTCPInfo ___________________________________________" << endl;
    EV_INFO << "\nORBTCPInfo - Received Data Ack" << endl;

//    if(simTime().dbl() >= 2.5){
//        state->T = 0.01;
//        state->additiveIncrease = ((state->B * state->T.dbl())*(1-state->eta))/2;
//    }
    TcpTahoeRenoFamily::receivedDataAck(firstSeqAcked);

//    std::cout << "\nInt Data size: " << intData.size() << endl;
//    for(int i = 0; i < intData.size(); i++){
//        IntMetaData* intDataEntry = intData.front();
//        std::cout << "\nData Ack has INT data!" << endl;
//        std::cout << "Queue Length: " << intDataEntry->getQLen() << endl;
//        std::cout << "Timestamp: " << intDataEntry->getTs() << endl;
//    }
//    std::cout << "\nMeasuring inflight u: " << measureInflight(intData) << endl;

    if (state->dupacks >= state->dupthresh) {
        //
        // Perform Fast Recovery: set cwnd to ssthresh (deflating the window).
        //
        EV_INFO << "Fast Recovery: setting cwnd to ssthresh=" << state->ssthresh << "\n";
        state->snd_cwnd = state->ssthresh;
        conn->emit(cwndSignal, state->snd_cwnd);
    }
    else {
        if(firstSeqAcked > state->lastUpdateSeq) {
            //std::cout << "\n firstSeqAcked: " << firstSeqAcked << endl;
            //std::cout << "\n state->lastUpdateSeq: " << state->lastUpdateSeq << endl;
            double uVal = measureInflight(intData);
            if(uVal > 0){
                state->snd_cwnd = computeWnd(uVal, true);
                //state->ssthresh = state->snd_cwnd / 2;
            }
            conn->emit(cwndSignal, state->snd_cwnd);
            state->lastUpdateSeq = state->snd_nxt;
        }
        else {
            double uVal = measureInflight(intData);
            if(uVal > 0){
                state->snd_cwnd = computeWnd(uVal, false);
                //state->ssthresh = state->snd_cwnd / 2;
            }
            conn->emit(cwndSignal, state->snd_cwnd);
        }
    }

    state->L = intData;

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
                conn->setPipe();

                // RFC 3517, page 7: "(C) If cwnd - pipe >= 1 SMSS the sender SHOULD transmit one or more
                // segments as follows:"
                if (((int)state->snd_cwnd - (int)state->pipe) >= (int)state->snd_mss) // Note: Typecast needed to avoid prohibited transmissions
                    conn->sendDataDuringLossRecoveryPhase(state->snd_cwnd);
            }
        }
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
        sendData(false);
}

double OrbtcpAvgRttFlavour::measureInflight(IntDataVec intData)
{
    double u = 0;
    double tau;
    double bottleneckAverageRtt;
    double bottleneckBandwidth;

    double totalQueueingDelay = 0;
    for(int i = 0; i < intData.size(); i++){ //Start at front of queue. First item is first hop etc.
        double uPrime = 0;
        IntMetaData* intDataEntry = intData.at(i);

        if(state->L.size() == intData.size()){ //TODO replace with check to ensure the hops are the same, maybe hopID? Look at paper/rfc

            if(intDataEntry->getAverageRtt() > 0) {
                initPackets = false;
            }
            else{
                return 0;
            }

            if(!initPackets){

                totalQueueingDelay += ((double)(std::min(intDataEntry->getQLen(), state->L.at(i)->getQLen()))/1460)/(double)(intDataEntry->getB()/1460);

                state->txRate = (intDataEntry->getTxBytes() - state->L.at(i)->getTxBytes())/(intDataEntry->getTs().dbl() - state->L.at(i)->getTs().dbl());
                uPrime = ((std::min(intDataEntry->getQLen(), state->L.at(i)->getQLen()))/(intDataEntry->getB()*intDataEntry->getAverageRtt()))+(state->txRate/intDataEntry->getB());
                if(uPrime > u) {
                    u = uPrime;
                    tau = intDataEntry->getTs().dbl() - state->L.at(i)->getTs().dbl();
                    state->sharingFlows = intDataEntry->getNumOfFlows();
                    bottleneckAverageRtt = intDataEntry->getAverageRtt();
                    if(bottleneckAverageRtt <= 0){
                        bottleneckAverageRtt = state->srtt.dbl();
                    }
                    bottleneckBandwidth = intDataEntry->getB();
                }
            }
        }
        else{
            if(intDataEntry->getAverageRtt() > 0) {
                initPackets = false;
            }
            else{
                return 0;
            }

            state->txRate = intDataEntry->getTxBytes()/intDataEntry->getAverageRtt();
            totalQueueingDelay += intDataEntry->getQLen()/(intDataEntry->getB()/1460);
            uPrime = (intDataEntry->getQLen()/(intDataEntry->getB()*intDataEntry->getAverageRtt()))+(state->txRate/intDataEntry->getB());
            if(uPrime > u) {
                u = uPrime;
                tau = intDataEntry->getTs().dbl();
                bottleneckAverageRtt = intDataEntry->getAverageRtt();
                if(bottleneckAverageRtt <= 0){
                    bottleneckAverageRtt = state->srtt.dbl();
                }
            }
        }
    }
    state->queueingDelay = totalQueueingDelay;
    conn->emit(queueingDelaySignal, state->queueingDelay);
    conn->emit(estimatedRttSignal, rtt-state->queueingDelay);
    conn->emit(txRateSignal, state->txRate);
    conn->emit(uSignal, u);
    //tau = std::min(tau, state->T.dbl());
    //tau = state->srtt.dbl();
    conn->emit(tauSignal, tau);
    conn->emit(sharingFlowsSignal, state->sharingFlows);

    state->u = (1-(tau/bottleneckAverageRtt))*state->u+(tau/bottleneckAverageRtt)*u;
    conn->emit(USignal, state->u);

    state->additiveIncrease = ((bottleneckBandwidth * state->srtt.dbl() )*(state->additiveIncreasePercent))/state->sharingFlows;
    dynamic_cast<OrbtcpConnection*>(conn)->changeIntersendingTime(state->srtt.dbl()/((double) state->snd_cwnd/1460));

    //std::cout << "\n SSTHRESH VAL: " << state->ssthresh << endl;

    conn->emit(bottleneckBandwidthSignal, bottleneckBandwidth);
    conn->emit(avgRttSignal, bottleneckAverageRtt);
    conn->emit(additiveIncreaseSignal, state->additiveIncrease);

    return state->u;
}

uint32_t OrbtcpAvgRttFlavour::computeWnd(double u, bool updateWc)
{
    uint32_t w;
    //std::cout << "\n\n computeWnd..." << endl;
    //std::cout << "\n u value: " << u << endl;
    if(u >= state->eta || state->incStage >= state->maxStage) {
        w = (state->prevWnd/(u/state->eta))+state->additiveIncrease;
        if(updateWc) {
            state->incStage = 0;
            state->prevWnd = w;
        }
    }
    else {
        w = state->prevWnd + state->additiveIncrease;
        //std::cout << "\n Updating with just additive increase " << state->additiveIncrease << " to " << w << " at sim time: " << simTime() << endl;
        if(updateWc) {
            state->incStage++;
            state->prevWnd = w;
        }
    }
    //std::cout << "\n state->txRate: " << state->txRate << endl;
    //std::cout << "\n state->u: " << state->u << endl;
    //std::cout << "\n state->eta: " << state->eta << endl;
   // std::cout << "\n additiveIncrease val: " << state->additiveIncrease << endl;
    //std::cout << "\n computeWnd result: " << w << endl;
    return w;
}

size_t OrbtcpAvgRttFlavour::getConnId()
{
    return connId;
}

simtime_t OrbtcpAvgRttFlavour::getSrtt()
{
    //return state->srtt;
    return rtt;
    //return state->;
}

unsigned int OrbtcpAvgRttFlavour::getCwnd()
{
    return state->snd_cwnd;
}

} // namespace tcp
} // namespace inet

