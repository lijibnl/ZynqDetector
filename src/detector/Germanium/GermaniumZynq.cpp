/**
 * @file GermaniumZynq.cpp
 * @brief Member function definitions of `GermaniumZynq` — Linux version.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */

//===========================================================================//

#include "GermaniumZynq.hpp"

//===========================================================================//

GermaniumZynq::GermaniumZynq( const Logger& logger )
    : Zynq<GermaniumZynq>( REG_BASE_ADDR, REG_MAP_SIZE, logger )
    , i2c0_        ( 0, logger )
    , i2c1_        ( 1, logger )
    , i2c0_worker_ ( "I2C0" )
    , i2c1_worker_ ( "I2C1" )
    , xadc_worker_ ( "XADC" )
    , logger_      ( logger )
{}

//===========================================================================//

void GermaniumZynq::create_device_access_tasks_special()
{
    ///< I2C bus 0 worker: Tmp100 ×3
    i2c0_worker_.set_handler(
        [this](uint32_t addr, uint32_t value) -> uint32_t {
            ///< addr encodes slave address; value encodes operation
            ///< Specific dispatch handled by GermaniumDetector
            (void)addr; (void)value;
            return 0;
        }
    );
    i2c0_worker_.start();

    ///< I2C bus 1 worker: Dac7678, Ltc2309 ×2
    i2c1_worker_.set_handler(
        [this](uint32_t addr, uint32_t value) -> uint32_t {
            (void)addr; (void)value;
            return 0;
        }
    );
    i2c1_worker_.start();

    ///< XADC worker: on-chip temperature/voltage via /sys/bus/iio
    xadc_worker_.set_handler(
        [this](uint32_t addr, uint32_t value) -> uint32_t {
            (void)addr; (void)value;
            ///< TODO: read /sys/bus/iio/devices/iio:device0/...
            return 0;
        }
    );
    xadc_worker_.start();
}

//===========================================================================//
