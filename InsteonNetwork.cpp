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
: io_service_(io_service), io_strand_(io_service), config_(config),
msg_proc_(new MessageProcessor(io_service, config["PLM"])) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    msg_proc_->set_message_handler(std::bind(&type::onMessage, this,
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
 * Adds an InsteonDevice to the map of InsteonDevices
 * Updates if the device already exists
 * 
 * @param insteon_address 
 * 
 * @return Returns a shared_ptr of InsteonDevice object
 */

std::shared_ptr<InsteonDevice>
InsteonNetwork::addDevice(uint32_t insteon_address) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);

    auto it = device_map_.find(insteon_address);
    if (it != device_map_.end())
        return it->second;

    std::shared_ptr<InsteonDevice> device = std::make_shared<InsteonDevice>
            (insteon_address, io_strand_, config_
            ["DEVICES"][ace::utils::int_to_hex(insteon_address)]);

    device->set_message_proc(msg_proc_);
    device->set_update_handler(std::bind(&type::onUpdateDevice,
            this, std::placeholders::_1));

    return device_map_.insert(InsteonDeviceMapPair(insteon_address, device))
            .first->second;

}

/**
 * LoadDevices
 * 
 * Loads devices from configuration object(YAML::Node)
 */
void
InsteonNetwork::loadDevices() {
    YAML::Node device = config_["DEVICES"];
    for (auto it = device.begin(); it != device.end(); ++it) {
        addDevice(it->first.as<int>(0));
    }
}

void
InsteonNetwork::saveDevices() {
    utils::Logger::Instance().Debug("%s\n\t  - %d devices total",
            FUNCTION_NAME_CSTR, device_map_.size());
    for (const auto& it : device_map_) {
        it.second->SerializeYAML();
    }
}

bool
InsteonNetwork::connect() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    
    PropertyKeys properties;
    if (!msg_proc_->connect(properties)){
        return false;
    }
    if (!properties.empty()){
        // TODO: HANDLE PLM Properties
    }

    loadDevices();

    // start loading the ALDB from PLM
    if (config_["PLM"]["load_aldb"].as<bool>(false)) {
        utils::Logger::Instance().Info("%s\n\t  - getting aldb from PLM",
                FUNCTION_NAME_CSTR);
        insteon_controller_->getDatabaseRecords(0x1F, 0xF8);
    }

    // wait here until the database is loaded
    std::unique_lock<std::mutex> lk(mx_load_db_);
    cv_load_db_.wait(lk, [this] {
        return insteon_controller_->is_loading_database_ == false;
    });

    // get aldb from each enabled device in the list
    if (config_["PLM"]["load_aldb"].as<bool>(false)) {
        utils::Logger::Instance().Info("%s\n\t  - getting aldb from known devices",
                FUNCTION_NAME_CSTR);
        for (const auto& it : device_map_) {
            if (!config_["DEVICES"][utils::int_to_hex(it.second->insteon_address())]
                    ["device_disabled_"].as<bool>(false)) {
                io_strand_.post(std::bind(&InsteonDevice::command, it.second,
                        InsteonDeviceCommand::ALDBReadWrite, 0x00));
            }
        }
    }

    // get status of each enabled device in the list
    if (config_["PLM"]["sync_device_status"].as<bool>(true)) {
        utils::Logger::Instance().Info("%s\n\t  - syncing device status",
                FUNCTION_NAME_CSTR);
        for (const auto& it : device_map_) {
            if (!config_["DEVICES"][utils::int_to_hex(it.second->insteon_address())]
                    ["device_disabled_"].as<bool>(false)) {
                io_strand_.post(std::bind(&InsteonDevice::command, it.second,
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
InsteonNetwork::deviceExists(uint32_t insteon_address) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    auto it = device_map_.find(insteon_address);
    return it != device_map_.end();
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
InsteonNetwork::getDevice(uint32_t insteon_address) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::shared_ptr<InsteonDevice>device;
    auto it = device_map_.find(insteon_address);
    if (it != device_map_.end())
        return it->second;
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
InsteonNetwork::serializeJson(uint32_t device_id) {
    Json::Value root;
    if (device_id == 0) {
        Json::Value devices;
        for (const auto& it : device_map_) {
            devices.append(it.second->SerializeJson());
        }
        root["devices"] = devices;
        root["event"] = "deviceList";
    } else {
        root["device"] = getDevice(device_id)->SerializeJson();
        root["event"] = "deviceUpdate";
    }
    return root;
}

void
InsteonNetwork::internalReceiveCommand(std::string json) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    Json::Reader reader;
    Json::Value root;
    std::string command;
    uint8_t command_two = 0x00;

    reader.parse(json, root);

    std::shared_ptr<InsteonDevice>device;
    std::string device_id;
    device_id = root.get("device_id", "").asString();

    command = root.get("command", "").asString();
    command_two = root.get("command_two", 0).asInt() <= 0 ? 0x00 :
            root.get("command_two", 0).asInt() >= 255 ? 0xFF :
            root.get("command_two", 0).asInt();
    
    device = getDevice(std::stoi(device_id));
    if (device) {
        device->internalReceiveCommand(command, command_two);
    } else {
        utils::Logger::Instance().Warning("Received command for device that"
                " doesn't exist.");
    }
}

/**
 * OnUpdateDevice
 * callback handler to receive device updates from InsteonDevice objects.
 * updates are then routed to out owner, autohub.
 * @param json
 */
void
InsteonNetwork::onUpdateDevice(Json::Value json) {
    if (on_update)
        io_service_.post([ = ]{on_update(json);});
}

/**
 * OnMessage
 * Invoked by the Message Processor when a fully formed message is ready.
 * Responsible for routing fully formed messages to devices and controllers
 * @param iMsg
 */
void
InsteonNetwork::onMessage(std::shared_ptr<InsteonMessage> im) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    uint32_t insteon_address = 0;

    if (houselinc_tx) {
        houselinc_tx(im->raw_message);
    }
    /*if (im->properties_.size() > 0) {
        std::ostringstream oss;
        oss << "The following message was received by the network\n";
        oss << "\t  - {0x" << utils::ByteArrayToStringStream(
                im->raw_message, 0, im->raw_message.size()) << "}\n";
        for (const auto& it : im->properties_) {
            oss << "\t  " << it.first << ": "
                    << utils::int_to_hex(it.second) << "\n";
        }
        utils::Logger::Instance().Debug(oss.str().c_str());
    }*/

    // automatically add devices found in other device databases
    // or devices found by linking.
    /*if (im->properties_.count("ext_link_address")) {
        insteon_address = im->properties_["ext_link_address"];
        if (!deviceExists(insteon_address)) {
            addDevice(insteon_address);
        }
    }*/

    // route messages to appropriate device or controller
    if (im->properties_.count("from_address")) { // route to device
        std::shared_ptr<InsteonDevice>device;
        insteon_address = im->properties_["from_address"];
        if (deviceExists(insteon_address)) {
            device = getDevice(insteon_address);
            io_strand_.post(std::bind(&InsteonDevice::OnMessage, device, im));
        } else if (im->message_type_ == InsteonMessageType::SetButtonPressed) {
            insteon_controller_->onMessage(im);
        } else {
            device = addDevice(insteon_address);
        }
    } else { // route to controller/PLM
        insteon_controller_->onMessage(im);
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
    on_update = callback;
}

void
InsteonNetwork::set_houselinc_tx(
        std::function<void(std::vector<uint8_t>) > callback) {
    houselinc_tx = callback;
}

void
InsteonNetwork::internalRawCommand(std::vector<uint8_t> buffer) {
    io_strand_.post([ = ](){
        msg_proc_->trySend(buffer, false);
    });
}

} // namespace insteon
} // namespace ace
