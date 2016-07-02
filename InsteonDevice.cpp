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

#include "include/json/json.h"
#include "include/json/json-forwards.h"

#include <iostream>

namespace ace {
namespace insteon {

InsteonDevice::InsteonDevice(int insteon_address,
        boost::asio::io_service& io_service) :
pImpl(new detail::InsteonDeviceImpl(this, insteon_address)),
io_service_(io_service),
last_action_(InsteonMessageType::Other){
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
}

InsteonDevice::~InsteonDevice() {

}

void
InsteonDevice::InternalReceiveCommand(std::string command,
        unsigned char command_two) {
    auto it = command_map_.find(command);
    if (it != command_map_.end()) {
        io_service_.post(std::bind(&type::Command, this, it->second, 
                command_two));
    } else {
        io_service_.post(std::bind(&type::Command, this,
                InsteonDeviceCommand::GetOperatingFlags, 0x00));

        io_service_.post(std::bind(&type::Command, this,
                InsteonDeviceCommand::GetInsteonEngineVersion, 0x00));

        io_service_.post(std::bind(&type::Command, this,
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

/**
 * AckOfDirectCommand
 * The function is called in Acknowledgment of a direct command.
 * Direct commands are initiated on the NetworkBus (Autohub)
 * @param cmd1 The original command sent on the bus
 * @param cmd2 The command_one field received in acknowledgment on the bus
 * @param cmd2_value The value returned in the command_two field
 */
void
InsteonDevice::AckOfDirectCommand(unsigned char sentCmdOne, 
        unsigned char recvCmdOne, unsigned char recvCmdTwo) {
    InsteonDeviceCommand command = static_cast<InsteonDeviceCommand> (sentCmdOne);
    switch (command) {
        case InsteonDeviceCommand::GetInsteonEngineVersion:
            device_properties_[PropertyKey::DevEngineVersion] = recvCmdTwo;
            break;
        case InsteonDeviceCommand::GetOperatingFlags:
            device_properties_[PropertyKey::EnableProgrammingLock]
                    = recvCmdTwo & 0x01;
            device_properties_[PropertyKey::EnableBlinkOnTraffic]
                    = recvCmdTwo & 0x02;
            device_properties_[PropertyKey::EnableResumeDim]
                    = recvCmdTwo & 0x04;
            device_properties_[PropertyKey::EnableLED]
                    = recvCmdTwo & 0x08;
            device_properties_[PropertyKey::EnableLoadSense]
                    = recvCmdTwo & 0x0A;
            break;
        case InsteonDeviceCommand::Off:
        case InsteonDeviceCommand::On:
        case InsteonDeviceCommand::FastOff:
        case InsteonDeviceCommand::FastOn:
            io_service_.post(std::bind(&type::StatusUpdate, this, recvCmdTwo));
            break;
        case InsteonDeviceCommand::LightStatusRequest:
            // Do we care about the database delta?
            // why do they include it in the lightStatusRequest response?
            device_properties_[PropertyKey::LinkDatabaseDelta] = recvCmdOne;
            io_service_.post(std::bind(&type::StatusUpdate, this, recvCmdTwo));
            break;
    }
}

void
InsteonDevice::OnMessage(std::shared_ptr<InsteonMessage> insteon_message) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    PropertyKeys keys = insteon_message->properties_;
    unsigned char command_one = keys[PropertyKey::Cmd1];
    unsigned char command_two = keys[PropertyKey::Cmd2];
    
    // TODO verify broadcast message cleanup events & to/from process
    bool actioned = ((unsigned char) insteon_message->message_type_
            == (unsigned char) last_action_);
    if (!actioned) { //used to prevent acting on duplicate or similar messages
        last_action_ = insteon_message->message_type_;
    }
    switch (insteon_message->message_type_) {
        case InsteonMessageType::Ack: // response to direct command
            pImpl->CommandAckProcessor(insteon_message);
            break;
        case InsteonMessageType::OnBroadcast:
            // device goes to set level at set ramp rate
            if (!actioned){
                unsigned char val = 0xFF;
                if (GetPropertyValue(PropertyKey::ButtonOnLevel, val)){
                    StatusUpdate(val);
                } else {
                    StatusUpdate(val);
                }
            }
            break;
        case InsteonMessageType::OnCleanup:
            break;
        case InsteonMessageType::OffBroadcast:
            // device turns off at set ramp rate
            if (!actioned)
                StatusUpdate(0x00);
            break;
        case InsteonMessageType::OffCleanup:
            break;
        case InsteonMessageType::FastOnBroadcast:
            // Device goes to max light level, no ramp
            if (!actioned)
                StatusUpdate(0xFF);
            break;
        case InsteonMessageType::FastOffBroadcast:
            // device turns off, no delay
            if (!actioned)
                StatusUpdate(0x00);
            break;
        case InsteonMessageType::FastOnCleanup:
            break;
        case InsteonMessageType::FastOffCleanup:
            break;
        case InsteonMessageType::IncrementBeginBroadcast:
            break;
        case InsteonMessageType::IncrementEndBroadcast:
            if (!actioned) {
                io_service_.post(std::bind(&type::Command, this,
                        InsteonDeviceCommand::LightStatusRequest, 0x00));
            }
            break;
        case InsteonMessageType::SetButtonPressed:
            device_properties_[PropertyKey::DevCat] =
                    keys[PropertyKey::DevCat];
            device_properties_[PropertyKey::DevSubCat] =
                    keys[PropertyKey::DevSubCat];
            device_properties_[PropertyKey::DevFirmwareVersion] =
                    keys[PropertyKey::DevFirmwareVersion];
            break;
    }
}

Json::Value
InsteonDevice::SerializeJson() {
    Json::Value root;
    Json::Value properties;
    root["device_address_"] = insteon_address();
    root["device_name_"] = device_name();
    
    for (const auto& it : device_properties_) {
        properties[PropertyKeyNames().GetPropertyName(it.first)] = it.second;
    }
    root["properties_"] = properties;
    return root;
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
InsteonDevice::Command(InsteonDeviceCommand command,
        unsigned char command_two) {
    //std::lock_guard<std::mutex>_(command_lock_);
    switch (command) {
        case InsteonDeviceCommand::On:
        {
            if (command_two > 0x00){
                return TryCommand(command, command_two);
            } 
            unsigned char val = 0x00;
            return TryCommand(command, GetPropertyValue(
                    PropertyKey::ButtonOnLevel, val) ? val : 0xFF);
        }
            break;
        case InsteonDeviceCommand::ExtendedGetSet:
            return TryGetExtendedInformation();
            break;
        default:
            return TryCommand(command, command_two);
            break;
    }
    return false;
}

void
InsteonDevice::GetExtendedMessage(std::vector<unsigned char>& send_buffer,
        unsigned char cmd1, unsigned char cmd2){
    pImpl->GetExtendedMessage(send_buffer, cmd1, cmd2);
}

bool
InsteonDevice::GetPropertyValue(const PropertyKey key, unsigned char& val){
    auto it = device_properties_.find(key);
    if (it != device_properties_.end()){
        val = it->second;
        return true;
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
InsteonDevice::TryCommand(InsteonDeviceCommand command, unsigned char value) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    pImpl->WaitAndSetPendingCommand((unsigned char) command, value);
    return pImpl->TryCommandInternal((unsigned char) command, value);
}

bool
InsteonDevice::TryGetExtendedInformation(){
    std::vector<unsigned char> send_buffer;
    GetExtendedMessage(send_buffer, 0x2E, 0x00);
    PropertyKeys properties;
    pImpl->WaitAndSetPendingCommand(0x2E, 0x00);
    EchoStatus status = pImpl->TrySendReceive(send_buffer, true, 0x51, properties);
    if ((status == EchoStatus::ACK) && (!properties.empty())){
        device_properties_[PropertyKey::X10HouseCode] 
                = properties[PropertyKey::Data5];
        device_properties_[PropertyKey::X10UnitCode] 
                = properties[PropertyKey::Data6];
        device_properties_[PropertyKey::ButtonRampRate] 
                = properties[PropertyKey::Data7] & 0x1F;
        device_properties_[PropertyKey::ButtonOnLevel] 
                = properties[PropertyKey::Data8];
        device_properties_[PropertyKey::SignalToNoiseThreshold] 
                = properties[PropertyKey::Data9];
        return true;
    } else {
        pImpl->ClearPendingCommand();
        return false;
    }
}

void
InsteonDevice::StatusUpdate(unsigned char status){
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    device_properties_[PropertyKey::LightStatus] = status;
    if (OnStatusUpdate)
        OnStatusUpdate(SerializeJson());
}

void
InsteonDevice::set_message_proc(
        std::shared_ptr<MessageProcessor> messenger) {
    pImpl->set_message_proc(messenger);
}

void
InsteonDevice::set_update_handler(std::function<
        void(Json::Value json) > callback) {
    OnStatusUpdate = callback;
}


}
}

