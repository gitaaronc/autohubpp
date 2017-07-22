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
#include "include/insteon/InsteonDevice.hpp"
#include "include/insteon/InsteonMessage.hpp"
#include "include/insteon/detail/InsteonDeviceImpl.hpp"
#include "include/Logger.h"
#include "include/utils/utils.hpp"

#include "include/json/json.h"
#include "include/json/json-forwards.h"

#include <iostream>

namespace ace
{
namespace insteon
{

InsteonDevice::InsteonDevice(int insteon_address,
        boost::asio::io_service::strand& io_strand, YAML::Node config) :
pImpl(new detail::InsteonDeviceImpl(this, insteon_address)),
io_strand_(io_strand), config_(config),
last_action_(InsteonMessageType::Other) {

    device_properties_["light_status"] = 0;
    command_map_["ping"] = InsteonDeviceCommand::Ping;
    command_map_["request_id"] = InsteonDeviceCommand::IDRequest;
    command_map_["on"] = InsteonDeviceCommand::On;
    command_map_["fast_on"] = InsteonDeviceCommand::FastOn;
    command_map_["off"] = InsteonDeviceCommand::Off;
    command_map_["fast_off"] = InsteonDeviceCommand::FastOff;
    command_map_["brighten"] = InsteonDeviceCommand::Brighten;
    command_map_["dim"] = InsteonDeviceCommand::Dim;
    command_map_["dimming_start"] = InsteonDeviceCommand::StartDimming;
    command_map_["dimming_stop"] = InsteonDeviceCommand::StopDimming;
    command_map_["status"] = InsteonDeviceCommand::LightStatusRequest;
    command_map_["beep"] = InsteonDeviceCommand::Beep;
    loadProperties();
}

InsteonDevice::~InsteonDevice() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

void
InsteonDevice::internalReceiveCommand(std::string command,
        unsigned char command_two) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    auto it = command_map_.find(command);
    if (it != command_map_.end()) {
        //Command(it->second, command_two);
        io_strand_.post(std::bind(&type::command, this, it->second,
                command_two));
    } else {
        /*Command(InsteonDeviceCommand::GetOperatingFlags, 0x00);
        Command(InsteonDeviceCommand::GetInsteonEngineVersion, 0x00);
        Command(InsteonDeviceCommand::IDRequest, 0x00);*/

        io_strand_.post(std::bind(&type::command, this,
                InsteonDeviceCommand::GetOperatingFlags, 0x00));
        io_strand_.post(std::bind(&type::command, this,
                InsteonDeviceCommand::GetInsteonEngineVersion, 0x00));
        io_strand_.post(std::bind(&type::command, this,
                InsteonDeviceCommand::IDRequest, 0x00));

    }
}

int
InsteonDevice::insteon_address() {
    return pImpl->insteon_address_;
}

void
InsteonDevice::device_name(std::string device_name) {
    pImpl->device_name_ = device_name;
}

std::string
InsteonDevice::device_name() {
    return pImpl->device_name_;
}

bool
InsteonDevice::device_disabled() {
    return pImpl->device_disabled_;
}

void
InsteonDevice::device_disabled(bool disabled) {
    pImpl->device_disabled_ = disabled;
}

/**
 * AckOfDirectCommand
 * The function is called in Acknowledgment of a direct command.
 * Direct commands are initiated on the NetworkBus (Autohub)
 * @param cmd1 The original command sent on the bus
 * @param cmd2 The command_one field received in acknowledgment on the bus
 * @param cmd2_value The value returned in the command_two field
 */
void
InsteonDevice::ackOfDirectCommand(unsigned char sentCmdOne,
        unsigned char recvCmdOne, unsigned char recvCmdTwo) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    if (!sentCmdOne) {
        utils::Logger::Instance().Debug("%s\n\t  - {%s}\n"
                "\t  - unexpected ACK received {0x%02x ,0x%02x, 0x%02x}",
                FUNCTION_NAME_CSTR, device_name().c_str(), sentCmdOne, recvCmdOne, recvCmdTwo);
    } else {
        utils::Logger::Instance().Debug("%s\n\t  - {%s}\n"
                "\t  - ACK received for command{0x%02x, 0x%02x,0x%02x}",
                FUNCTION_NAME_CSTR, device_name().c_str(), sentCmdOne, recvCmdOne, recvCmdTwo);
    }
    InsteonDeviceCommand command = static_cast<InsteonDeviceCommand> (sentCmdOne);
    switch (command) {
        case InsteonDeviceCommand::GetInsteonEngineVersion:
            writeDeviceProperty("device_engine_version", recvCmdTwo);
            break;
        case InsteonDeviceCommand::GetOperatingFlags:
            writeDeviceProperty("enable_programming_lock", recvCmdTwo & 0x01);
            writeDeviceProperty("enable_blink_on_traffic", recvCmdTwo & 0x02);
            writeDeviceProperty("enable_resume_dim", recvCmdTwo & 0x04);
            writeDeviceProperty("enable_led", recvCmdTwo & 0x08);
            writeDeviceProperty("enable_load_sense", recvCmdTwo & 0x0a);
            break;
        case InsteonDeviceCommand::Off:
        case InsteonDeviceCommand::FastOff:
            io_strand_.get_io_service().post(std::bind(&type::statusUpdate, this, 0x00));
            break;
        case InsteonDeviceCommand::On:
        case InsteonDeviceCommand::FastOn:
            io_strand_.get_io_service().post(std::bind(&type::statusUpdate, this, recvCmdTwo));
            break;
        case InsteonDeviceCommand::LightStatusRequest:
            writeDeviceProperty("link_database_delta", recvCmdOne);
            io_strand_.get_io_service().post(std::bind(&type::statusUpdate, this, recvCmdTwo));
            break;
        default:
            io_strand_.get_io_service().post(std::bind(&type::statusUpdate, this, recvCmdTwo));
            break;
    }
}

/**
 * LoadDeviceProperties
 * Loads device object properties from yaml-cpp object
 */
void InsteonDevice::loadProperties() {
    std::lock_guard<std::mutex>lock(property_lock_);
    device_name(config_["device_name_"].as<std::string>(
            utils::int_to_hex(insteon_address())));
    device_disabled(config_["device_disabled_"].as<bool>(false));
    YAML::Node node = config_["properties_"];
    for (auto it = node.begin(); it != node.end(); ++it) {
        device_properties_[it->first.as<std::string>()]
                = it->second.as<unsigned int>();
    }
}

void
InsteonDevice::OnMessage(std::shared_ptr<InsteonMessage> im) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    PropertyKeys keys = im->properties_;
    unsigned char command_one = keys["command_one"];
    unsigned char command_two = keys["command_two"];
    unsigned char set_level = readDeviceProperty("button_on_level");
    unsigned char current_level = readDeviceProperty("light_status");
    device_disabled(false);

    if (im->properties_.size() > 0) {
        std::ostringstream oss;
        oss << "The following message was received by this device\n";
        oss << "\t  - " << device_name() << " {0x" << utils::int_to_hex(
                this->insteon_address()) << "}\n";
        oss << "\t  - {0x" << utils::ByteArrayToStringStream(
                im->raw_message, 0, im->raw_message.size()) << "}\n";
        for (const auto& it : im->properties_) {
            oss << "\t  " << it.first << ": "
                    << utils::int_to_hex(it.second) << "\n";
        }
        utils::Logger::Instance().Debug(oss.str().c_str());
    }

    // if group number > 0 return, we don't care yet
    if (im->properties_.count("group")) {
        if (im->properties_["group"]) {
            utils::Logger::Instance().Debug("%s\n\t  - Group command received, "
                    "returning.", FUNCTION_NAME_CSTR);
            return;
        }
    }

    if (!set_level) {
        io_strand_.post(std::bind(&type::command, this,
                InsteonDeviceCommand::LightStatusRequest, 0x02));
        io_strand_.post(std::bind(&type::command, this,
                InsteonDeviceCommand::ExtendedGetSet, 0x00));
    }

    switch (im->message_type_) {
        case InsteonMessageType::Ack: // response to direct command
            pImpl->CommandAckProcessor(im);
            break;
            // device goes to set level at set ramp rate
        case InsteonMessageType::OnBroadcast:
            set_level = current_level != set_level ? set_level : 0xFF;
            io_strand_.get_io_service().post(std::bind(&type::statusUpdate, this, set_level));
            break;
            // go to saved on level instantly
        case InsteonMessageType::FastOnBroadcast:
            io_strand_.get_io_service().post(std::bind(&type::statusUpdate, this, 0xFF));
            break;
            // goes to off level instantly
        case InsteonMessageType::FastOffBroadcast:
            // goes to off level at set ramp rate
        case InsteonMessageType::OffBroadcast:
            io_strand_.get_io_service().post(std::bind(&type::statusUpdate, this, 0x00));
            break;
        case InsteonMessageType::IncrementEndBroadcast:
        case InsteonMessageType::FastOffCleanup:
        case InsteonMessageType::FastOnCleanup:
        case InsteonMessageType::OffCleanup:
        case InsteonMessageType::OnCleanup:
            io_strand_.post(std::bind(&type::command, this,
                    InsteonDeviceCommand::LightStatusRequest, 0x00));
            break;
        case InsteonMessageType::IncrementBeginBroadcast:
            break;
        case InsteonMessageType::SetButtonPressed:
            writeDeviceProperty("device_category", keys["device_category"]);
            writeDeviceProperty("device_subcategory",
                    keys["device_subcategory"]);
            writeDeviceProperty("device_firmware_version",
                    keys["device_firmware_version"]);

            break;
        case InsteonMessageType::DeviceLinkRecord:
            utils::Logger::Instance().Debug("Link record received");
            break;
        case InsteonMessageType::DirectMessage:
            utils::Logger::Instance().Debug("Direct Message Received");
            break;
        default:
            utils::Logger::Instance().Debug("%s\n\t - unknown message type received\n"
                    "\t  - for device %s{%s}", FUNCTION_NAME_CSTR, device_name().c_str(),
                    utils::int_to_hex(insteon_address()).c_str());
            break;
    }
    SerializeYAML();
}

Json::Value
InsteonDevice::SerializeJson() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::lock_guard<std::mutex>lock(property_lock_);
    Json::Value root;
    Json::Value properties;
    root["device_address_"] = insteon_address();
    root["device_name_"] = device_name();
    root["device_disabled_"] = config_["device_disabled_"].as<bool>(false);
    for (const auto& it : device_properties_) {
        properties[it.first] = it.second;
    }
    root["properties_"] = properties;
    return root;
}

void InsteonDevice::SerializeYAML() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::lock_guard<std::mutex>lock(property_lock_);
    config_["device_address_"] = insteon_address();
    config_["device_name_"] = device_name();
    config_["device_disabled_"] = device_disabled();
    for (const auto& it : device_properties_) {
        config_["properties_"][it.first] = it.second;
    }
}

/**
 * Command
 * 
 * Invoked when only ISTEON command field #1 is provided
 * - command field #2 is set to default values
 * 
 * @param command INSTEON command field #1
 * @return Returns TRUE/FALSE
 */
bool
InsteonDevice::command(InsteonDeviceCommand command,
        unsigned char command_two) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    if (device_disabled()) {
        io_strand_.get_io_service().post(std::bind(&type::statusUpdate, this, command_two));
        return false; // device disabled, stop here and return
    }
    switch (command) {
        case InsteonDeviceCommand::ExtendedGetSet:
            return tryGetExtendedInformation();
            break;
        case InsteonDeviceCommand::ALDBReadWrite:
            return tryReadWriteALDB();
            break;
        case InsteonDeviceCommand::LightStatusRequest:
            return tryLightStatusRequest();
            break;
        default:
            return tryCommand(command, command_two);
            break;
    }
    return false;
}

/**
 * 
 * @param command Insteon Command #1 field
 * @param value   Insteon Command #2 field
 * @return Success/Fail
 */
bool
InsteonDevice::tryCommand(InsteonDeviceCommand command, unsigned char value) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    pImpl->WaitAndSetPendingCommand((unsigned char) command, value);
    return pImpl->TryCommandInternal((unsigned char) command, value);
}

bool
InsteonDevice::tryGetExtendedInformation() {
    std::vector<unsigned char> send_buffer;
    PropertyKeys properties;
    pImpl->GetExtendedMessage(send_buffer, 0x2E, 0x00);
    pImpl->WaitAndSetPendingCommand(0x2E, 0x00);
    EchoStatus status = pImpl->TrySendReceive(send_buffer, true, 0x51, properties);
    if ((status == EchoStatus::ACK) && (!properties.empty())) {
        writeDeviceProperty("x10_house_code", properties["data_five"]);
        writeDeviceProperty("x10_unit_code", properties["data_six"]);
        writeDeviceProperty("button_on_ramp_rate",
                properties["data_seven"] & 0x1F);
        writeDeviceProperty("button_on_level", properties["data_eight"]);
        writeDeviceProperty("signal_to_noise_threshold", properties["data_nine"]);
        return true;
    } else {
        writeDeviceProperty("button_on_level", 0xFF);
        pImpl->ClearPendingCommand();
        return false;
    }
}

bool
InsteonDevice::tryLightStatusRequest() {
    std::vector<unsigned char> send_buffer;
    PropertyKeys properties;
    pImpl->GetStandardMessage(send_buffer, 0x19, 0x02);
    pImpl->WaitAndSetPendingCommand(0x19, 0x02);
    EchoStatus status = pImpl->TrySendReceive(send_buffer, true, 0x50, properties);
    if ((status == EchoStatus::ACK) && (!properties.empty())) {
        writeDeviceProperty("link_database_delta", properties["command_one"]);
        writeDeviceProperty("light_status", properties["command_two"]);
        return true;
    } else {
        pImpl->ClearPendingCommand();
        return false;
    }
}

bool
InsteonDevice::tryReadWriteALDB() {
    std::vector<unsigned char> send_buffer;
    pImpl->GetExtendedMessage(send_buffer, 0x2F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    PropertyKeys properties;
    pImpl->WaitAndSetPendingCommand(0x2F, 0x00);
    EchoStatus status = pImpl->TrySendReceive(send_buffer, true, 0x50, properties);
    if ((status == EchoStatus::ACK) && (!properties.empty())) {
        return true;
    } else {
        pImpl->ClearPendingCommand();
        return false;
    }
}

void
InsteonDevice::statusUpdate(unsigned char status) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    writeDeviceProperty("light_status", status);
    if (onStatusUpdate)
        onStatusUpdate(SerializeJson());
}

void
InsteonDevice::set_message_proc(
        std::shared_ptr<MessageProcessor> messenger) {
    pImpl->set_message_proc(messenger);
}

void
InsteonDevice::set_update_handler(std::function<
        void(Json::Value json) > callback) {
    onStatusUpdate = callback;
}

void
InsteonDevice::writeDeviceProperty(const std::string key, const unsigned int value) {
    std::lock_guard<std::mutex>lock(property_lock_);
    device_properties_[key] = value;
}

unsigned char
InsteonDevice::readDeviceProperty(const std::string key) {
    std::lock_guard<std::mutex>lock(property_lock_);
    auto it = device_properties_.find(key);
    if (it != device_properties_.end()) {
        return it->second;
    }
    return 0;
}

}
}

