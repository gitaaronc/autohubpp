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
#include "include/insteon/InsteonNetwork.hpp"
#include "include/insteon/InsteonMessage.hpp"
#include "include/insteon/MessageProcessor.hpp"
#include "include/insteon/InsteonController.h"

#include "include/json/json.h"
#include "include/Logger.h"

#include <iostream>

#include <boost/format.hpp>

namespace ace {
namespace insteon {

InsteonNetwork::InsteonNetwork(boost::asio::io_service& io_service)
: io_service_(io_service), msg_proc_(new MessageProcessor(io_service)){
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    msg_proc_->set_message_handler(std::bind(&type::OnMessage, this,
            std::placeholders::_1));
    insteon_controller_ = std::move(std::unique_ptr<InsteonController>(
            new InsteonController(this, io_service)));
}

InsteonNetwork::~InsteonNetwork() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

/**
 * AddDevice
 * 
 * Adds an InsteonDevice to the list of InsteonDevices
 * 
 * @param insteon_address
 * @param insteon_identity
 * 
 * @return Returns a shared_ptr of InsteonDevice object
 */

std::shared_ptr<InsteonDevice>
InsteonNetwork::AddDevice(int insteon_address) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);

    std::shared_ptr<InsteonDevice>device =
            std::make_shared<InsteonDevice>(insteon_address, io_service_);
    device->set_message_proc(msg_proc_);
    device->set_update_handler(std::bind(&type::OnUpdateDevice,
            this, std::placeholders::_1));
    device_list_.insert(InsteonDeviceMapPair(insteon_address, device));
    return device;

}

bool
InsteonNetwork::Connect() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    msg_proc_->Connect();
}

/**
 * DeviceExists
 * 
 * Returns true if the InsteonDevice with a specific Address exists
 * in the list of InsteonDevices
 * 
 * @param insteon_address
 * @return bool True/False
 */
bool
InsteonNetwork::DeviceExists(int insteon_address) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    auto it = device_list_.find(insteon_address);
    return it != device_list_.end();
}

/**
 * GetDevice
 * 
 * Gets the InsteonDevice object from the INSTEON device list, if it exists
 * 
 * @param insteon_address
 * @return Returns a ptr to shared<InsteonDevice> object.
 */
std::shared_ptr<InsteonDevice>
InsteonNetwork::GetDevice(int insteon_address) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::shared_ptr<InsteonDevice>device;
    auto it = device_list_.find(insteon_address);
    if (it != device_list_.end())
        device = it->second;
    return device;
}

Json::Value
InsteonNetwork::SerializeJson(int device_id) {
    Json::Value root;
    if (device_id == 0) {
        Json::Value devices;
        for (const auto& it : device_list_) {
            devices.append(it.second->SerializeJson());
        }
        root["devices"] = devices;
        root["event"] = "deviceList";
    } else {
        root["device"] = GetDevice(device_id)->SerializeJson();
        root["event"] = "deviceUpdate";
    }
    return root;
}

void
InsteonNetwork::InternalReceiveCommand(std::string json) {
    Json::Reader reader;
    Json::Value root;
    std::string command;
    unsigned char command_two = 0x00;

    reader.parse(json, root);

    std::shared_ptr<InsteonDevice>device;
    std::string device_id;
    device_id = root.get("device_id", "").asString();

    command = root.get("command", "").asString();
    command_two = root.get("command_two", 0).asInt();
    device = GetDevice(std::stoi(device_id));
    if (device) {
        device->InternalReceiveCommand(command, command_two);
    }
}

void
InsteonNetwork::OnUpdateDevice(Json::Value json) {
    Json::Value root;
    root["command"] = "update_device";
    root["devices"] = json;
    if (OnUpdate)
        io_service_.post([ = ]{OnUpdate(json);});
}

void
InsteonNetwork::OnMessage(std::shared_ptr<InsteonMessage> iMsg) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    int insteon_address = 0;
    std::shared_ptr<InsteonDevice>device;
    // TODO verify broadcast message cleanup events & to/from process
    if (iMsg->properties_.count(PropertyKey::FromAddress)) {
        insteon_address = iMsg->properties_[PropertyKey::FromAddress];
        if (DeviceExists(insteon_address)) {
            device = GetDevice(insteon_address);

            io_service_.post(std::bind(&InsteonDevice::OnMessage, device, iMsg));

        } else if (iMsg->message_type_ == InsteonMessageType::SetButtonPressed) {
            insteon_controller_->OnMessage(iMsg);
        } else {
            device = AddDevice(insteon_address);
            device->OnMessage(iMsg);

            io_service_.post(std::bind(&InsteonDevice::Command, device,
                    InsteonDeviceCommand::GetOperatingFlags, 0x00));

            io_service_.post(std::bind(&InsteonDevice::Command, device,
                    InsteonDeviceCommand::GetInsteonEngineVersion, 0x00));

            io_service_.post(std::bind(&InsteonDevice::Command, device,
                    InsteonDeviceCommand::IDRequest, 0x00));

            io_service_.post(std::bind(&InsteonDevice::Command, device,
                    InsteonDeviceCommand::ExtendedGetSet, 0x00));

            io_service_.post(std::bind(&InsteonDevice::Command, device,
                    InsteonDeviceCommand::LightStatusRequest, 0x00));

        }
    } else {
        insteon_controller_->OnMessage(iMsg);
    }

}

void
InsteonNetwork::set_update_handler(
        std::function<void(Json::Value) > callback) {
    OnUpdate = callback;
}
} // namespace insteon
} // namespace ace