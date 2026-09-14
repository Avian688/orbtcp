// SPDX-License-Identifier: LGPL-3.0-or-later
package orbtcp.queueing.queue;

// FIFO PINT queue retaining the original telemetry and mutable capacities.
simple PintFrontTailQueue extends PintQueue
{
    parameters:
        @class("inet::queueing::PintQueue");
        dropperClass = default("inet::queueing::PacketAtCollectionBeginDropper");
}
