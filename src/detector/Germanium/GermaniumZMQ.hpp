/**
 * @file GermaniumZMQ.hpp
 * @brief Class definition of `GermaniumZMQ` — ZMQ REP transport.
 * @details
 * Implements Network<GermaniumZMQ> using ZMQ REP socket on port 5555.
 * Translates between 12-byte ZMQ wire messages and DeviceMsg.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */
#pragma once

//===========================================================================//

#include <atomic>
#include <functional>
#include <cstdint>

#include "Network.hpp"
#include "DeviceMsg.hpp"

//===========================================================================//

class GermaniumZMQ : public Network<GermaniumZMQ>
{
public:
    ///< Wire format — matches DeviceMsg layout for simplicity
    struct WireMsg {
        uint32_t cmd;
        uint32_t addr;
        uint32_t value;
    };

    static constexpr int ZMQ_PORT = 5555;

    explicit GermaniumZMQ( const Logger& logger );
    ~GermaniumZMQ();

    /**
     * @brief CRTP hook — initialize ZMQ context and socket.
     */
    void network_init_special();

    /**
     * @brief CRTP hook — no separate threads needed (run_special is the loop).
     */
    void create_network_tasks_special();

    /**
     * @brief CRTP hook — blocking command loop.
     * @param handler  Callback to GermaniumDetector::handle_command().
     */
    void run_special(Network::CommandHandler handler);

    /**
     * @brief CRTP hook — signal shutdown.
     */
    void stop_special();

private:
    void*              zmq_ctx_;
    void*              zmq_rep_;
    std::atomic<bool>  running_;
};

//===========================================================================//
