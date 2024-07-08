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

#ifndef INET_TRANSPORTLAYER_TCP_FLAVOURS_ORBTCPFAMILY_H_
#define INET_TRANSPORTLAYER_TCP_FLAVOURS_ORBTCPFAMILY_H_

#include "../../orbtcp/flavours/OrbtcpFamilyState_m.h"
#include "../OrbtcpConnection.h"
#include "inet/transportlayer/tcp/flavours/TcpTahoeRenoFamily.h"

namespace inet {
namespace tcp {
/**
 * Provides utility functions to implement Hpcc.
 */
class OrbtcpFamily : public TcpTahoeRenoFamily
{
  protected:
    OrbtcpFamilyStateVariables *& state; // alias to TcpAlgorithm's 'state'

  public:
    /** Ctor */
    OrbtcpFamily();

    virtual void receivedDataAck(uint32_t firstSeqAcked, IntDataVec intData);

    virtual void receiveSeqChanged(IntDataVec intData);

    virtual void receivedDuplicateAck(uint32_t firstSeqAcked, IntDataVec intData);

    virtual simtime_t getRtt();

    virtual unsigned int getCwnd();

    virtual size_t getConnId();

    virtual bool getInitialPhase();

    virtual void receivedOutOfOrderSegment(IntDataVec intData);


};

} // namespace tcp
} // namespace inet

#endif
