/**
 * @file Register.cpp
 * @brief Member function definitions of `Register` — Linux version.
 * @details
 * Hardware build: /dev/mem mmap for FPGA register access.
 * Simulation build: heap-backed array with preset defaults.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */

//===========================================================================//

#include <cstdio>

#include "Register.hpp"

//===========================================================================//

#ifdef SIM_MODE

#include "GermaniumRegister.hpp"

Register::Register( uintptr_t /*base_addr*/, size_t map_size )
    : base_( nullptr )
{
    size_t nwords = map_size / sizeof(uint32_t);
    sim_mem_.resize( nwords, 0 );
    base_ = sim_mem_.data();

    for (size_t i = 0; i < GermaniumReg::SIM_DEFAULTS_COUNT; ++i) {
        base_[ GermaniumReg::SIM_DEFAULTS[i].offset ] =
            GermaniumReg::SIM_DEFAULTS[i].value;
    }

    printf("Register: simulation mode (heap-backed, %zu words)\n", nwords);
}

Register::~Register() {}

#else // !SIM_MODE

#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

Register::Register( uintptr_t base_addr, size_t map_size )
    : base_     ( nullptr  )
    , fd_       ( -1       )
    , map_size_ ( map_size )
{
    fd_ = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd_ < 0) {
        perror("Register: open /dev/mem failed");
        std::abort();
    }

    void* mapped = mmap( nullptr
                       , map_size_
                       , PROT_READ | PROT_WRITE
                       , MAP_SHARED
                       , fd_
                       , static_cast<off_t>(base_addr)
                       );

    if (mapped == MAP_FAILED) {
        perror("Register: mmap failed");
        close(fd_);
        std::abort();
    }

    base_ = static_cast<volatile uint32_t*>(mapped);

    printf("Register: hardware mode (mmap /dev/mem, %zu bytes)\n", map_size_);

}

Register::~Register()
{
    if (base_ && base_ != MAP_FAILED) {
        munmap(const_cast<uint32_t*>(base_), map_size_);
    }
    if (fd_ >= 0) {
        close(fd_);
    }
}

#endif // SIM_MODE

//===========================================================================//

void Register::write( uint16_t word_offset, uint32_t value )
{
    std::lock_guard<std::mutex> lock(mutex_);
    base_[word_offset] = value;
}

//===========================================================================//

uint32_t Register::read( uint16_t word_offset )
{
    std::lock_guard<std::mutex> lock(mutex_);
    return base_[word_offset];
}

//===========================================================================//

void Register::set_status( uint32_t status )
{
    write( 0xC, status );
}

//===========================================================================//

void Register::multi_access_start()
{
    mutex_.lock();
}

//===========================================================================//

void Register::multi_access_write( uint16_t word_offset, uint32_t value )
{
    base_[word_offset] = value;
}

//===========================================================================//

uint32_t Register::multi_access_read( uint16_t word_offset )
{
    return base_[word_offset];
}

//===========================================================================//

void Register::multi_access_end()
{
    mutex_.unlock();
}

//===========================================================================//
