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

#include <inet/networklayer/ipv4/Ipv4Header_m.h>
#include <inet/transportlayer/tcp_common/TcpHeader_m.h>
#include <inet/common/PacketEventTag.h>
#include <inet/common/TimeTag.h>
#include <inet/networklayer/common/NetworkInterface.h>
#include "../../common/IntTag_m.h"
#include "IntQueue.h"

namespace inet {
namespace queueing {

Define_Module(IntQueue);

simsignal_t IntQueue::avgRttSignal = cComponent::registerSignal("avgRttQueue");
simsignal_t IntQueue::numberOfFlowsSignal = cComponent::registerSignal("numberOfFlows");
simsignal_t IntQueue::persistentQueueingDelaySignal = cComponent::registerSignal("persistentQueueingDelay");

void IntQueue::initialize(int stage)
{
    PacketQueue::initialize(stage);
    txBytes = 0;
    sumRttByCwnd = 0;
    sumRttSquareByCwnd = 0;
    persistentQueueSize = 2147483647;
    avgRtt = 0;
    avgRttTimer = SimTime(10, SIMTIME_MS);
    fixedAvgRTTVal = par("fixedAvgRTTVal");
    if (stage == INITSTAGE_TRANSPORT_LAYER) {
        averageRttTimerMsg = new cMessage("averageRttTimerMsg");
        averageRttTimerMsg->setContextPointer(this);
        scheduleTimer();
    }
}

void IntQueue::finish()
{
    if (averageRttTimerMsg->isScheduled()) {
        cancelEvent(averageRttTimerMsg);
    }
    delete averageRttTimerMsg;
}

void IntQueue::handleMessage(cMessage *message)
{
    if (message == averageRttTimerMsg) {
        processTimer();
    }
    else{
        auto packet = check_and_cast<Packet *>(message);
        pushPacket(packet, packet->getArrivalGate());
    }
}

void IntQueue::processTimer()
{
    if(sumRttSquareByCwnd > 0 && sumRttByCwnd > 0){
        double queueingDelay = 0;
        if(persistentQueueSize != 2147483647){
            queueingDelay = (persistentQueueSize / dynamic_cast<NetworkInterface*>(getParentModule())->getRxTransmissionChannel()->getNominalDatarate()/8);
            //cSimpleModule::emit(persistentQueueingDelaySignal, (persistentQueueSize / dynamic_cast<NetworkInterface*>(getParentModule())->getRxTransmissionChannel()->getNominalDatarate()/8));
        }
        queueingDelay = (queue.getByteLength() / dynamic_cast<NetworkInterface*>(getParentModule())->getRxTransmissionChannel()->getNominalDatarate()/8);
        cSimpleModule::emit(persistentQueueingDelaySignal, queueingDelay);
        if(fixedAvgRTTVal > 0){
            avgRtt = fixedAvgRTTVal;
        }
        else{
            avgRtt = SimTime(sumRttSquareByCwnd/sumRttByCwnd) - queueingDelay;
        }
        numbOfFlows = flowIds.size();
        sumRttSquareByCwnd = 0;
        sumRttByCwnd = 0;
        changePersistentQueueSize = true;
        flowIds.clear();
        cSimpleModule::emit(avgRttSignal, avgRtt);
        cSimpleModule::emit(numberOfFlowsSignal, numbOfFlows);
    }
    scheduleTimer();
}

void IntQueue::scheduleTimer()
{
    if(!averageRttTimerMsg->isScheduled()){
        if(avgRtt > 0){
            avgRttTimer = avgRtt;
        }
        scheduleAt(simTime()+avgRttTimer, averageRttTimerMsg);
    }
}

void IntQueue::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    cNamedObject packetPushStartedDetails("atomicOperationStarted");
    emit(packetPushStartedSignal, packet, &packetPushStartedDetails);
    EV_INFO << "Pushing packet" << EV_FIELD(packet) << EV_ENDL;

    auto ipv4Header = packet->removeAtFront<Ipv4Header>();
    if (ipv4Header->getTotalLengthField() < packet->getDataLength())
    packet->setBackOffset(B(ipv4Header->getTotalLengthField()) - ipv4Header->getChunkLength());
    auto tcpHeader = packet->removeAtFront<tcp::TcpHeader>();
    if(tcpHeader->findTag<IntTag>()){
        if(tcpHeader->getTag<IntTag>()->getRtt().dbl() > 0 && tcpHeader->getTag<IntTag>()->getCwnd() > 0){
            sumRttByCwnd += tcpHeader->getTag<IntTag>()->getRtt().dbl() * packet->getByteLength() / tcpHeader->getTag<IntTag>()->getCwnd();
            sumRttSquareByCwnd += tcpHeader->getTag<IntTag>()->getRtt().dbl() * tcpHeader->getTag<IntTag>()->getRtt().dbl() * packet->getByteLength() / tcpHeader->getTag<IntTag>()->getCwnd();
        }
        flowIds.insert(tcpHeader->getTag<IntTag>()->getConnId());
        //std::cout << "\n INT QUEUE CWND: " << tcpHeader->getTag<IntTag>()->getCwnd() << endl;

    }

    if(packet->getDataLength() > b(0)) { //Data Packet
            tcpHeader->addTagIfAbsent<IntTag>();
            IntMetaData* intData = new IntMetaData();
            intData->setRxQlen(queue.getByteLength());
            tcpHeader->addTagIfAbsent<IntTag>()->getIntDataForUpdate().push_back(intData);
    }

    if(queue.getByteLength() < persistentQueueSize || changePersistentQueueSize){
        persistentQueueSize = queue.getByteLength();
        changePersistentQueueSize = false;
    }

    packet->insertAtFront(tcpHeader);
    ipv4Header->setTotalLengthField(ipv4Header->getChunkLength() + packet->getDataLength());
    packet->insertAtFront(ipv4Header);

    queue.insert(packet);
    if (buffer != nullptr)
        buffer->addPacket(packet);
    else if (packetDropperFunction != nullptr) {
        while (isOverloaded()) {
            auto packet = packetDropperFunction->selectPacket(this);
            EV_INFO << "Dropping packet" << EV_FIELD(packet) << EV_ENDL;
            queue.remove(packet);
            dropPacket(packet, QUEUE_OVERFLOW);
        }
    }
    ASSERT(!isOverloaded());
    if (collector != nullptr && getNumPackets() != 0)
        collector->handleCanPullPacketChanged(outputGate->getPathEndGate());
    cNamedObject packetPushEndedDetails("atomicOperationEnded");
    emit(packetPushEndedSignal, nullptr, &packetPushEndedDetails);
    updateDisplayString();
}

Packet *IntQueue::pullPacket(cGate *gate)
{
    Enter_Method("pullPacket");
    auto packet = check_and_cast<Packet *>(queue.front());
    EV_INFO << "Pulling packet" << EV_FIELD(packet) << EV_ENDL;
    if (buffer != nullptr) {
        queue.remove(packet);
        buffer->removePacket(packet);
    }
    else
        queue.pop();
    auto queueingTime = simTime() - packet->getArrivalTime();
    auto packetEvent = new PacketQueuedEvent();
    packetEvent->setQueuePacketLength(getNumPackets());
    packetEvent->setQueueDataLength(getTotalLength());
    insertPacketEvent(this, packet, PEK_QUEUED, queueingTime, packetEvent);
    increaseTimeTag<QueueingTimeTag>(packet, queueingTime, queueingTime);

    auto ipv4Header = packet->removeAtFront<Ipv4Header>();
    if (ipv4Header->getTotalLengthField() < packet->getDataLength())
        packet->setBackOffset(B(ipv4Header->getTotalLengthField()) - ipv4Header->getChunkLength());
    auto tcpHeader = packet->removeAtFront<tcp::TcpHeader>();
    txBytes += packet->getByteLength();
    if(packet->getDataLength() > b(0)) { //Data Packet
        IntMetaData* intData = tcpHeader->addTagIfAbsent<IntTag>()->getIntDataForUpdate().back();
        intData->setAverageRtt(avgRtt.dbl());
        intData->setNumOfFlows(numbOfFlows);

        intData->setHopName(getParentModule()->getParentModule()->getFullName());
        intData->setQLen(queue.getByteLength());
        intData->setTs(simTime());
        intData->setTxBytes(txBytes);
        intData->setB(dynamic_cast<NetworkInterface*>(getParentModule())->getRxTransmissionChannel()->getNominalDatarate()/8);
        //tcpHeader->addTagIfAbsent<IntTag>()->getIntDataForUpdate().push_back(intData);
    }
    packet->insertAtFront(tcpHeader);
    ipv4Header->setTotalLengthField(ipv4Header->getChunkLength() + packet->getDataLength());
    packet->insertAtFront(ipv4Header);

    emit(packetPulledSignal, packet);
    animatePullPacket(packet, outputGate);
    updateDisplayString();

    return packet;
}

} // namespace queueing
} // namespace inet
