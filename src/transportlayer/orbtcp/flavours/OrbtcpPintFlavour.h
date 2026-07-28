//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//

#ifndef TRANSPORTLAYER_ORBTCP_FLAVOURS_ORBTCPPINTFLAVOUR_H_
#define TRANSPORTLAYER_ORBTCP_FLAVOURS_ORBTCPPINTFLAVOUR_H_

#include <cstdint>

#include "OrbtcpFlavour.h"

namespace inet {
namespace tcp {

/**
 * OrbCC variant consuming one HPCC-PINT-style bottleneck record.
 */
class OrbtcpPintFlavour : public OrbtcpFlavour
{
  protected:
    double pintFeedbackProbability = 1;
    uint32_t lastPathDigest = 0;
    bool hasPathDigest = false;
    simtime_t lastPintFeedback;

    virtual void initialize() override;

  public:
    OrbtcpPintFlavour() = default;
    virtual ~OrbtcpPintFlavour() = default;

    virtual double measureInflight(IntDataVec intData) override;
};

} // namespace tcp
} // namespace inet

#endif
