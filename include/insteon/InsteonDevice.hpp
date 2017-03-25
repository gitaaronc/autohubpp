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

#include "InsteonMessageType.hpp"
#include "InsteonDeviceCommands.hpp"
#include "PropertyKey.hpp"

#include <boost/asio.hpp>

#include <yaml-cpp/yaml.h>

namespace Json {
    class Value;
}

namespace ace {

    namespace insteon {
        class InsteonMessage;
        class MessageProcessor;

        namespace detail {
            class InsteonDeviceImpl;
        }

        // Implements Insteon Device functions and stores attributes
        class InsteonDevice {
            typedef InsteonDevice type;
        public:
            InsteonDevice() = delete;
            InsteonDevice(int insteon_address, boost::asio::io_service& io_service,
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
            int insteon_address(); // returns insteon address assigned to this device
            std::string device_name(); // returns the name assigned to this device
            bool device_disabled();

            bool Command(InsteonDeviceCommand command, unsigned char command_two);
            void InternalReceiveCommand(std::string command, unsigned char command_two);
            void writeDeviceProperty(const std::string key, const unsigned int value);
            unsigned char readDeviceProperty(const std::string key);
            
        protected:
            void GetExtendedMessage(std::vector<unsigned char>& send_buffer,
                    unsigned char cmd1, unsigned char cmd2);
            bool TryCommand(InsteonDeviceCommand command, unsigned char value);
            bool TryGetExtendedInformation();
            bool TryReadWriteALDB();
            void StatusUpdate(unsigned char status);
            boost::asio::io_service& io_service_;
            boost::asio::io_service::strand io_strand_;
            
            std::map<std::string, InsteonDeviceCommand> command_map_;

        private:
            friend class detail::InsteonDeviceImpl;
            std::unique_ptr<detail::InsteonDeviceImpl> pImpl;

            std::function<void(Json::Value) > OnStatusUpdate;

            InsteonMessageType last_action_;
            void AckOfDirectCommand(unsigned char sentCmdOne, unsigned char recvCmdOne,
                    unsigned char recvCmdTwo);

            void device_name(std::string device_name);
            void device_disabled(bool disabled);
            
            PropertyKeys device_properties_;
            std::mutex property_lock_;
            
            void LoadProperties();
            YAML::Node config_;
        };

        typedef std::map<int, std::shared_ptr<InsteonDevice >> InsteonDeviceMap;
        typedef std::pair<int, std::shared_ptr<InsteonDevice >> InsteonDeviceMapPair;
    } // namespace insteon
} // namespace ace
#endif /* INSTEONDEVICEBASE_H */

