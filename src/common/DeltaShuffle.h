// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DELTA_SHUFFLE_H
#define INET_DELTA_SHUFFLE_H
#include <algorithm>
#include <cstdint>
namespace inet { namespace delta {
struct Shuffle { double meanNeed = 0, raw = 0, returned = 0; };
// Only aggregate link counters. Rates/budget/shuffle: payload bytes/s.
// Packet mass L/(x * roundDuration): dimensionless. Need 1/X: seconds/byte.
struct LinkShuffle {
    int64_t epoch = -1;
    double mass = 0, needMass = 0;
    double previousMass = 0, previousMean = 0;
    double minNeed = 0, maxNeed = 0, previousRange = 0;
    double magnitude = 0, net = 0;

    constexpr void beginRound(int64_t nextEpoch) {
        if (nextEpoch != epoch) {
            const bool adjacent = nextEpoch == epoch + 1 && mass > 0;
            previousMass = adjacent ? mass : 0;
            previousMean = adjacent ? needMass / mass : 0;
            previousRange = adjacent ? maxNeed - minNeed : 0;
            mass = needMass = magnitude = net = 0;
            epoch = nextEpoch;
        }
    }

    constexpr Shuffle issue(int64_t nextEpoch, double budget, double packetMass, double need, double gain = 8) {
        beginRound(nextEpoch);
        minNeed = mass > 0 ? std::min(minNeed, need) : need;
        maxNeed = mass > 0 ? std::max(maxNeed, need) : need;
        mass += packetMass;
        needMass += packetMass * need;
        // Increase sensitivity to modest connection-rate gaps, without raising
        // the link budget. A common range-based scale preserves both signs:
        // for a stable population, sum(mass * abs(need-mean)) <= M * range.
        const double scale = gain > 0 ? std::max(2 * previousMean / gain, previousRange) : 0;
        const double raw = previousMass > 0 && scale > 0 ?
                budget * (packetMass / previousMass) * ((need - previousMean) / scale) : 0;
        const double remaining = std::max(0.0, budget - magnitude);
        const double returned = std::clamp(raw, -remaining, remaining);
        magnitude += returned < 0 ? -returned : returned;
        net += returned;
        return {previousMean, raw, returned};
    }
};
} }
#endif
