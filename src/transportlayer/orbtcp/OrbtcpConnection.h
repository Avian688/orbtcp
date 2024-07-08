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

#ifndef TRANSPORTLAYER_ORBTCP_ORBTCPCONNECTION_H_
#define TRANSPORTLAYER_ORBTCP_ORBTCPCONNECTION_H_

#include <queue>
#include <inet/common/INETUtils.h>
#include <inet/transportlayer/tcp/TcpConnection.h>
#include <inet/networklayer/common/EcnTag_m.h>
#include <inet/transportlayer/common/L4Tools.h>
#include <inet/networklayer/common/DscpTag_m.h>
#include <inet/networklayer/common/HopLimitTag_m.h>
#include <inet/networklayer/common/TosTag_m.h>
#include <inet/networklayer/common/L3AddressTag_m.h>
#include <inet/networklayer/contract/IL3AddressType.h>
#include "../../common/IntTag_m.h"
namespace inet {
namespace tcp {
class OrbtcpConnection : public TcpConnection {
public:
    OrbtcpConnection();
    virtual ~OrbtcpConnection();
protected:
    virtual TcpEventCode processSegment1stThru8th(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader) override;
    virtual bool processAckInEstabEtc(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader) override;

    virtual void initConnection(TcpOpenCommand *openCmd) override;
    virtual void initClonedConnection(TcpConnection *listenerConn) override;
    virtual void configureStateVariables() override;
    virtual void process_SEND(TcpEventCode& event, TcpCommand *tcpCommand, cMessage *msg) override;
    virtual TcpConnection *cloneListeningConnection() override;
public:
    virtual bool processTimer(cMessage *msg) override;
    virtual uint32_t sendSegment(uint32_t bytes) override;
    virtual void sendToIP(Packet *packet, const Ptr<TcpHeader> &tcpseg) override;
    virtual void changeIntersendingTime(simtime_t _intersendingTime);
    virtual void setPipe() override;
private:
    virtual void processPaceTimer();
    void addPacket(Packet *packet);
    virtual Packet* addIntTags(Packet* packet);
public:
    virtual void sendIntAck(IntDataVec intData);
protected:
    cOutVector paceValueVec;
    cOutVector bufferedPacketsVec;
    bool pace;
    simtime_t paceStart;
public:
    std::queue<Packet*> packetQueue;
    cMessage *paceMsg;
    simtime_t intersendingTime;

};

}
}

#endif /* TRANSPORTLAYER_ORBTCP_ORBTCPCONNECTION_H_ */
