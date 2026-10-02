// SPDX-License-Identifier: LGPL-3.0-or-later
#include "OmegaPriceTag.h"

namespace inet {
Register_Class(OmegaPriceTag);

void OmegaPriceTag::parsimPack(cCommBuffer *buffer) const
{
    TagBase::parsimPack(buffer);
    buffer->pack(price);
    buffer->pack(capacity);
    buffer->pack(sampledAt);
    buffer->pack(hops);
    buffer->pack(pricedHops);
    buffer->pack(echoed);
}

void OmegaPriceTag::parsimUnpack(cCommBuffer *buffer)
{
    TagBase::parsimUnpack(buffer);
    buffer->unpack(price);
    buffer->unpack(capacity);
    buffer->unpack(sampledAt);
    buffer->unpack(hops);
    buffer->unpack(pricedHops);
    buffer->unpack(echoed);
}
} // namespace inet
