// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DELTA_SHUFFLE_TAG_H
#define INET_DELTA_SHUFFLE_TAG_H
#include "inet/common/TagBase.h"
#include <vector>
namespace inet {
// TCP-header region telemetry; header size is abstracted in the simulation.
class DeltaShuffleTag : public TagBase {
  public:
    double subflowRate = 0, connectionRate = 0; // payload bytes/s
    double need = 0; // 1/X, seconds/byte; last participating link's rate floor
    double rawShuffle = 0, shuffle = 0; // selected bottleneck rate increments, bytes/s
    unsigned long long serial = 0; // new for each data transmission
    unsigned int hops = 0;
    bool echoed = false;
    bool senderControlled = false;
    // Selected bottleneck measurement (zero or one record). mass is dimensionless, need/meanNeed s/B,
    // coefficient (B/s)^2. No signed grant is calculated in the switch.
    struct Measurement { double mass, need, meanNeed, coefficient; };
    std::vector<Measurement> measurements;
    virtual DeltaShuffleTag *dup() const override { return new DeltaShuffleTag(*this); }
    virtual void parsimPack(cCommBuffer *b) const override {
        TagBase::parsimPack(b);
        b->pack(subflowRate); b->pack(connectionRate); b->pack(need);
        b->pack(rawShuffle); b->pack(shuffle); b->pack(serial); b->pack(hops); b->pack(echoed);
        b->pack(senderControlled);
        b->pack(static_cast<unsigned int>(measurements.size()));
        for (const auto& m : measurements) {
            b->pack(m.mass); b->pack(m.need); b->pack(m.meanNeed); b->pack(m.coefficient);
        }
    }
    virtual void parsimUnpack(cCommBuffer *b) override {
        TagBase::parsimUnpack(b);
        b->unpack(subflowRate); b->unpack(connectionRate); b->unpack(need);
        b->unpack(rawShuffle); b->unpack(shuffle); b->unpack(serial); b->unpack(hops); b->unpack(echoed);
        b->unpack(senderControlled);
        unsigned int count;
        b->unpack(count);
        measurements.resize(count);
        for (auto& m : measurements) {
            b->unpack(m.mass); b->unpack(m.need); b->unpack(m.meanNeed); b->unpack(m.coefficient);
        }
    }
};
}
#endif
