/**
 * @file detector_main.cpp
 * @brief Starting point of the program — Linux version.
 * @details
 * Creates GermaniumDetector (constructor initializes everything),
 * then enters the blocking ZMQ command loop.
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */

//===========================================================================//

#include <cstdio>
#include <csignal>

#include "GermaniumDetector.hpp"

//===========================================================================//

static GermaniumDetector* g_det = nullptr;

static void signal_handler( int /*sig*/ )
{
    if (g_det) {
        g_det->stop();
    }
}

//===========================================================================//

int main()
{
    printf("GermaniumDetector starting...\n");

    GermaniumDetector det;
    g_det = &det;

    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    det.run();

    printf("GermaniumDetector stopped.\n");
    return 0;
}

//===========================================================================//
