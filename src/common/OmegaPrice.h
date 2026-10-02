// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_OMEGA_PRICE_H
#define INET_OMEGA_PRICE_H

#include <algorithm>
#include <cstdint>
#include <limits>

namespace inet { namespace omega {

// All prices and gains are dimensionless. Rates are bytes/s, times seconds.
struct LinkPrice {
    double memory = 0;
    double advertised = 0;

    constexpr void update(double offeredRate, double capacity, double queueBytes,
            double dt, double referenceRtt, double target,
            double integralGain, double proportionalGain)
    {
        const double error = offeredRate / (target * capacity) - 1;
        memory = std::max(0.0, memory + integralGain * dt / referenceRtt * (1 + memory) * error);
        // The transient terms damp the delayed source/price loop and drain queues.
        // They vanish at a queue-free equilibrium; memory does not.
        advertised = std::max(0.0, memory + proportionalGain * (1 + memory) * error +
                queueBytes / (capacity * referenceRtt));
    }
};

constexpr double nextRate(double rate, double totalRate, double pathPrice,
        double rateScale, double gain, double dt, double referenceRtt,
        double floor, double ceiling, unsigned int paths = 1)
{
    // Positive connection-wide preconditioning of the log-utility gradient.
    // It controls step size at small X without multiplying by this path's rate.
    const double residual = 1 - totalRate / rateScale * pathPrice;
    const double change = gain * dt / referenceRtt * totalRate / paths *
            std::clamp(residual, -1.0, 1.0);
    return std::clamp(rate + change, floor, std::max(floor, ceiling));
}

constexpr uint32_t windowForRate(double rate, double rtt, uint32_t mss)
{
    return std::max(mss, static_cast<uint32_t>(std::min(rate * rtt,
            static_cast<double>(std::numeric_limits<uint32_t>::max()))));
}

} } // namespace inet::omega
#endif
