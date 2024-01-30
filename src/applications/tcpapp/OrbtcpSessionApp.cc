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

#include "OrbtcpSessionApp.h"

#include <inet/applications/base/ApplicationPacket_m.h>
#include <inet/common/ModuleAccess.h>
#include <inet/common/TagBase_m.h>
#include <inet/common/TimeTag_m.h>
#include <inet/common/lifecycle/ModuleOperations.h>
#include <inet/common/packet/Packet.h>
#include <inet/common/packet/chunk/ByteCountChunk.h>
#include <inet/common/packet/chunk/BytesChunk.h>
#include <inet/networklayer/common/L3AddressResolver.h>


namespace inet {
Define_Module(OrbtcpSessionApp);

Packet *OrbtcpSessionApp::createDataPacket(long sendBytes)
{
    const char *dataTransferMode = par("dataTransferMode");
    Ptr<Chunk> payload;
    if (!strcmp(dataTransferMode, "bytecount")) {
        payload = makeShared<ByteCountChunk>(B(sendBytes));
    }
    else if (!strcmp(dataTransferMode, "object")) {
        const auto& applicationPacket = makeShared<ApplicationPacket>();
        applicationPacket->setChunkLength(B(sendBytes));
        payload = applicationPacket;
    }
    else if (!strcmp(dataTransferMode, "bytestream")) {
        const auto& bytesChunk = makeShared<BytesChunk>();
        std::vector<uint8_t> vec;
        vec.resize(sendBytes);
        for (int i = 0; i < sendBytes; i++)
            vec[i] = (bytesSent + i) & 0xFF;
        bytesChunk->setBytes(vec);
        payload = bytesChunk;
    }
    else
        throw cRuntimeError("Invalid data transfer mode: %s", dataTransferMode);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    Packet *packet = new Packet("data1");
    packet->insertAtBack(payload);
    //packet->addTag<IntTag>();
    return packet;
}

}
