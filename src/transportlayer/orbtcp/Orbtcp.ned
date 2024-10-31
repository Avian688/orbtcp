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

package orbtcp.transportlayer.orbtcp;

import tcppaced.transportlayer.tcp.TcpPaced;

simple Orbtcp extends TcpPaced
{
    parameters:
        @class("inet::tcp::Orbtcp");
        int bandwidth = default(18750000);
        double basePropagationRTT @unit(s) = default(0.01s);
        int subFlows = default(1);
        int sharingFlows = default(2);
        double additiveIncreasePercent = default(0.05);
        double eta = default(0.95);
        double alpha = default (0.03);
}