/**
 * @file GermaniumDetector.cpp
 * @brief Member function definitions of `GermaniumDetector` — Linux version.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */

//===========================================================================//

#include "GermaniumDetector.hpp"

//===========================================================================//

GermaniumDetector::GermaniumDetector()
    : ZynqDevice< GermaniumDetector
                , GermaniumZMQ
                , GermaniumZynq
                >()
    , mars_worker_( "MARS" )
{
    create_queues();

    create_components();

    network_init();

    create_tasks();
}

//===========================================================================//

void GermaniumDetector::create_queues_special()
{
    ///< No additional queues — DeviceWorkers use ThreadQueue internally.
}

//===========================================================================//

void GermaniumDetector::create_components_special()
{
    auto z = std::make_unique<GermaniumZynq>( this->logger_ );
    this->set_zynq( std::move(z) );

    auto n = std::make_unique<GermaniumZMQ>( this->logger_ );
    this->set_network( std::move(n) );

    mars_ = std::make_unique<Mars>( this->zynq_->reg() );
}

//===========================================================================//

void GermaniumDetector::create_tasks_special()
{
    ///< Mars worker — runs stuff_mars (long-running, holds register mutex)
    mars_worker_.set_handler(
        [this](uint32_t addr, uint32_t /*value*/) -> uint32_t {
            ///< addr = chip_mask
            mars_->load( static_cast<uint16_t>(addr) );
            return 0;
        }
    );
    mars_worker_.start();
}

//===========================================================================//

void GermaniumDetector::handle_command( DeviceMsg& msg )
{
    switch ( msg.cmd )
    {
        case DeviceCmd::REG_READ:
        {
            ///< Submit read to register worker; bit 15 set = read convention
            auto fut = zynq_->register_worker().submit( msg.addr | 0x8000, 0 );
            msg.value = fut.get();
            break;
        }

        case DeviceCmd::REG_WRITE:
        {
            ///< Submit write to register worker
            auto fut = zynq_->register_worker().submit( msg.addr, msg.value );
            fut.get();
            break;
        }

        case DeviceCmd::SET_GLOBAL:
        {
            ///< Inline — just updates chipstr state (fast, no hardware I/O)
            uint16_t chip_mask = static_cast<uint16_t>( (msg.addr >> 16) & 0xFFF );
            uint16_t field_id  = static_cast<uint16_t>( msg.addr & 0xFFFF );
            mars_->set_global_field( chip_mask, field_id, msg.value );
            break;
        }

        case DeviceCmd::SET_CHANNEL:
        {
            ///< Inline — just updates chanstr state (fast, no hardware I/O)
            uint16_t channel  = static_cast<uint16_t>( (msg.addr >> 16) & 0xFFF );
            uint16_t field_id = static_cast<uint16_t>( msg.addr & 0xFFFF );
            mars_->set_channel_field( channel, field_id, msg.value );
            break;
        }

        case DeviceCmd::MARS_LOAD:
        {
            ///< Submit to mars_worker_ (long-running operation)
            uint16_t chip_mask = static_cast<uint16_t>( msg.addr & 0xFFF );
            auto fut = mars_worker_.submit( chip_mask, 0 );
            fut.get();
            break;
        }

        default:
            logger_.log_warn( "GermaniumDetector: unknown cmd 0x%02X", msg.cmd );
            break;
    }
}

//===========================================================================//
