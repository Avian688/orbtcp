// No simulator or executable is built.
// clang++ -std=c++17 -fsyntax-only samples/orbtcp/tests/PintSplitDelayTest.cc
#include "../src/common/PintQueueingDelay.h"

using namespace inet::pint;

struct Feedback {
    uint32_t forward = 0;
    uint32_t reverse = 0;
    bool separate = false;
    constexpr uint32_t getQueueingDelayCode() const { return forward; }
    constexpr uint32_t getReverseQueueingDelayCode() const { return reverse; }
    constexpr bool getSeparateQueueingDelay() const { return separate; }
    constexpr void setQueueingDelayCode(uint32_t value) { forward = value; }
    constexpr void setReverseQueueingDelayCode(uint32_t value) { reverse = value; }
};

constexpr bool splitDelayChecks()
{
    Feedback data;
    accumulatePacketQueueingDelay(data, true, 64e-6 * 10);
    accumulatePacketQueueingDelay(data, true, 64e-6 * 20);
    if (data.forward != 30 || data.reverse != 0)
        return false;
    Feedback ack = data;
    ack.separate = true;
    accumulatePacketQueueingDelay(ack, false, 64e-6 * 40);
    accumulatePacketQueueingDelay(ack, false, 64e-6 * 50);
    if (ack.forward != 30 || ack.reverse != 90)
        return false;
    const double expected = decodeQueueingDelay(30) + decodeQueueingDelay(90);
    if (decodeTotalQueueingDelay(ack) != expected)
        return false;

    Feedback legacy = data;
    accumulatePacketQueueingDelay(legacy, false, 64e-6 * 90);
    if (legacy.forward != 120 || legacy.reverse != 0)
        return false;
    legacy.reverse = 999; // ignored unless the split format is present
    if (decodeTotalQueueingDelay(legacy) != decodeQueueingDelay(120))
        return false;

    accumulatePacketQueueingDelay(ack, false, 10);
    if (ack.forward != 30 || ack.reverse != QUEUEING_DELAY_MAX_CODE)
        return false;
    accumulatePacketQueueingDelay(ack, true, 10);
    return ack.forward == QUEUEING_DELAY_MAX_CODE &&
            decodeTotalQueueingDelay(ack) == 2 * decodeQueueingDelay(QUEUEING_DELAY_MAX_CODE);
}

static_assert(splitDelayChecks(), "Separate queue residence while preserving the CCA's round-trip sum");
static_assert(encodeQueueingDelay(-1) == 0, "Reject negative residence time");
static_assert(encodeQueueingDelay(31e-6) == 0 && encodeQueueingDelay(32e-6) == 1,
        "Retain rounding at a half-quantum");
