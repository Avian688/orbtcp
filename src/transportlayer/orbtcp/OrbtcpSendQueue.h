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

#ifndef TRANSPORTLAYER_ORBTCP_ORBTCPSENDQUEUE_H_
#define TRANSPORTLAYER_ORBTCP_ORBTCPSENDQUEUE_H_

#include <inet/transportlayer/tcp/TcpSendQueue.h>
#include <inet/common/TimeTag_m.h>
#include "../../common/IntTag_m.h"

namespace inet {
namespace tcp {

class OrbtcpSendQueue : public TcpSendQueue {
public:
    virtual Packet *createSegmentWithBytes(uint32_t fromSeq, uint32_t numBytes) override;

    virtual void discardUpTo(uint32_t seqNum) override;
};

}
}
#endif /* TRANSPORTLAYER_ORBTCP_ORBTCPSENDQUEUE_H_ */
