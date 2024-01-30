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

#ifndef APPLICATIONS_TCPAPP_ORBTCPSESSIONAPP_H_
#define APPLICATIONS_TCPAPP_ORBTCPSESSIONAPP_H_

#include <inet/applications/tcpapp/TcpSessionApp.h>
#include "../../common/IntTag_m.h"
namespace inet {

/**
 * Single-connection OrbTCP application.
 */
class OrbtcpSessionApp : public TcpSessionApp
{
protected:
    virtual Packet *createDataPacket(long sendBytes) override;
};

} // namespace inet

#endif

