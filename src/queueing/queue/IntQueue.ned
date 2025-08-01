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

package orbtcp.queueing.queue;

import inet.queueing.queue.PacketQueue;

simple IntQueue extends PacketQueue
{
    parameters:
        @class("inet::queueing::IntQueue");
        
        @signal[avgRttQueue];
        @statistic[avgRttQueue](record=vector; interpolationmode=sample-hold);
        
        @signal[numberOfFlows];
        @statistic[numberOfFlows](record=vector; interpolationmode=sample-hold);
        
        @signal[numOfFlowsInInitialPhase];
        @statistic[numOfFlowsInInitialPhase](record=vector; interpolationmode=sample-hold);
        
        @signal[persistentQueueingDelay];
        @statistic[persistentQueueingDelay](record=vector; interpolationmode=sample-hold);
    	
    	@signal[bandwidth];
   		@statistic[bandwidth](record=vector; interpolationmode=sample-hold);
    	
    	@signal[txBytes];
   		@statistic[txBytes](record=vector; interpolationmode=sample-hold);
   		
        packetCapacity @mutable = default(100);
        double fixedAvgRTTVal = default(0);
        dropperClass = default("inet::queueing::PacketAtCollectionEndDropper");
}
