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

#ifndef TRANSPORTLAYER_ORBTCP_ORBTCP_H_
#define TRANSPORTLAYER_ORBTCP_ORBTCP_H_

#include <inet/transportlayer/tcp/Tcp.h>
#include <inet/transportlayer/tcp/TcpConnection.h>

namespace inet {
namespace tcp {

class Orbtcp : public Tcp {
public:
    Orbtcp();
    virtual ~Orbtcp();
protected:
    /** Factory method; may be overriden for customizing Tcp */
    virtual TcpConnection* createConnection(int socketId) override;
public:
    virtual TcpSendQueue *createSendQueue() override;
};

} // namespace tcp
} // namespace inet

#endif /* TRANSPORTLAYER_ORBTCP_ORBTCP_H_ */
