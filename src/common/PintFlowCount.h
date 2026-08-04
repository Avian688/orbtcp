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

constexpr int FLOW_COUNT_BITS = 8;
constexpr uint32_t FLOW_COUNT_EXACT_MAX = 31;
constexpr uint32_t FLOW_COUNT_FIRST_LOG_CODE = FLOW_COUNT_EXACT_MAX + 1;
constexpr uint32_t FLOW_COUNT_MAX_CODE = (1U << FLOW_COUNT_BITS) - 1;
constexpr uint32_t FLOW_COUNT_MAX = 65535;
constexpr uint32_t FLOW_COUNT_LOG_INTERVALS =
        FLOW_COUNT_MAX_CODE - FLOW_COUNT_FIRST_LOG_CODE;

static_assert(FLOW_COUNT_LOG_INTERVALS > 0,
        "PINT flow-count encoding needs at least one logarithmic interval");

inline double flowCountLogBase()
{
    static const double base = std::pow(
            static_cast<double>(FLOW_COUNT_MAX) /
                    FLOW_COUNT_FIRST_LOG_CODE,
            1.0 / FLOW_COUNT_LOG_INTERVALS);
    return base;
}

inline uint32_t decodeFlowCount(uint32_t code)
{
    code = std::min(code, FLOW_COUNT_MAX_CODE);
    if (code <= FLOW_COUNT_EXACT_MAX)
        return code;
    if (code == FLOW_COUNT_MAX_CODE)
        return FLOW_COUNT_MAX;

    const uint32_t exponent = code - FLOW_COUNT_FIRST_LOG_CODE;
    const double value = FLOW_COUNT_FIRST_LOG_CODE *
            std::pow(flowCountLogBase(), exponent);
    return std::min(FLOW_COUNT_MAX,
            static_cast<uint32_t>(std::ceil(value)));
}

inline uint32_t encodeFlowCount(uint32_t count)
{
    if (count <= FLOW_COUNT_EXACT_MAX)
        return count;
    if (count >= FLOW_COUNT_MAX)
        return FLOW_COUNT_MAX_CODE;

    const double exponent = std::log(
            static_cast<double>(count) / FLOW_COUNT_FIRST_LOG_CODE) /
            std::log(flowCountLogBase());
    uint32_t code = FLOW_COUNT_FIRST_LOG_CODE +
            static_cast<uint32_t>(std::ceil(exponent));
    code = std::min(code, FLOW_COUNT_MAX_CODE);

    // Floating-point rounding must never turn conservative encoding into an undercount.
    while (code < FLOW_COUNT_MAX_CODE && decodeFlowCount(code) < count)
        code++;
    while (code > FLOW_COUNT_FIRST_LOG_CODE &&
            decodeFlowCount(code - 1) >= count)
        code--;
    return code;
}

} // namespace pint
} // namespace inet

#endif
