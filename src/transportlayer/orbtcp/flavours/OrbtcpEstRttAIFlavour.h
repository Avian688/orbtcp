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

#ifndef TRANSPORTLAYER_ORBTCP_FLAVOURS_ORBTCPESTRTTAIFLAVOUR_H_
#define TRANSPORTLAYER_ORBTCP_FLAVOURS_ORBTCPESTRTTAIFLAVOUR_H_

#include "../../../common/IntTag_m.h"
#include "../OrbtcpConnection.h"
#include "OrbtcpFamily.h"

namespace inet {
namespace tcp {

/**
 * State variables for OrbtcpEstRttAIFlavour.
 */
typedef OrbtcpFamilyStateVariables OrbtcpEstRttAIStateVariables;

/**
 * Implements OrbTCP where the AI factor utilises an estimated RTT rather than a standard RTT. Using estRtt vs rtt leads to unfairness in different rtt flows.
 */
class OrbtcpEstRttAIFlavour : public OrbtcpFamily
{
  protected:
    OrbtcpEstRttAIStateVariables *& state;

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
    static simsignal_t avgEstimatedRttSignal;
    static simsignal_t alphaSignal;

    size_t connId;
    simtime_t rtt;
    simtime_t estimatedRtt;
    bool initPackets;
    /** Create and return a OrbtcpEstRttAIStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new OrbtcpEstRttAIStateVariables();
    }

    virtual void initialize() override;

  public:
    /** Constructor */
    OrbtcpEstRttAIFlavour();

    virtual void established(bool active) override;

    virtual void rttMeasurementComplete(simtime_t tSent, simtime_t tAcked) override;

    virtual void receivedDataAck(uint32_t firstSeqAcked, IntDataVec intData) override;

    virtual uint32_t computeWnd(double u, bool updateWc);

    virtual double measureInflight(IntDataVec intData);

    virtual size_t getConnId() override;

    virtual simtime_t getRtt() override;
    virtual unsigned int getCwnd() override;


    };

} // namespace tcp
} // namespace inet

#endif

