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

#ifndef INSTEONNETWORK_HPP
#define	INSTEONNETWORK_HPP

#include "InsteonDevice.hpp"
#include "../io/SerialPort.h"

#include <memory>
#include <condition_variable>

#include <boost/asio.hpp>

#include <yaml-cpp/yaml.h>

namespace Json{
class Value;
}

namespace ace {
namespace insteon {
class MessageProcessor;
class InsteonMessage;
class InsteonController;

class InsteonNetwork {
    typedef InsteonNetwork type;
public:
    InsteonNetwork() = delete;
    InsteonNetwork(boost::asio::io_service& io_service, YAML::Node config);
    ~InsteonNetwork();

    void
    Close() {
    };

    bool Connect();
    void LoadDevices();
    void SaveDevices();
    Json::Value SerializeJson(int device_id = 0);
    void InternalReceiveCommand(std::string json);
    void set_update_handler(
            std::function<void(Json::Value json) > callback);
protected:
    friend class InsteonController;
    // adds a device to insteon_device_list
    std::shared_ptr<InsteonDevice> AddDevice(int insteon_address);
    std::shared_ptr<InsteonDevice> GetDevice(int insteon_address);

    void
    Disconnect() {
    };

    bool
    DeviceExists(int insteon_address);

    void OnMessage(std::shared_ptr<InsteonMessage> im);
    void OnUpdateDevice(Json::Value json);

    std::mutex mx_load_db_;
    std::condition_variable cv_load_db_;
private:
    boost::asio::io_service& io_service_;
    boost::asio::io_service::strand io_strand_;
    // pointer to Insteon Controller object
    std::unique_ptr<InsteonController> insteon_controller_;
    // list of Insteon Devices
    InsteonDeviceMap device_list_;
    // pointer to message processor
    std::shared_ptr<MessageProcessor> msg_proc_;
    // pointer to callback function, executed when updates occur
    std::function<void(Json::Value) > OnUpdate;
    
    YAML::Node config_;
};
} // namespace insteon
} // namespace ace

#endif	/* INSTEONNETWORK_HPP */

