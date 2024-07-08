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

#ifndef HPCC_IPV4NETWORKCONFIGURATORUPDATE_H_
#define HPCC_IPV4NETWORKCONFIGURATORUPDATE_H_

#include "inet/networklayer/configurator/ipv4/Ipv4NetworkConfigurator.h"
#include "inet/networklayer/configurator/base/NetworkConfiguratorBase.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace inet {

class Ipv4NetworkConfiguratorUpdate : public Ipv4NetworkConfigurator{
    public:
        virtual void reinvokeConfigurator(Topology& topology, cXMLElement *autorouteElement);
        void configureRoutingTable(Node *node);
        virtual void configureAllRoutingTables() override;
        virtual void configureRoutingTable(IIpv4RoutingTable *routingTable) override;
        virtual ~Ipv4NetworkConfiguratorUpdate(){};
    protected:
        simtime_t timerInterval;
        cMessage * timer = nullptr;
        virtual void handleMessage(cMessage *msg) override;
        virtual void initialize(int stage) override;

        virtual double computeWiredLinkWeight(Link *link, const char *metric, cXMLElement *parameters) override;


        //virtual void extractTopology(Topology& topology);



};
}
#endif /* OS3_NETWORKLAYER_CONFIGURATOR_IPV4_SATELLITENETWORKCONFIGURATOR_H_ */
