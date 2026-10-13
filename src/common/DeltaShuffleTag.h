// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DELTA_SHUFFLE_TAG_H
#define INET_DELTA_SHUFFLE_TAG_H
#include "inet/common/TagBase.h"
namespace inet {
// TCP-header region telemetry; header size is abstracted as for Omega.
class DeltaShuffleTag : public TagBase {
  public:
    double subflowRate = 0, connectionRate = 0; // payload bytes/s
    double need = 0; // 1/X, seconds/byte; last participating link's rate floor
    double rawShuffle = 0, shuffle = 0; // SUM of forward link rate increments, bytes/s
    unsigned long long serial = 0; // new for each data transmission
    unsigned int hops = 0;
    bool echoed = false;
    virtual DeltaShuffleTag *dup() const override { return new DeltaShuffleTag(*this); }
    virtual void parsimPack(cCommBuffer *b) const override {
        TagBase::parsimPack(b);
        b->pack(subflowRate); b->pack(connectionRate); b->pack(need);
        b->pack(rawShuffle); b->pack(shuffle); b->pack(serial); b->pack(hops); b->pack(echoed);
    }
    virtual void parsimUnpack(cCommBuffer *b) override {
        TagBase::parsimUnpack(b);
        b->unpack(subflowRate); b->unpack(connectionRate); b->unpack(need);
        b->unpack(rawShuffle); b->unpack(shuffle); b->unpack(serial); b->unpack(hops); b->unpack(echoed);
    }
};
}
#endif
