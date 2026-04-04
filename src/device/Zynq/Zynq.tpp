/**
 * @file Zynq.tpp
 * @brief Member function definitions of `Zynq` — Linux version.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */
#pragma once

//===========================================================================//

#include "Register.hpp"

//===========================================================================//

/**
 * @brief Zynq constructor.
 * @param base_addr  FPGA register base address for mmap.
 * @param map_size   Size of the mmap region.
 * @param logger     Reference to the logger.
 */
template < typename DerivedZynq >
Zynq<DerivedZynq>::Zynq
    ( uintptr_t     base_addr
    , size_t        map_size
    , const Logger& logger
    )
    : reg_              ( std::make_unique<Register>( base_addr, map_size ) )
    , register_worker_  ( "REG" )
    , logger_           ( logger )
{}

//===========================================================================//

/**
 * @brief Create device access worker threads.
 */
template < typename DerivedZynq >
void Zynq<DerivedZynq>::create_device_access_tasks()
{
    ///< Set up register worker handler
    register_worker_.set_handler(
        [this](uint32_t addr, uint32_t value) -> uint32_t {
            ///< Bit 15 set = read, else write (matches FreeRTOS convention)
            if (addr & 0x8000) {
                return reg_->read(addr & 0x7FFF);
            } else {
                reg_->write(static_cast<uint16_t>(addr), value);
                return 0;
            }
        }
    );
    register_worker_.start();

    ///< Derived class creates its own workers (I2C, XADC, etc.)
    derived().create_device_access_tasks_special();
}

//===========================================================================//
