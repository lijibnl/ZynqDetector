/**
 * @file Network.tpp
 * @brief Member function definitions of `Network` — Linux version.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */
#pragma once

//===========================================================================//

/**
 * @brief Network constructor.
 */
template < typename DerivedNetwork >
Network<DerivedNetwork>::Network( const Logger& logger )
    : logger_( logger )
{}

//===========================================================================//

/**
 * @brief Network initialization — delegates to derived.
 */
template < typename DerivedNetwork >
void Network<DerivedNetwork>::network_init()
{
    derived().network_init_special();
}

//===========================================================================//

/**
 * @brief Create network tasks — delegates to derived.
 */
template < typename DerivedNetwork >
void Network<DerivedNetwork>::create_network_tasks()
{
    derived().create_network_tasks_special();
}

//===========================================================================//

/**
 * @brief Run the command loop — delegates to derived.
 */
template < typename DerivedNetwork >
void Network<DerivedNetwork>::run(CommandHandler handler)
{
    derived().run_special(std::move(handler));
}

//===========================================================================//

/**
 * @brief Signal shutdown — delegates to derived.
 */
template < typename DerivedNetwork >
void Network<DerivedNetwork>::stop()
{
    derived().stop_special();
}

//===========================================================================//
