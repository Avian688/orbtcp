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

import tcppaced.transportlayer.tcp.TcpPacedConnection;

simple OrbtcpConnection extends TcpPacedConnection
{
    parameters:
        @class("inet::tcp::OrbtcpConnection"); 
        
        @signal[txRate];
        @signal[tau];
        @signal[u];
        @signal[U];
        @signal[sharingFlows];
        @signal[initialPhaseSharingFlows];
        @signal[bottleneckBandwidth];
        @signal[avgRtt];
        @signal[queueingDelay];
        @signal[estimatedRtt];
        @signal[smoothedEstimatedRtt];
        @signal[avgEstimatedRtt];
        @signal[alpha];
        @signal[measuringInflight];
        @signal[testRtt];
        @signal[recoveryPoint];	
        @signal[sndUna];
        @signal[sndMax];
        @signal[txBytes];
        
        @statistic[sndMax](record=vector; interpolationmode=sample-hold);  
		@statistic[sndUna](record=vector; interpolationmode=sample-hold);  
		@statistic[recoveryPoint](record=vector; interpolationmode=sample-hold); 
        @statistic[txRate](record=vector; interpolationmode=sample-hold);
        @statistic[txBytes](record=vector; interpolationmode=sample-hold);
        @statistic[tau](record=vector; interpolationmode=sample-hold);
        @statistic[u](record=vector; interpolationmode=sample-hold);
        @statistic[U](record=vector; interpolationmode=sample-hold);
		@statistic[sharingFlows](record=vector; interpolationmode=sample-hold);
		@statistic[initialPhaseSharingFlows](record=vector; interpolationmode=sample-hold);
		@statistic[bottleneckBandwidth](record=vector; interpolationmode=sample-hold);
		@statistic[avgRtt](record=vector; interpolationmode=sample-hold);
		@statistic[queueingDelay](record=vector; interpolationmode=sample-hold);
		@statistic[estimatedRtt](record=vector; interpolationmode=sample-hold);
		@statistic[smoothedEstimatedRtt](record=vector; interpolationmode=sample-hold);
		@statistic[avgEstimatedRtt](record=vector; interpolationmode=sample-hold);
		@statistic[alpha](record=vector; interpolationmode=sample-hold);
		@statistic[measuringInflight](record=vector; interpolationmode=sample-hold);
		@statistic[testRtt](record=vector; interpolationmode=sample-hold);
}
