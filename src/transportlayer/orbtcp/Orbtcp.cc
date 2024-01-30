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

#include "Orbtcp.h"

#include "../orbtcp/OrbtcpSendQueue.h"

namespace inet {
namespace tcp {

Define_Module(Orbtcp);

Orbtcp::Orbtcp() {
    // TODO Auto-generated constructor stub

}

Orbtcp::~Orbtcp() {
    // TODO Auto-generated destructor stub
}

TcpConnection* Orbtcp::createConnection(int socketId)
{
    auto moduleType = cModuleType::get("orbtcp.transportlayer.orbtcp.OrbtcpConnection");
    char submoduleName[24];
    sprintf(submoduleName, "conn-%d", socketId);
    auto module = check_and_cast<TcpConnection*>(moduleType->createScheduleInit(submoduleName, this));
    module->initConnection(this, socketId);
    return module;
}

TcpSendQueue *Orbtcp::createSendQueue()
{
    return new OrbtcpSendQueue();
}

}
}
