/**
 * @file Register.cpp
 * @brief Member function definitions of `Register` — Linux version.
 * @details
 * Uses /dev/mem mmap for FPGA register access.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */

//===========================================================================//

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "Register.hpp"

//===========================================================================//

Register::Register( uintptr_t base_addr, size_t map_size )
    : base_     ( nullptr   )
    , fd_       ( -1        )
    , map_size_ ( map_size  )
{
    fd_ = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd_ < 0) {
        perror("Register: failed to open /dev/mem");
        return;
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
        fd_ = -1;
        return;
    }

    base_ = static_cast<volatile uint32_t*>(mapped);
}

//===========================================================================//

Register::~Register()
{
    if (base_ && base_ != MAP_FAILED) {
        munmap(const_cast<uint32_t*>(base_), map_size_);
    }
    if (fd_ >= 0) {
        close(fd_);
    }
}

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
