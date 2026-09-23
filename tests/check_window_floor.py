#!/usr/bin/env python3
"""Constant-evaluate actual OrbCC window updates; no simulator build or run."""
from pathlib import Path
import subprocess
import tempfile

base = Path(__file__).resolve().parents[1] / 'src/transportlayer/orbtcp/flavours'

def body(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]

pint = (base / 'OrbtcpPintFlavour.cc').read_text()
full = (base / 'OrbtcpFlavour.cc').read_text()
clamp = body(pint, 'uint32_t clampWindow(')
growth = body(full, 'uint32_t OrbtcpFlavour::limitCwndGrowth(').replace(
    'uint32_t OrbtcpFlavour::limitCwndGrowth(', 'constexpr uint32_t limitCwndGrowth(', 1)
preamble = r'''
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
struct State {
    uint32_t prevWnd=8000, snd_cwnd=8000, snd_mss=1448, additiveIncrease=0, ssthresh=0;
    double eta=0.95, txRate=0;
    bool initialPhase=false, endInitialPhase=false;
};
struct Connection { constexpr void emit(int, double) {} };
'''
common = r'''
    State storage;
    State *state=&storage;
    Connection connection;
    Connection *conn=&connection;
    bool limited=true, updateWindow=true;
    static constexpr int cwndLimitedSignal=1, txRateSignal=2;
    constexpr bool isCwndLimited() const { return limited; }
'''
source = preamble + 'constexpr ' + clamp + '\n'
for name, text in [('OrbtcpFlavour', full), ('OrbtcpPintFlavour', pint)]:
    method = body(text, f'uint32_t {name}::computeWnd(').replace(
        f'uint32_t {name}::computeWnd(', 'constexpr uint32_t computeWnd(', 1)
    source += f'struct {name} {{\n' + common + growth + method + '\n};\n'
source += r'''
template<class Controller> constexpr bool checkFloor() {
    Controller c;
    // The original target is 400 bytes: floor the emitted target before commit.
    if (c.computeWnd(19, false) != 1448 || c.state->prevWnd != 8000) return false;
    if (c.computeWnd(19, true) != 1448 || c.state->prevWnd != 1448 || c.updateWindow) return false;
    c.state->snd_cwnd=1448;
    if (c.computeWnd(19, true) != 1448 || c.state->prevWnd != 1448) return false;

    // A legacy sub-MSS state must not be held below the sender's floor by the growth gate.
    Controller old;
    old.state->snd_cwnd=400; old.state->prevWnd=400; old.limited=false;
    old.state->additiveIncrease=10000;
    if (old.computeWnd(0.5, true) != 1448 || old.state->prevWnd != 1448) return false;

    // Ordinary windows retain byte granularity and the existing growth gate.
    Controller normal;
    normal.state->snd_cwnd=2000; normal.state->prevWnd=2000;
    normal.state->additiveIncrease=100;
    if (normal.computeWnd(0.5, false) != 2100) return false;
    normal.limited=false;
    if (normal.computeWnd(0.5, false) != 2000) return false;

    // Startup caps cannot create an unsendable window either.
    Controller startup;
    startup.state->initialPhase=true; startup.state->ssthresh=400;
    if (startup.computeWnd(0.5, true) != 1448 || startup.state->prevWnd != 1448 ||
        startup.state->initialPhase) return false;
    return true;
}
static_assert(checkFloor<OrbtcpFlavour>(), "Full INT: target, commit and sender floor agree");
static_assert(checkFloor<OrbtcpPintFlavour>(), "PINT/Alpha: target, commit and sender floor agree");
'''
with tempfile.TemporaryDirectory(prefix='orb-window-floor-') as temp:
    path = Path(temp) / 'window.cc'
    path.write_text(source)
    subprocess.run(['clang++', '-std=c++23', '-fsyntax-only', str(path)], check=True)
print('OrbCC/PINT window-floor compile-time checks passed (no simulator build or run).')
