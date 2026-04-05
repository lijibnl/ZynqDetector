/**
 * @file GermaniumRegister.hpp
 * @brief FPGA register map and simulation default values.
 * @details
 * Shared between ZynqDetector (Zynq server) and ADGermaniumZMQ (IOC).
 * The server initializes simulated registers with these defaults;
 * the IOC reads them back to verify connectivity.
 *
 * Register offsets are word offsets into the FPGA AXI register space
 * at base address 0x43C00000.
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
#include <utility>

//===========================================================================//
//  Register word offsets
//===========================================================================//

namespace GermaniumReg {

static constexpr uint16_t MARS_CONF_LOAD    = 0;
static constexpr uint16_t LEDS              = 1;
static constexpr uint16_t MARS_CONFIG       = 2;
static constexpr uint16_t VERSIONREG        = 3;
static constexpr uint16_t MARS_CALPULSE     = 4;
static constexpr uint16_t MARS_PIPE_DELAY   = 5;
static constexpr uint16_t DETECTOR_TYPE     = 6;
static constexpr uint16_t MARS_RDOUT_ENB    = 8;
static constexpr uint16_t EVENT_TIME_CNTR   = 9;
static constexpr uint16_t SIM_EVT_SEL       = 10;
static constexpr uint16_t SIM_EVENT_RATE    = 11;
static constexpr uint16_t ADC_SPI           = 12;
static constexpr uint16_t CALPULSE_CNT      = 16;
static constexpr uint16_t CALPULSE_RATE     = 17;
static constexpr uint16_t CALPULSE_WIDTH    = 18;
static constexpr uint16_t CALPULSE_MODE     = 19;
static constexpr uint16_t TD_CAL            = 20;
static constexpr uint16_t EVENT_FIFO_DATA   = 24;
static constexpr uint16_t EVENT_FIFO_CNT    = 25;
static constexpr uint16_t EVENT_FIFO_CTRL   = 26;
static constexpr uint16_t UDP_IP_ADDR       = 40;
static constexpr uint16_t TRIG              = 52;
static constexpr uint16_t COUNT_TIME_LO     = 53;
static constexpr uint16_t COUNT_TIME_HI     = 54;
static constexpr uint16_t FRAME_NO          = 55;
static constexpr uint16_t COUNT_MODE        = 56;

//===========================================================================//
//  Simulation default values
//
//  When the server runs in simulation mode (no /dev/mem), registers are
//  initialized to these values.  The IOC can read VERSIONREG to verify
//  connectivity and confirm it is talking to the correct firmware/sim.
//
//  Format: { word_offset, default_value }
//===========================================================================//

static constexpr uint32_t SIM_VERSION       = 0x20260404;   // firmware version
static constexpr uint32_t SIM_DETECTOR_TYPE = 0x0001;       // 1 = Germanium

struct RegDefault {
    uint16_t offset;
    uint32_t value;
};

static constexpr RegDefault SIM_DEFAULTS[] = {
    { VERSIONREG,       SIM_VERSION       },
    { DETECTOR_TYPE,    SIM_DETECTOR_TYPE  },
    { MARS_CONFIG,      0x00000000        },
    { MARS_PIPE_DELAY,  0x00000005        },
    { MARS_RDOUT_ENB,   0x00000000        },
    { SIM_EVT_SEL,      0x00000000        },
    { SIM_EVENT_RATE,   0x00000000        },
    { CALPULSE_CNT,     0x00000000        },
    { CALPULSE_RATE,    0x00000000        },
    { CALPULSE_WIDTH,   0x00000000        },
    { CALPULSE_MODE,    0x00000000        },
    { TD_CAL,           0x00000000        },
    { EVENT_FIFO_CTRL,  0x00000000        },
    { TRIG,             0x00000000        },
    { COUNT_TIME_LO,    0x00000000        },
    { COUNT_TIME_HI,    0x00000000        },
    { FRAME_NO,         0x00000000        },
    { COUNT_MODE,       0x00000000        },
};

static constexpr size_t SIM_DEFAULTS_COUNT =
    sizeof(SIM_DEFAULTS) / sizeof(SIM_DEFAULTS[0]);

} // namespace GermaniumReg

//===========================================================================//
