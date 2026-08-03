//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//

#ifndef COMMON_PINTQUEUEINGDELAY_H_
#define COMMON_PINTQUEUEINGDELAY_H_

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace inet {
namespace pint {

// Conceptual wire format: 12 bits in 64 us units, saturating at 262.08 ms.
constexpr int QUEUEING_DELAY_BITS = 12;
constexpr double QUEUEING_DELAY_UNIT_SECONDS = 64e-6;
constexpr uint32_t QUEUEING_DELAY_MAX_CODE =
        (1U << QUEUEING_DELAY_BITS) - 1;

inline uint32_t encodeQueueingDelay(double delaySeconds)
{
    if (!(delaySeconds > 0))
        return 0;

    const double units = std::round(delaySeconds / QUEUEING_DELAY_UNIT_SECONDS);
    if (!std::isfinite(units) || units >= QUEUEING_DELAY_MAX_CODE)
        return QUEUEING_DELAY_MAX_CODE;
    return static_cast<uint32_t>(units);
}

inline uint32_t accumulateQueueingDelay(uint32_t currentCode, double localDelaySeconds)
{
    currentCode = std::min(currentCode, QUEUEING_DELAY_MAX_CODE);
    const uint32_t localCode = encodeQueueingDelay(localDelaySeconds);
    const uint32_t availableUnits = QUEUEING_DELAY_MAX_CODE - currentCode;
    if (localCode >= availableUnits)
        return QUEUEING_DELAY_MAX_CODE;

    return currentCode + localCode;
}

inline double decodeQueueingDelay(uint32_t code)
{
    return std::min(code, QUEUEING_DELAY_MAX_CODE) *
            QUEUEING_DELAY_UNIT_SECONDS;
}

} // namespace pint
} // namespace inet

#endif
