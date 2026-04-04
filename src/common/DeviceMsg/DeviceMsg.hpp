/**
 * @file DeviceMsg.hpp
 * @brief Transport-agnostic application-level command structure.
 * @details
 * All commands between the Network layer and the Detector use this
 * uniform message format. Each Network derived class translates
 * between its wire format and DeviceMsg.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */
#pragma once

//===========================================================================//

#include <cstdint>

//===========================================================================//

struct DeviceMsg {
    uint32_t cmd;       // command code
    uint32_t addr;      // word offset, field encoding, or bus-specific address
    uint32_t value;     // data in/out
};

//===========================================================================//

///< Command codes
namespace DeviceCmd {
    static constexpr uint32_t REG_READ        = 0x00;
    static constexpr uint32_t REG_WRITE       = 0x01;
    static constexpr uint32_t SET_GLOBAL      = 0x10;
    static constexpr uint32_t SET_CHANNEL     = 0x11;
    static constexpr uint32_t MARS_LOAD       = 0x12;
}

//===========================================================================//
