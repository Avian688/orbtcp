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

#include "OrbtcpSendQueue.h"

namespace inet {
namespace tcp {

Register_Class(OrbtcpSendQueue);

Packet *OrbtcpSendQueue::createSegmentWithBytes(uint32_t fromSeq, uint32_t numBytes)
{
    ASSERT(seqLE(begin, fromSeq) && seqLE(fromSeq + numBytes, end));

    char msgname[32];
    sprintf(msgname, "tcpseg(l=%u)", (unsigned int)numBytes);

    Packet *tcpSegment = new Packet(msgname);
    const auto& payload = dataBuffer.peekAt(B(fromSeq - begin), B(numBytes)); // get data from buffer
    tcpSegment->addTagIfAbsent<CreationTimeTag>()->setCreationTime(simTime());
    tcpSegment->insertAtBack(payload);
    return tcpSegment;
}

void OrbtcpSendQueue::discardUpTo(uint32_t seqNum)
{
    ASSERT(seqLE(begin, seqNum) && seqLE(seqNum, end));

    uint32_t dataPopped = seqNum - begin;
    if (seqNum != begin) {
        dataBuffer.pop(B(seqNum - begin));
        begin = seqNum;
    }

    if(dataPopped > 0){
        Ptr<Chunk> payload = makeShared<ByteCountChunk>(B(dataPopped));
        //payload->addTag<IntTag>();
        Packet *packet = new Packet("data");
        packet->insertAtBack(payload);

        dataBuffer.push(packet->peekDataAt(B(0), packet->getDataLength()));
        end += packet->getByteLength();
        delete packet;
    }
}

}
}

