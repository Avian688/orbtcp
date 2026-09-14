// SPDX-License-Identifier: LGPL-3.0-or-later
package orbtcp.queueing.queue;

// FIFO INT queue with INET's drop-head overflow policy.
simple IntFrontTailQueue extends IntQueue
{
    parameters:
        @class("inet::queueing::IntQueue");
        dropperClass = default("inet::queueing::PacketAtCollectionBeginDropper");
}
