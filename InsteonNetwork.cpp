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
#include "include/utils/utils.hpp"

#include <iostream>

#include <mutex>
#include <condition_variable>

namespace ace
{
namespace insteon
{

InsteonNetwork::InsteonNetwork(boost::asio::io_service& io_service,
        YAML::Node config)
: io_service_(io_service), config_(config),
msg_proc_(new MessageProcessor(io_service, config)) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    msg_proc_->set_message_handler(std::bind(&type::OnMessage, this,
            std::placeholders::_1));
    insteon_controller_ = std::move(std::unique_ptr<InsteonController>(
            new InsteonController(this, io_service)));
    if (config_.IsNull() || !config_.IsDefined())
        throw; // TODO remove throw for something better
}

InsteonNetwork::~InsteonNetwork() {
    for (const auto& it : device_list_)
        it.second->SerializeYAML();
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

/**
 * AddDevice
 * 
 * Adds an InsteonDevice to the map of InsteonDevices
 * Updates if the device already exists
 * 
 * @param insteon_address 
 * 
 * @return Returns a shared_ptr of InsteonDevice object
 */

std::shared_ptr<InsteonDevice>
InsteonNetwork::AddDevice(int insteon_address) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);

    if (device_list_.count(insteon_address)){
        return device_list_[insteon_address];
    }
    
    std::shared_ptr<InsteonDevice> device = std::make_shared<InsteonDevice>
            (insteon_address, io_service_, config_
            ["DEVICES"][ace::utils::int_to_hex(insteon_address)]);

    device->set_message_proc(msg_proc_);
    device->set_update_handler(std::bind(&type::OnUpdateDevice,
            this, std::placeholders::_1));
    
    device_list_.insert(InsteonDeviceMapPair(insteon_address, device));
    
    return device;
    /*
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
     */
}

/**
 * LoadDevices
 * 
 * Loads devices from configuration object(YAML::Node)
 */
void
InsteonNetwork::LoadDevices() {
    YAML::Node device = config_["DEVICES"];
    for (auto it = device.begin(); it != device.end(); ++it) {
        AddDevice(it->first.as<int>(0));
    }
}

bool
InsteonNetwork::Connect() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    if (!msg_proc_->Connect())
        return false;

    LoadDevices();

    if (config_["PLM"]["enable_monitor_mode"].as<bool>(false))
        if (insteon_controller_->EnableMonitorMode())
            utils::Logger::Instance().Info("PLM monitor mode enabled "
                "successfully!");

    // start loading the ALDB from PLM
    if (config_["PLM"]["load_aldb"].as<bool>(true)) {
        utils::Logger::Instance().Info("%s\n\t  - getting aldb from PLM",
                FUNCTION_NAME_CSTR);
        insteon_controller_->GetDatabaseRecords(0x1F, 0xF8);
    }

    // wait here until the database is loaded
    std::unique_lock<std::mutex> lk(mx_load_db_);
    cv_load_db_.wait(lk, [this] {
        return insteon_controller_->is_loading_database_ == false;
    });

    // get aldb from each enabled device in the list
    if (config_["PLM"]["load_aldb"].as<bool>(true)) {
        utils::Logger::Instance().Info("%s\n\t  - getting aldb from known devices",
                FUNCTION_NAME_CSTR);
        for (const auto& it : device_list_) {
            if (config_["DEVICES"][utils::int_to_hex(it.second->insteon_address())]
                    ["device_disabled"].as<int>(0) == 0) {
                io_service_.post(std::bind(&InsteonDevice::Command, it.second,
                        InsteonDeviceCommand::ALDBReadWrite, 0x00));
            }
        }
    }

    // get status of each enabled device in the list
    if (config_["PLM"]["sync_device_status"].as<bool>(true)) {
        utils::Logger::Instance().Info("%s\n\t  - syncing device status",
                FUNCTION_NAME_CSTR);
        for (const auto& it : device_list_) {
            if (config_["DEVICES"][utils::int_to_hex(it.second->insteon_address())]
                    ["device_disabled"].as<int>(0) == 0) {
                io_service_.post(std::bind(&InsteonDevice::Command, it.second,
                        InsteonDeviceCommand::LightStatusRequest, 0x02));
            }
        }
    }

    return true;
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

/**
 * SerializeJson
 * 
 * Serialize Insteon Device list to json format
 * a device_id of zero will return all devices
 * 
 * @param device_id 
 * @return 
 */
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

/**
 * OnUpdateDevice
 * callback handler to receive device updates from InsteonDevice objects.
 * updates are then routed to out owner, autohub.
 * @param json
 */
void
InsteonNetwork::OnUpdateDevice(Json::Value json) {
    if (OnUpdate)
        io_service_.post([ = ]{OnUpdate(json);});
}

/**
 * OnMessage
 * Invoked by the Message Processor when a fully formed message is ready.
 * Responsible for routing fully formed messages to devices and controllers
 * @param iMsg
 */
void
InsteonNetwork::OnMessage(std::shared_ptr<InsteonMessage> im) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    int insteon_address = 0;
    std::shared_ptr<InsteonDevice>device;

    if (im->properties_.size() > 0) {
        std::ostringstream oss;
        oss << "The following message was received by the network\n";
        oss << "\t  - {0x" << utils::ByteArrayToStringStream(
                im->raw_message, 0, im->raw_message.size()) << "}\n";
        for (const auto& it : im->properties_) {
            oss << "\t  " << it.first << ": "
                    << utils::int_to_hex(it.second) << "\n";
        }
        utils::Logger::Instance().Debug(oss.str().c_str());
    }
    // automatically add devices found in other device databases
    // or devices found by linking.
    if (im->properties_.count("ext_link_address")) {
        insteon_address = im->properties_["ext_link_address"];
        if (!DeviceExists(insteon_address)) {
            AddDevice(insteon_address);
        }
    }

    // route messages to appropriate device or controller
    // automatically add new/discovered devices.
    if (im->properties_.count("from_address")) { // route to device
        insteon_address = im->properties_["from_address"];
        if (DeviceExists(insteon_address)) {
            device = GetDevice(insteon_address);
            io_service_.post(std::bind(&InsteonDevice::OnMessage, device, im));
        } else if (im->message_type_ == InsteonMessageType::SetButtonPressed) {
            insteon_controller_->OnMessage(im);
        } else {
            device = AddDevice(insteon_address);
            device->OnMessage(im);
        }
    } else { // route to controller/PLM
        insteon_controller_->OnMessage(im);
    }

}

/**
 * Sets the update handler. The update handler will be called by the network
 * when an update occurs. ie: switch turned on or off.
 * 
 * @param callback
 */
void
InsteonNetwork::set_update_handler(
        std::function<void(Json::Value) > callback) {
    OnUpdate = callback;
}
} // namespace insteon
} // namespace ace
