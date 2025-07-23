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

#ifndef TRANSPORTLAYER_ORBTCP_FLAVOURS_ORBTCPNOINITFLOWSFLAVOUR_H_
#define TRANSPORTLAYER_ORBTCP_FLAVOURS_ORBTCPNOINITFLOWSFLAVOUR_H_

#include "../../../common/IntTag_m.h"
#include "../OrbtcpConnection.h"
#include "OrbtcpFamily.h"

namespace inet {
namespace tcp {

/**
 * State variables for Orbtcp.
 */
typedef OrbtcpFamilyStateVariables OrbtcpStateVariables;

/**
 * Implements OrbTCP.
 */
class OrbtcpNoInitFlowsFlavour : public OrbtcpFamily
{
  protected:
    OrbtcpStateVariables *& state;

    static simsignal_t txRateSignal; // will record load
    static simsignal_t tauSignal; // will record total number of RTOs
    static simsignal_t uSignal;
    static simsignal_t USignal;
    static simsignal_t additiveIncreaseSignal;
    static simsignal_t sharingFlowsSignal;
    static simsignal_t bottleneckBandwidthSignal;
    static simsignal_t avgRttSignal;
    static simsignal_t queueingDelaySignal;
    static simsignal_t estimatedRttSignal;
    static simsignal_t smoothedEstimatedRttSignal;
    static simsignal_t avgEstimatedRttSignal;
    static simsignal_t alphaSignal;
    static simsignal_t measuringInflightSignal;
    static simsignal_t testRttSignal;
    static simsignal_t recoveryPointSignal;
    static simsignal_t sndUnaSignal;
    static simsignal_t sndMaxSignal;
    static simsignal_t txBytesSignal;

    size_t connId;
    simtime_t rtt;
    simtime_t estimatedRtt;
    simtime_t smoothedEstimatedRtt;

    cMessage *reactTimer;
    bool updateWindow;

    bool pathChanged;

    std::vector<bool> pathId;

    bool initPackets;
    /** Create and return a OrbtcpStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new OrbtcpStateVariables();
    }

    virtual void initialize() override;

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TcpEventCode& event) override;

  public:
    /** Constructor */
    OrbtcpNoInitFlowsFlavour();

    ~OrbtcpNoInitFlowsFlavour();

    virtual void established(bool active) override;

    virtual void rttMeasurementComplete(simtime_t tSent, simtime_t tAcked) override;

    virtual void receivedDataAck(uint32_t firstSeqAcked, IntDataVec intData) override;

    virtual uint32_t computeWnd(double u, bool updateWc);

    virtual double measureInflight(IntDataVec intData);

    virtual size_t getConnId() override;

    virtual simtime_t getRtt() override;

    virtual simtime_t getEstimatedRtt() override;

    /** Redefine what should happen when dupAck was received, to add congestion window management */

    virtual void receivedDuplicateAck(uint32_t firstSeqAcked, IntDataVec intData) override;

    virtual void processTimer(cMessage *timer, TcpEventCode& event) override;

    };

} // namespace tcp
} // namespace inet

#endif

