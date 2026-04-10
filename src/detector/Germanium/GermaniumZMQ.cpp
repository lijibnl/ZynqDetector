/**
 * @file GermaniumZMQ.cpp
 * @brief Member function definitions of `GermaniumZMQ`.
 *
 * Async PUSH-PULL transport:
 *   PULL :5555  — receives commands from IOC
 *   PUSH :5557  — sends replies to IOC
 *   rx path: recv → dispatch (non-blocking)
 *   tx path: pop tx_queue → send
 *
 * @author Ji Li <liji@bnl.gov>
 * @date 04/04/2026
 * @copyright
 * Copyright (c) 2026 Brookhaven National Laboratory
 * @license BSD 3-Clause License. See LICENSE file for details.
 */

//===========================================================================//

#include <cstdio>
#include <cstring>

#include <zmq.h>

#include "GermaniumZMQ.hpp"

//===========================================================================//

GermaniumZMQ::GermaniumZMQ( const Logger& logger )
    : Network<GermaniumZMQ>( logger )
    , zmq_ctx_  ( nullptr )
    , zmq_rx_ ( nullptr )
    , zmq_tx_ ( nullptr )
    , running_  ( false   )
{}

//===========================================================================//

GermaniumZMQ::~GermaniumZMQ()
{
    stop_special();

    if (tx_thread_.joinable())
        tx_thread_.join();

    if (zmq_rx_) {
        zmq_close(zmq_rx_);
        zmq_rx_ = nullptr;
    }
    if (zmq_tx_) {
        zmq_close(zmq_tx_);
        zmq_tx_ = nullptr;
    }
    if (zmq_ctx_) {
        zmq_ctx_destroy(zmq_ctx_);
        zmq_ctx_ = nullptr;
    }
}

//===========================================================================//

void GermaniumZMQ::network_init_special()
{
    zmq_ctx_ = zmq_ctx_new();
    if (!zmq_ctx_) {
        logger_.log_error("GermaniumZMQ: failed to create ZMQ context");
        return;
    }

    ///< rx socket — receives commands
    zmq_rx_ = zmq_socket(zmq_ctx_, ZMQ_PULL);
    if (!zmq_rx_) {
        logger_.log_error("GermaniumZMQ: failed to create PULL socket");
        return;
    }

    char rx_ep[64];
    snprintf(rx_ep, sizeof(rx_ep), "tcp://*:%d", ZMQ_CMD_PORT);

    if (zmq_bind(zmq_rx_, rx_ep) != 0) {
        logger_.log_error("GermaniumZMQ: failed to bind PULL to %s: %s",
                          rx_ep, zmq_strerror(zmq_errno()));
        return;
    }

    logger_.log_debug("GermaniumZMQ: rx socket bound on %s", rx_ep);

    ///< tx socket — sends replies
    zmq_tx_ = zmq_socket(zmq_ctx_, ZMQ_PUSH);
    if (!zmq_tx_) {
        logger_.log_error("GermaniumZMQ: failed to create PUSH socket");
        return;
    }

    char tx_ep[64];
    snprintf(tx_ep, sizeof(tx_ep), "tcp://*:%d", ZMQ_REPLY_PORT);

    if (zmq_bind(zmq_tx_, tx_ep) != 0) {
        logger_.log_error("GermaniumZMQ: failed to bind PUSH to %s: %s",
                          tx_ep, zmq_strerror(zmq_errno()));
        return;
    }

    logger_.log_debug("GermaniumZMQ: tx socket bound on %s", tx_ep);
}

//===========================================================================//

void GermaniumZMQ::create_network_tasks_special()
{
    ///< Threads are created in run_special().
}

//===========================================================================//

void GermaniumZMQ::run_special(Network::CommandDispatcher dispatcher)
{
    running_ = true;

    ///< Start tx_thread
    tx_thread_ = std::thread( &GermaniumZMQ::tx_loop, this );

    logger_.log_debug("GermaniumZMQ: rx_thread running");

    ///< rx loop — blocks on PULL socket
    WireMsg wire;
    while (running_) {
        int rc = zmq_recv(zmq_rx_, &wire, sizeof(wire), 0);
        if (rc < 0) {
            if (zmq_errno() == ETERM || zmq_errno() == EINTR) {
                break;
            }
            logger_.log_error("GermaniumZMQ: recv error: %s",
                              zmq_strerror(zmq_errno()));
            continue;
        }

        if (rc != sizeof(WireMsg)) {
            logger_.log_warn("GermaniumZMQ: unexpected msg size %d (expected %zu)",
                             rc, sizeof(WireMsg));
            continue;
        }

        DeviceMsg msg;
        msg.cmd   = wire.cmd;
        msg.addr  = wire.addr;
        msg.value = wire.value;


#ifdef SIM_MODE
    logger_.log_debug("ZMQ RX: cmd=0x%02X addr=0x%04X value=0x%08X",
              msg.cmd, msg.addr, msg.value);
    logger_.log_debug("ZMQ RX (decoded): %s",
              format_rx_msg(msg).c_str());
#endif
//===========================================================================//
// Helper functions for decoding command and address/parameter info
#include <string>
#include <sstream>
#include <iomanip>
#include "germaniumDetectorTypes.hpp"
#include "germaniumDetectorRegister.hpp"

namespace {

const char* decode_cmd(uint32_t cmd) {
    switch (cmd) {
        case DeviceCmd::REG_READ:      return "REG_READ";
        case DeviceCmd::REG_WRITE:     return "REG_WRITE";
        case DeviceCmd::SET_GLOBAL:    return "SET_GLOBAL";
        case DeviceCmd::SET_CHANNEL:   return "SET_CHANNEL";
        case DeviceCmd::MARS_LOAD:     return "MARS_LOAD";
        case DeviceCmd::ADC_CLK_SKEW:  return "ADC_CLK_SKEW";
        case DeviceCmd::I2C_TEMP_READ: return "I2C_TEMP_READ";
        case DeviceCmd::XADC_READ:     return "XADC_READ";
        case DeviceCmd::I2C_DAC_WRITE: return "I2C_DAC_WRITE";
        case DeviceCmd::I2C_ADC_READ:  return "I2C_ADC_READ";
        case DeviceCmd::I2C_DAC_INIT:  return "I2C_DAC_INIT";
        case DeviceCmd::SET_LOG_LEVEL: return "SET_LOG_LEVEL";
        default: return "UNKNOWN";
    }
}

// Register name lookup
const char* decode_reg(uint32_t addr) {
    switch (addr) {
        case MARS_CONF_LOAD:    return "MARS_CONF_LOAD";
        case LEDS:              return "LEDS";
        case MARS_CONFIG:       return "MARS_CONFIG";
        case VERSIONREG:        return "VERSIONREG";
        case MARS_CALPULSE:     return "MARS_CALPULSE";
        case MARS_PIPE_DELAY:   return "MARS_PIPE_DELAY";
        case DETECTOR_TYPE:     return "DETECTOR_TYPE";
        case MARS_RDOUT_ENB:    return "MARS_RDOUT_ENB";
        case EVENT_TIME_CNTR:   return "EVENT_TIME_CNTR";
        case SIM_EVT_SEL:       return "SIM_EVT_SEL";
        case SIM_EVENT_RATE:    return "SIM_EVENT_RATE";
        case ADC_SPI:           return "ADC_SPI";
        case CALPULSE_CNT:      return "CALPULSE_CNT";
        case CALPULSE_RATE:     return "CALPULSE_RATE";
        case CALPULSE_WIDTH:    return "CALPULSE_WIDTH";
        case CALPULSE_MODE:     return "CALPULSE_MODE";
        case TD_CAL:            return "TD_CAL";
        case EVENT_FIFO_DATA:   return "EVENT_FIFO_DATA";
        case EVENT_FIFO_CNT:    return "EVENT_FIFO_CNT";
        case EVENT_FIFO_CTRL:   return "EVENT_FIFO_CTRL";
        case UDP_IP_ADDR:       return "UDP_IP_ADDR";
        case TRIG:              return "TRIG";
        case COUNT_TIME_LO:     return "COUNT_TIME_LO";
        case COUNT_TIME_HI:     return "COUNT_TIME_HI";
        case FRAME_NO:          return "FRAME_NO";
        case COUNT_MODE:        return "COUNT_MODE";
        default: return nullptr;
    }
}

const char* decode_global_field(uint16_t field_id) {
    switch (field_id) {
        case MARS_FIELD_ST:   return "ST";
        case MARS_FIELD_GAIN: return "GAIN";
        case MARS_FIELD_POL:  return "POL";
        case MARS_FIELD_EBLK: return "EBLK";
        case MARS_FIELD_GMON: return "GMON";
        case MARS_FIELD_PUEN: return "PUEN";
        case MARS_FIELD_MFS:  return "MFS";
        case MARS_FIELD_TDS:  return "TDS";
        case MARS_FIELD_TDM:  return "TDM";
        case MARS_FIELD_TH:   return "TH";
        case MARS_FIELD_C:    return "C";
        case MARS_FIELD_M0:   return "M0";
        case MARS_FIELD_SAUX: return "SAUX";
        default: return "?";
    }
}

const char* decode_channel_field(uint16_t field_id) {
    switch (field_id) {
        case MARS_CH_CHEN: return "CHEN";
        case MARS_CH_TSEN: return "TSEN";
        case MARS_CH_THTR: return "THTR";
        case MARS_CH_PUTR: return "PUTR";
        default: return "?";
    }
}

// Main formatter for RX messages
std::string format_rx_msg(const DeviceMsg& msg) {
    std::ostringstream oss;
    oss << decode_cmd(msg.cmd) << ": ";
    switch (msg.cmd) {
        case DeviceCmd::REG_READ:
        case DeviceCmd::REG_WRITE: {
            const char* regname = decode_reg(msg.addr);
            oss << (regname ? regname : "reg?") << " [0x" << std::hex << msg.addr << "] value=0x" << std::hex << msg.value;
            break;
        }
        case DeviceCmd::SET_GLOBAL: {
            uint16_t chip_mask = (msg.addr >> 16) & 0x0FFF;
            uint16_t field_id  = msg.addr & 0xFFFF;
            oss << "chip_mask=0x" << std::hex << chip_mask << " field=" << decode_global_field(field_id) << " [" << field_id << "] value=0x" << std::hex << msg.value;
            break;
        }
        case DeviceCmd::SET_CHANNEL: {
            uint16_t channel  = (msg.addr >> 16) & 0x0FFF;
            uint16_t field_id = msg.addr & 0xFFFF;
            oss << "channel=" << std::dec << channel << " field=" << decode_channel_field(field_id) << " [" << field_id << "] value=0x" << std::hex << msg.value;
            break;
        }
        case DeviceCmd::MARS_LOAD: {
            uint16_t chip_mask = msg.addr & 0x0FFF;
            oss << "chip_mask=0x" << std::hex << chip_mask;
            break;
        }
        case DeviceCmd::ADC_CLK_SKEW:
        case DeviceCmd::I2C_TEMP_READ:
        case DeviceCmd::XADC_READ:
        case DeviceCmd::I2C_DAC_WRITE:
        case DeviceCmd::I2C_ADC_READ:
        case DeviceCmd::I2C_DAC_INIT: {
            oss << "addr=0x" << std::hex << msg.addr << " value=0x" << std::hex << msg.value;
            break;
        }
        case DeviceCmd::SET_LOG_LEVEL: {
            oss << "level=" << msg.value;
            break;
        }
        default:
            oss << "addr=0x" << std::hex << msg.addr << " value=0x" << std::hex << msg.value;
    }
    return oss.str();
}

} // namespace

        ///< Non-blocking dispatch to per-bus workers
        dispatcher(msg);
    }
}

//===========================================================================//

void GermaniumZMQ::tx_loop()
{
    logger_.log_debug("GermaniumZMQ: tx_thread running");

    while (running_) {
        DeviceMsg msg = tx_queue_.pop();
        if (!running_) break;

        WireMsg wire;
        wire.cmd   = msg.cmd;
        wire.addr  = msg.addr;
        wire.value = msg.value;

#ifdef SIM_MODE
        logger_.log_debug("ZMQ TX: cmd=0x%02X addr=0x%04X value=0x%08X",
                          wire.cmd, wire.addr, wire.value);
#endif

        zmq_send(zmq_tx_, &wire, sizeof(wire), 0);
    }
}

//===========================================================================//

void GermaniumZMQ::tx_reply_special(const DeviceMsg& msg)
{
    tx_queue_.push(msg);
}

//===========================================================================//

void GermaniumZMQ::stop_special()
{
    running_ = false;

    ///< Unblock tx_thread (flush tx_queue)
    tx_queue_.stop();

    ///< Closing context will unblock zmq_recv with ETERM
    if (zmq_ctx_) {
        zmq_ctx_shutdown(zmq_ctx_);
    }
}

//===========================================================================//
