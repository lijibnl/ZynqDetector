#pragma once

#include <map>
#include <cstdint>
#include <variant>
#include <memory>

#include "concepts.hpp"
#include "queue.hpp"

template<typename T>
class I2CDevice
{
protected:
    T&                         i2c_;
    uint8_t                    i2c_addr_;
    
    std::unique_ptr<AccessReq> req_;
    QueueHandle_t              req_queue_;

    std::map<int, int> chan_assign_;  // stores <variable:channel>
                                      // defined by detector and passed to the constructor

    virtual void pack_req() = 0;

public:

    struct I2CInfo
    {
        std::shared_ptr<I2C> i2c;
        uint8_t              chan;
    };

    std::map<uint16_t, I2CInfo> chan_assign;

    I2CDevice( T&                  i2c
             , uint8_t             i2c_addr
             , bool                is_single_ended
             , std::map<int, char> chan_assign
             )
    requires IsSameType<T, PLI2C>;

    ~I2CDevice() = default;

    void read( uint8_t chan )
        requires IsSameType<T, PLI2C>;
};

