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

#include "GermaniumDetectorProtocol.hpp"

//===========================================================================//

using DeviceMsg = GermaniumProtocol::Message;

//===========================================================================//

///< Command codes
namespace DeviceCmd {
    static constexpr uint32_t REG_READ          = GermaniumProtocol::Command::REG_READ;
    static constexpr uint32_t REG_WRITE         = GermaniumProtocol::Command::REG_WRITE;
    static constexpr uint32_t SET_GLOBAL        = GermaniumProtocol::Command::MARS_GLOBAL_SET;
    static constexpr uint32_t MARS_GLOBAL_READ  = GermaniumProtocol::Command::MARS_GLOBAL_READ;
    static constexpr uint32_t GET_GLOBAL        = GermaniumProtocol::Command::MARS_GLOBAL_READ;
    static constexpr uint32_t SET_CHANNEL       = GermaniumProtocol::Command::MARS_CHANNEL_SET;
    static constexpr uint32_t MARS_CHANNEL_READ = GermaniumProtocol::Command::MARS_CHANNEL_READ;
    static constexpr uint32_t GET_CHANNEL       = GermaniumProtocol::Command::MARS_CHANNEL_READ;
    static constexpr uint32_t MARS_LOAD         = GermaniumProtocol::Command::MARS_LOAD;
    static constexpr uint32_t ADC_CLK_SKEW_SET  = GermaniumProtocol::Command::ADC_CLK_SKEW_SET;
    static constexpr uint32_t ADC_CLK_SKEW      = GermaniumProtocol::Command::ADC_CLK_SKEW_SET;
    static constexpr uint32_t I2C_TEMP_READ     = GermaniumProtocol::Command::I2C_TEMP_READ;
    static constexpr uint32_t XADC_READ         = GermaniumProtocol::Command::XADC_READ;
    static constexpr uint32_t I2C_DAC_WRITE     = GermaniumProtocol::Command::I2C_DAC_WRITE;
    static constexpr uint32_t I2C_ADC_READ      = GermaniumProtocol::Command::I2C_ADC_READ;
    static constexpr uint32_t I2C_DAC_INIT      = GermaniumProtocol::Command::I2C_DAC_INIT;
    static constexpr uint32_t ADC_CLK_SKEW_READ = GermaniumProtocol::Command::ADC_CLK_SKEW_READ;
    static constexpr uint32_t SET_LOG_LEVEL     = GermaniumProtocol::Command::SET_LOG_LEVEL;
    static constexpr uint32_t GET_PROTOCOL_VERSION = GermaniumProtocol::Command::GET_PROTOCOL_VERSION;
    static constexpr uint32_t HEARTBEAT         = GermaniumProtocol::Command::HEARTBEAT;
}

//===========================================================================//
