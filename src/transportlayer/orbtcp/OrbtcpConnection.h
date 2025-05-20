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
#include <inet/networklayer/common/EcnTag_m.h>
#include <inet/transportlayer/common/L4Tools.h>
#include <inet/networklayer/common/DscpTag_m.h>
#include <inet/networklayer/common/HopLimitTag_m.h>
#include <inet/networklayer/common/TosTag_m.h>
#include <inet/networklayer/common/L3AddressTag_m.h>
#include <inet/networklayer/contract/IL3AddressType.h>
#include "../../common/IntTag_m.h"
#include "../../../../tcpPaced/src/transportlayer/tcp/TcpPacedConnection.h"

namespace inet {
namespace tcp {
class OrbtcpConnection : public TcpPacedConnection {
public:
    OrbtcpConnection();
    virtual ~OrbtcpConnection();
protected:
    virtual TcpEventCode processSegment1stThru8th(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader) override;
    virtual bool processAckInEstabEtc(Packet *tcpSegment, const Ptr<const TcpHeader>& tcpHeader) override;

    virtual void initConnection(TcpOpenCommand *openCmd) override;
    virtual void initClonedConnection(TcpConnection *listenerConn) override;
    virtual TcpConnection *cloneListeningConnection() override;
public:
    virtual uint32_t sendSegment(uint32_t bytes) override;
public:
    virtual void sendIntAck(IntDataVec intData);
public:
    std::queue<Packet*> packetQueue;

};

}
}

#endif /* TRANSPORTLAYER_ORBTCP_ORBTCPCONNECTION_H_ */
