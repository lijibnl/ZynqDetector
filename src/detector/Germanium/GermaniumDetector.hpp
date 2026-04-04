/**
 * @file GermaniumDetector.hpp
 * @brief Class definition of `GermaniumDetector` — Linux version.
 * @details
 * The main detector class. Inherits ZynqDevice with 3 CRTP params.
 * Owns Mars and all detector-specific components.
 * Single handle_command() dispatches DeviceMsg to the appropriate
 * worker or handler.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */
#pragma once

//===========================================================================//

#include <memory>

#include "ZynqDevice.hpp"
#include "GermaniumZMQ.hpp"
#include "GermaniumZynq.hpp"
#include "Mars.hpp"
#include "DeviceWorker.hpp"
#include "DeviceMsg.hpp"

//===========================================================================//

class GermaniumDetector : public ZynqDevice< GermaniumDetector
                                           , GermaniumZMQ
                                           , GermaniumZynq
                                           >
{
public:
    GermaniumDetector();

    /**
     * @brief CRTP hook — create queues (no additional queues needed).
     */
    void create_queues_special();

    /**
     * @brief CRTP hook — create Zynq, Network, Mars, and other components.
     */
    void create_components_special();

    /**
     * @brief CRTP hook — start detector-specific worker threads.
     */
    void create_tasks_special();

    /**
     * @brief Single command dispatcher.
     * @param msg  Application-level command from the network layer.
     *             msg.value is modified in place for read responses.
     *
     * Called by ZynqDevice::run() → Network::run() → handler(msg).
     */
    void handle_command( DeviceMsg& msg );

private:
    std::unique_ptr<Mars>  mars_;
    DeviceWorker           mars_worker_;
};

//===========================================================================//
