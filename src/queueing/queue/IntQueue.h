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

    long txBytes;
    simtime_t avgRtt;
    int numbOfFlows;
    simtime_t avgRttTimer;
    cMessage *averageRttTimerMsg = nullptr;
    //std::map<std::string, simtime_t> rtts;
    std::set<long> flowIds;
    double sumRttByCwnd;
    double sumRttSquareByCwnd;
    long persistentQueueSize;
    bool changePersistentQueueSize;
    double fixedAvgRTTVal;

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void processTimer();
    virtual void scheduleTimer();

    virtual void finish() override;
public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual Packet *pullPacket(cGate *gate) override;
};

} // namespace queueing
}// namespace inet

#endif /* QUEUEING_QUEUE_INTQUEUE_H_ */
