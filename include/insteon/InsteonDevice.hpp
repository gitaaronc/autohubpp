/*
 * Copyright (c) 2012, Aaron Coombs. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Autohub++ Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL AARON COOMBS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef INSTEONDEVICEBASE_H
#define INSTEONDEVICEBASE_H

#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <mutex>

#include "InsteonAddress.h"
#include "InsteonMessageType.hpp"
#include "InsteonDeviceCommands.hpp"
#include "PropertyKey.hpp"

#include <cstdint>

#include <boost/asio.hpp>

#include <yaml-cpp/yaml.h>

namespace Json
{
class Value;
}

namespace ace
{

namespace insteon
{
class InsteonMessage;
class MessageProcessor;
class InsteonAddress;

// Implements Insteon Device functions and stores attributes

class InsteonDevice {
    typedef InsteonDevice type;
public:
    InsteonDevice() = delete;
    InsteonDevice(uint32_t insteon_address,
                  boost::asio::io_service::strand& io_strand,
                  YAML::Node config);
    InsteonDevice(const InsteonDevice& rhs) = delete;
    InsteonDevice(InsteonDevice&& rhs) noexcept = delete;
    InsteonDevice& operator=(const InsteonDevice& rhs) = delete;
    InsteonDevice& operator=(InsteonDevice&& rhs) noexcept = delete;

    virtual ~InsteonDevice();
    virtual void OnMessage(std::shared_ptr<InsteonMessage> im);
    Json::Value SerializeJson();
    void SerializeYAML();

    void set_message_proc(std::shared_ptr<MessageProcessor> messenger);
    void set_update_handler(
                            std::function<void(Json::Value json) > callback);
    /* member variables, setters and getters */
    uint32_t insteon_address(); // returns insteon address assigned to this device
    std::string device_name(); // returns the name assigned to this device
    bool device_disabled();

    bool command(InsteonDeviceCommand command, uint8_t command_two);
    void internalReceiveCommand(std::string command, uint8_t command_two);
    void writeDeviceProperty(const std::string key, const uint32_t value);
    uint8_t readDeviceProperty(const std::string key);

protected:
    bool tryCommand(uint8_t command, uint8_t value);
    bool tryGetExtendedInformation();
    bool tryReadWriteALDB();
    bool tryLightStatusRequest();
    void statusUpdate(uint8_t status);
    //boost::asio::io_service& io_service_;
    boost::asio::io_service::strand io_strand_;

    std::map<std::string, InsteonDeviceCommand> command_map_;

private:
    std::shared_ptr<MessageProcessor> msgProc_;

    std::function<void(Json::Value) > onStatusUpdate;

    void ackOfDirectCommand(const std::shared_ptr<InsteonMessage>& im);
    void BuildDirectStandardMessage(std::vector<uint8_t>& send_buffer,
                                    uint8_t cmd1, uint8_t cmd2);
    void BuildDirectExtendedMessage(std::vector<uint8_t>& send_buffer,
                                    uint8_t cmd1, uint8_t cmd2,
                                    uint8_t d1 = 0, uint8_t d2 = 0, uint8_t d3 = 0,
                                    uint8_t d4 = 0, uint8_t d5 = 0, uint8_t d6 = 0,
                                    uint8_t d7 = 0, uint8_t d8 = 0, uint8_t d9 = 0,
                                    uint8_t d10 = 0, uint8_t d11 = 0, uint8_t d12 = 0,
                                    uint8_t d13 = 0);

    void device_name(std::string device_name);
    void device_disabled(bool disabled);
    std::string device_name_;
    bool device_disabled_;
    
    InsteonAddress insteon_address_;
    PropertyKeys device_properties_; // properties of this device
    std::mutex property_lock_; // mutex lock for access to device_properties

    void loadProperties(); // loads properties of this devices from config
    YAML::Node config_; // YAML node used to store configuration of this device

    uint8_t direct_cmd_; // the last command sent by/to this device
};

typedef std::map<int, std::shared_ptr<InsteonDevice >> InsteonDeviceMap;
typedef std::pair<int, std::shared_ptr<InsteonDevice >> InsteonDeviceMapPair;
} // namespace insteon
} // namespace ace
#endif /* INSTEONDEVICEBASE_H */

