//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//

#ifndef COMMON_PINTSENDERTELEMETRY_H_
#define COMMON_PINTSENDERTELEMETRY_H_

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace inet {
namespace pint {

constexpr int BASE_RTT_BITS = 24;
constexpr double BASE_RTT_UNIT_SECONDS = 1e-6;
constexpr uint32_t BASE_RTT_MAX_CODE = (1U << BASE_RTT_BITS) - 1;

constexpr int CWND_BITS = 16;
constexpr int CWND_EXPONENT_BITS = 5;
constexpr int CWND_MANTISSA_BITS = 11;
constexpr uint32_t CWND_MAX_CODE = (1U << CWND_BITS) - 1;
constexpr uint32_t CWND_MANTISSA_MASK =
        (1U << CWND_MANTISSA_BITS) - 1;
constexpr uint64_t CWND_IMPLICIT_BIT = 1ULL << CWND_MANTISSA_BITS;
constexpr uint32_t CWND_MAX_EXPONENT =
        (1U << CWND_EXPONENT_BITS) - 1;
constexpr uint64_t CWND_MAX_BYTES =
        (CWND_IMPLICIT_BIT + CWND_MANTISSA_MASK) <<
        (CWND_MAX_EXPONENT - 1);

static_assert(CWND_EXPONENT_BITS + CWND_MANTISSA_BITS == CWND_BITS,
        "PINT cwnd encoding must occupy 16 bits");

inline uint32_t encodeBaseRtt(double rttSeconds)
{
    if (!(rttSeconds > 0))
        return 0;

    const double microseconds = std::round(rttSeconds / BASE_RTT_UNIT_SECONDS);
    if (!std::isfinite(microseconds) || microseconds >= BASE_RTT_MAX_CODE)
        return BASE_RTT_MAX_CODE;
    return static_cast<uint32_t>(microseconds);
}

inline double decodeBaseRtt(uint32_t code)
{
    return std::min(code, BASE_RTT_MAX_CODE) * BASE_RTT_UNIT_SECONDS;
}

inline uint32_t encodeCwnd(uint64_t cwndBytes)
{
    if (cwndBytes == 0)
        return 0;
    if (cwndBytes <= CWND_MANTISSA_MASK)
        return static_cast<uint32_t>(cwndBytes);
    if (cwndBytes >= CWND_MAX_BYTES)
        return CWND_MAX_CODE;

    uint32_t shift = 0;
    while ((cwndBytes >> shift) >
            CWND_IMPLICIT_BIT + CWND_MANTISSA_MASK)
        shift++;

    const uint64_t quantum = 1ULL << shift;
    uint64_t significand = (cwndBytes + quantum / 2) / quantum;
    if (significand > CWND_IMPLICIT_BIT + CWND_MANTISSA_MASK) {
        significand >>= 1;
        shift++;
    }

    const uint32_t exponent = shift + 1;
    const uint32_t mantissa =
            static_cast<uint32_t>(significand - CWND_IMPLICIT_BIT);
    return (exponent << CWND_MANTISSA_BITS) | mantissa;
}

inline uint64_t decodeCwnd(uint32_t code)
{
    code = std::min(code, CWND_MAX_CODE);
    const uint32_t exponent = code >> CWND_MANTISSA_BITS;
    const uint32_t mantissa = code & CWND_MANTISSA_MASK;
    if (exponent == 0)
        return mantissa;

    return (CWND_IMPLICIT_BIT + mantissa) << (exponent - 1);
}

} // namespace pint
} // namespace inet

#endif
