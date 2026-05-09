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
    static constexpr uint32_t GET_GLOBAL      = 0x11;
    static constexpr uint32_t SET_CHANNEL     = 0x12;
    static constexpr uint32_t GET_CHANNEL     = 0x13;
    static constexpr uint32_t MARS_LOAD       = 0x14;
    static constexpr uint32_t ADC_CLK_SKEW_SET  = 0x20;
    static constexpr uint32_t I2C_TEMP_READ     = 0x21;
    static constexpr uint32_t XADC_READ         = 0x22;
    static constexpr uint32_t I2C_DAC_WRITE     = 0x23;
    static constexpr uint32_t I2C_ADC_READ      = 0x24;
    static constexpr uint32_t I2C_DAC_INIT      = 0x25;
    static constexpr uint32_t ADC_CLK_SKEW_READ = 0x26;
    static constexpr uint32_t SET_LOG_LEVEL   = 0x30;
}

//===========================================================================//
