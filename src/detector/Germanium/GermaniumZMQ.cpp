/**
 * @file GermaniumZMQ.cpp
 * @brief Member function definitions of `GermaniumZMQ`.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */

//===========================================================================//

#include <cstdio>
#include <cstring>

#include <zmq.h>

#include "GermaniumZMQ.hpp"

//===========================================================================//

GermaniumZMQ::GermaniumZMQ( const Logger& logger )
    : Network<GermaniumZMQ>( logger )
    , zmq_ctx_  ( nullptr )
    , zmq_rep_  ( nullptr )
    , running_  ( false   )
{}

//===========================================================================//

GermaniumZMQ::~GermaniumZMQ()
{
    stop_special();

    if (zmq_rep_) {
        zmq_close(zmq_rep_);
        zmq_rep_ = nullptr;
    }
    if (zmq_ctx_) {
        zmq_ctx_destroy(zmq_ctx_);
        zmq_ctx_ = nullptr;
    }
}

//===========================================================================//

void GermaniumZMQ::network_init_special()
{
    zmq_ctx_ = zmq_ctx_new();
    if (!zmq_ctx_) {
        logger_.log_error("GermaniumZMQ: failed to create ZMQ context");
        return;
    }

    zmq_rep_ = zmq_socket(zmq_ctx_, ZMQ_REP);
    if (!zmq_rep_) {
        logger_.log_error("GermaniumZMQ: failed to create ZMQ REP socket");
        return;
    }

    char endpoint[64];
    snprintf(endpoint, sizeof(endpoint), "tcp://*:%d", ZMQ_PORT);

    if (zmq_bind(zmq_rep_, endpoint) != 0) {
        logger_.log_error("GermaniumZMQ: failed to bind to %s: %s",
                          endpoint, zmq_strerror(zmq_errno()));
        return;
    }

    logger_.log_debug("GermaniumZMQ: listening on %s", endpoint);
}

//===========================================================================//

void GermaniumZMQ::create_network_tasks_special()
{
    ///< No separate threads — run_special() IS the blocking loop.
}

//===========================================================================//

void GermaniumZMQ::run_special(Network::CommandHandler handler)
{
    running_ = true;
    WireMsg wire;

    while (running_) {
        int rc = zmq_recv(zmq_rep_, &wire, sizeof(wire), 0);
        if (rc < 0) {
            if (zmq_errno() == ETERM || zmq_errno() == EINTR) {
                break;  ///< context destroyed or signal
            }
            logger_.log_error("GermaniumZMQ: recv error: %s", zmq_strerror(zmq_errno()));
            continue;
        }

        if (rc != sizeof(WireMsg)) {
            logger_.log_warn("GermaniumZMQ: unexpected message size %d (expected %zu)",
                             rc, sizeof(WireMsg));
            ///< Must still send a reply to maintain REQ/REP pattern
            zmq_send(zmq_rep_, &wire, sizeof(wire), 0);
            continue;
        }

        ///< Decode wire → DeviceMsg
        DeviceMsg msg;
        msg.cmd   = wire.cmd;
        msg.addr  = wire.addr;
        msg.value = wire.value;

        ///< Dispatch to detector
        handler(msg);

        ///< Encode DeviceMsg → wire reply
        wire.cmd   = msg.cmd;
        wire.addr  = msg.addr;
        wire.value = msg.value;

        zmq_send(zmq_rep_, &wire, sizeof(wire), 0);
    }
}

//===========================================================================//

void GermaniumZMQ::stop_special()
{
    running_ = false;
    ///< Closing context will unblock zmq_recv with ETERM
    if (zmq_ctx_) {
        zmq_ctx_shutdown(zmq_ctx_);
    }
}

//===========================================================================//
