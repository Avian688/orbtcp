#!/usr/bin/env python3
"""Constant-evaluate PintQueue's actual EWMA update; no simulator build/run.

Only time, queue state and the flow-state map are stubbed. The map uses a
constexpr vector because std::unordered_map is not constexpr in C++23.
"""
from pathlib import Path
import subprocess
import tempfile

base = Path(__file__).resolve().parents[1] / 'src/queueing/queue'
source = (base / 'PintQueue.cc').read_text()
start = source.index('double PintQueue::updatePintUtilization(')
end = source.index('\nuint16_t PintQueue::encodePintUtilization(', start)
method = source[start:end].replace('double PintQueue::updatePintUtilization(',
                                   'constexpr double updatePintUtilization(', 1)
header = (base / 'PintQueue.h').read_text()
start = header.index('    struct FlowUtilization {')
flow_state = header[start:header.index('\n    };', start) + len('\n    };')]
preamble = r'''
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>
struct Time {
    double value=0;
    constexpr double dbl() const { return value; }
    constexpr operator double() const { return value; }
    friend constexpr Time operator-(Time a, Time b) { return {a.value-b.value}; }
};
using simtime_t=Time;
constexpr Time SIMTIME_ZERO{};
template<class T> struct FlowMap {
    std::vector<std::pair<uint64_t,T>> entries;
    constexpr auto emplace(uint64_t key, T value) {
        for (auto it=entries.begin(); it!=entries.end(); ++it)
            if (it->first==key) return std::make_pair(it,false);
        entries.emplace_back(key,value);
        return std::make_pair(entries.end()-1,true);
    }
    constexpr T& operator[](uint64_t key) { return emplace(key,T{}).first->second; }
};
struct PintQueue {
    Time avgRtt{.04}, pintInitialRtt{.01}, pintNoAverageRttInterval{.02};
    Time lastPintUpdate{}, now{};
    bool pintUseAverageRtt=true, hasPintSample=false;
    double pintUtilization=0;
    constexpr Time simTime() const { return now; }
'''
checks = r'''
constexpr bool near(double a, double b) { return a-b<1e-10 && b-a<1e-10; }
constexpr double update(PintQueue& q, double time, uint64_t bytes=1500,
                       uint64_t queued=0, double capacity=12500000,
                       double flowRtt=.04, uint64_t flow=1) {
    q.now={time};
    return q.updatePintUtilization(bytes,queued,capacity,flowRtt,flow);
}
constexpr bool averageRttGain() {
    PintQueue q;
    if (update(q,0)!=0) return false; // First sample retains queue-only seeding.
    if (!near(update(q,.00012),.003)) return false; // .12ms / 40ms.
    q.avgRtt={.02}; // The most recently measured queue RTT takes effect immediately.
    if (!near(update(q,.00024),.994*.003+.006)) return false;
    PintQueue fallback;
    fallback.avgRtt={0};
    update(fallback,0);
    if (!near(update(fallback,.00012),.012)) return false; // Initial RTT = 10ms.
    // RTT is shared across packet IDs in average-RTT mode.
    PintQueue shared;
    update(shared,0,1500,0,12500000,.2,1);
    return near(update(shared,.00012,1500,0,12500000,.002,2),.003);
}
static_assert(averageRttGain(), "Current avgRTT, initial fallback and queue-wide packet clock");

constexpr bool idleAndServiceFloor() {
    PintQueue q;
    if (!near(update(q,0,10,20,1000),.5)) return false;
    // Repeated timestamp: retain the existing payload service-time lower bound.
    if (!near(update(q,0,10,20,1000),.75)) return false;
    // A 1s idle: gain caps at one, but rate denominator stays 1s (not RTT=40ms).
    if (!near(update(q,1,10,8,1000),.21)) return false;
    PintQueue boundary;
    update(boundary,0,10,0,1000);
    return near(update(boundary,.04,10,0,1000),.25);
}
static_assert(idleAndServiceFloor(), "Safe zero intervals and full idle interval in the rate sample");

constexpr bool smoothingTimescale() {
    PintQueue fast, slow;
    update(fast,0,500,0,2500000);
    update(slow,0,1000,0,2500000);
    for (int i=1;i<=200;++i) update(fast,i*.0002,500,0,2500000);
    for (int i=1;i<=100;++i) update(slow,i*.0004,1000,0,2500000);
    // After 40ms, a unit step reaches about 1-exp(-1), despite different event rates.
    return fast.pintUtilization>.63 && fast.pintUtilization<.635 &&
           slow.pintUtilization>.63 && slow.pintUtilization<.635 &&
           slow.pintUtilization-fast.pintUtilization<.001;
}
static_assert(smoothingTimescale(), "Smoothing time follows RTT rather than a fixed packet count");

constexpr bool perFlowClock() {
    PintQueue q;
    q.pintUseAverageRtt=false;
    update(q,0,1000,0,1000000,.04,1);
    // Other flows keep the link busy while flow 1 is quiet.
    for (int i=1;i<10;++i) update(q,i*.001,1000,0,1000000,.02,2);
    // Sample uses the queue's 1ms interval; EWMA uses this flow's 10ms interval.
    if (!near(update(q,.010,1000,0,1000000,.04,1),.25)) return false;
    if (!near(q.perFlowPintUtilization[2].value,1)) return false;
    // Following a quiet link interval, the sample still measures the link rate.
    update(q,.011,1000,0,1000000,.02,2);
    if (!near(update(q,1.011,1000,0,1000000,.04,1),.001)) return false;
    PintQueue fallback;
    fallback.pintUseAverageRtt=false;
    update(fallback,0,1000,0,1000000,0,1);
    // Missing flow RTT uses pintNoAverageRttInterval=20ms.
    return near(update(fallback,.001,1000,0,1000000,0,1),.05);
}
static_assert(perFlowClock(), "Per-flow RTT ablation has its own EWMA clock and a queue-wide rate sample");
'''
with tempfile.TemporaryDirectory(prefix='pint-smoothing-') as temp:
    path = Path(temp) / 'smoothing.cc'
    path.write_text(preamble + flow_state + '\n    FlowMap<FlowUtilization> perFlowPintUtilization;\n'
                    + method + '\n};\n' + checks)
    subprocess.run(['clang++', '-std=c++23', '-fsyntax-only', str(path)], check=True)
print('PINT time-weighted smoothing compile-time checks passed (no simulator build or run).')
