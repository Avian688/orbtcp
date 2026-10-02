// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_OMEGA_PRICE_TAG_H
#define INET_OMEGA_PRICE_TAG_H

#include "inet/common/TagBase.h"

namespace inet {

// Separate from PINT's max-U record: a sum over forward links, echoed unchanged
// by ACKs. Simulator metadata; wire size/quantisation are intentionally abstract.
class OmegaPriceTag : public TagBase
{
  public:
    double price = 0;
    double capacity = 0; // Minimum forward link capacity, bytes/s
    simtime_t sampledAt = SIMTIME_ZERO; // First forward queue observation
    unsigned int hops = 0;
    unsigned int pricedHops = 0;
    bool echoed = false;

    virtual OmegaPriceTag *dup() const override { return new OmegaPriceTag(*this); }
    virtual void parsimPack(cCommBuffer *buffer) const override;
    virtual void parsimUnpack(cCommBuffer *buffer) override;
};

} // namespace inet
#endif
