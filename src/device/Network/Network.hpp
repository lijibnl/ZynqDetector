/**
 * @file Network.hpp
 * @brief Class template definition of `Network` — Linux version.
 * @details
 * Transport-agnostic CRTP base. Derived classes (GermaniumZMQ,
 * GermaniumUDP, etc.) implement the actual wire protocol.
 *
 * Replaces the FreeRTOS lwIP version.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */
#pragma once

//===========================================================================//

#include <functional>

#include "Logger.hpp"
#include "DeviceMsg.hpp"

//===========================================================================//

template < typename DerivedNetwork >
class Network
{
public:

    using CommandHandler = std::function<void(DeviceMsg&)>;

    explicit Network( const Logger& logger );

    //------------------------------
    // CRTP helper
    //------------------------------
    auto& derived()             { return static_cast<DerivedNetwork&>(*this); }
    const auto& derived() const { return static_cast<const DerivedNetwork&>(*this); }

    /**
     * @brief Network initialization (transport-specific).
     */
    void network_init();

    /**
     * @brief Create network tasks / threads.
     */
    void create_network_tasks();

    /**
     * @brief Run the command loop (blocks).
     * @param handler  Callback for each received command.
     *
     * The derived class implements run_special() which:
     *   1. Receives a wire message
     *   2. Decodes it into DeviceMsg
     *   3. Calls handler(msg)
     *   4. Encodes the (possibly modified) DeviceMsg as a reply
     *   5. Sends the reply
     *   6. Loops
     */
    void run(CommandHandler handler);

    /**
     * @brief Signal the network loop to stop.
     */
    void stop();

protected:
    const Logger& logger_;
};

//===========================================================================//

#include "Network.tpp"
