#pragma once
#include <string>
#include <cstdint>
#include "GermaniumParamMap.hpp"
#include "../../common/DeviceMsg/DeviceMsg.hpp"

// Decodes command code to string
const char* decode_cmd(uint32_t cmd);
// Decodes register address to string
const char* decode_reg(uint32_t addr);
// Decodes global field id to string
const char* decode_global_field(uint16_t field_id);
// Decodes channel field id to string
const char* decode_channel_field(uint16_t field_id);
// Formats a DeviceMsg as a human-readable string
std::string format_rx_msg(const DeviceMsg& msg);
