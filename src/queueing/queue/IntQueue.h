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

#ifndef QUEUEING_QUEUE_INTQUEUE_H_
#define QUEUEING_QUEUE_INTQUEUE_H_

#include <map>
#include "inet/queueing/queue/PacketQueue.h"

namespace inet {
namespace queueing {

class IntQueue : public PacketQueue {
protected:
    static simsignal_t avgRttSignal;
    static simsignal_t numberOfFlowsSignal;
    static simsignal_t persistentQueueingDelaySignal;
    static simsignal_t numOfFlowsInInitialPhaseSignal;
    static simsignal_t bandwidthSignal;
    static simsignal_t txBytesSignal;


    simtime_t bandwidthRecorderTimer;
    cMessage *bandwidthRecorderTimerMsg = nullptr;
    int bandwidth;
    bool isActive;
    long txBytes;
    simtime_t avgRtt;
    int numbOfFlows;
    int numOfFlowsInInitialPhase;
    simtime_t avgRttTimer;
    cMessage *averageRttTimerMsg = nullptr;
    //std::map<std::string, simtime_t> rtts;
    std::set<long> flowIds;
    std::set<long> initialPhaseFlowIds;
    double sumRttByCwnd;
    double sumRttSquareByCwnd;
    long persistentQueueSize;
    bool changePersistentQueueSize;
    double fixedAvgRTTVal;

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void processTimer();
    virtual void processBWTimer();
    virtual void scheduleTimer();
    virtual void scheduleBWTimer();

    virtual ~IntQueue();
public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual Packet *pullPacket(cGate *gate) override;
};

} // namespace queueing
}// namespace inet

#endif /* QUEUEING_QUEUE_INTQUEUE_H_ */
