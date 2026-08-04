//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//

#ifndef COMMON_PINTFLOWCOUNT_H_
#define COMMON_PINTFLOWCOUNT_H_

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace inet {
namespace pint {

constexpr uint32_t FLOW_COUNT_MAX = 65535;

inline bool isValidFlowCountBits(int bits)
{
    return bits == 0 || (bits >= 2 && bits <= 16);
}

inline uint32_t flowCountMaxCode(int bits)
{
    return bits == 0 ? FLOW_COUNT_MAX : (1U << bits) - 1;
}

inline uint32_t flowCountExactMax(int bits, uint32_t maxCount)
{
    const uint32_t maxCode = flowCountMaxCode(bits);
    if (maxCount <= maxCode)
        return maxCount;

    // Keep the lowest one eighth of the code space exact. This preserves the
    // previous 8-bit mapping, where counts 0 through 31 were exact.
    return std::max(1U, maxCode / 8);
}

inline double flowCountLogBase(int bits, uint32_t maxCount)
{
    const uint32_t maxCode = flowCountMaxCode(bits);
    const uint32_t firstLogCode = flowCountExactMax(bits, maxCount) + 1;
    const uint32_t intervals = maxCode - firstLogCode;
    return std::pow(static_cast<double>(maxCount) / firstLogCode,
            1.0 / intervals);
}

inline uint32_t decodeFlowCount(uint32_t code, int bits = 8,
        uint32_t maxCount = FLOW_COUNT_MAX)
{
    if (bits == 0)
        return std::min(code, FLOW_COUNT_MAX);

    const uint32_t maxCode = flowCountMaxCode(bits);
    maxCount = std::clamp(maxCount, 1U, FLOW_COUNT_MAX);
    code = std::min(code, maxCode);

    if (maxCount <= maxCode)
        return std::min(code, maxCount);

    const uint32_t exactMax = flowCountExactMax(bits, maxCount);
    if (code <= exactMax)
        return code;
    if (code == maxCode)
        return maxCount;

    const uint32_t firstLogCode = exactMax + 1;
    const uint32_t exponent = code - firstLogCode;
    const double value = firstLogCode *
            std::pow(flowCountLogBase(bits, maxCount), exponent);
    return std::min(maxCount,
            static_cast<uint32_t>(std::ceil(value)));
}

inline uint32_t encodeFlowCount(uint32_t count, int bits = 8,
        uint32_t maxCount = FLOW_COUNT_MAX)
{
    count = std::min(count, FLOW_COUNT_MAX);
    if (bits == 0)
        return count;

    const uint32_t maxCode = flowCountMaxCode(bits);
    maxCount = std::clamp(maxCount, 1U, FLOW_COUNT_MAX);
    if (maxCount <= maxCode)
        return std::min(count, maxCount);

    const uint32_t exactMax = flowCountExactMax(bits, maxCount);
    if (count <= exactMax)
        return count;
    if (count >= maxCount)
        return maxCode;

    const uint32_t firstLogCode = exactMax + 1;
    const double exponent = std::log(
            static_cast<double>(count) / firstLogCode) /
            std::log(flowCountLogBase(bits, maxCount));
    uint32_t code = firstLogCode +
            static_cast<uint32_t>(std::ceil(exponent));
    code = std::min(code, maxCode);

    // Floating-point rounding must never turn conservative encoding into an undercount.
    while (code < maxCode && decodeFlowCount(code, bits, maxCount) < count)
        code++;
    while (code > firstLogCode &&
            decodeFlowCount(code - 1, bits, maxCount) >= count)
        code--;
    return code;
}

} // namespace pint
} // namespace inet

#endif
