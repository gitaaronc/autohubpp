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
#include "include/insteon/detail/InsteonController_impl.h"

#include "include/insteon/InsteonController.h"
#include "include/insteon/InsteonNetwork.hpp"
#include "include/insteon/MessageProcessor.hpp"
#include "include/insteon/InsteonControllerGroupCommands.h"
#include "include/insteon/InsteonMessage.hpp"

#include "include/Logger.h"
#include "include/system/Timer.hpp"
#include "include/utils/utils.hpp"

#include <memory>

#include <iostream>
#include <iomanip>

namespace ace
{
namespace insteon
{

// TODO Move to utils

std::string
ByteArrayToStringStream(
        const std::vector<unsigned char>& data, int offset, int count) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::stringstream strStream;
    for (int i = offset; i < offset + count; ++i) {
        if (i < data.size()) {
            strStream << std::hex << std::setw(2) << std::setfill('0')
                    << (unsigned int) data[i];
        }
    }
    return strStream.str();
}

InsteonController::InsteonController(InsteonNetwork *network,
        boost::asio::io_service& io_service)
: insteon_network_(network), is_loading_database_(false),
pImpl_(new detail::InsteonController_impl) {

    pImpl_->timer_ = std::move(
            std::unique_ptr<system::Timer>(new system::Timer(io_service)));
    pImpl_->timer_->SetTimerCallback(
            std::bind(&InsteonController::onTimerEvent, this));
    pImpl_->timer_->RunAsync();
    pImpl_->timer_->Stop();
}

InsteonController::~InsteonController() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

void
InsteonController::setAddress(int address) {
    pImpl_->insteon_address_.address_high_ = address >> 16 & 0xFF;
    pImpl_->insteon_address_.address_middle_ = address >> 8 & 0xFF;
    pImpl_->insteon_address_.address_low_ = address & 0xFF;
}

int
InsteonController::getAddress() {
    return pImpl_->insteon_address_.address_high_ << 16 |
            pImpl_->insteon_address_.address_middle_ << 8 |
            pImpl_->insteon_address_.address_low_;
}

void
InsteonController::getDatabaseRecords(unsigned char one, unsigned char two) {
    is_loading_database_ = true;
    if ((one == 0x1C) && (two == 0x00)) {
        is_loading_database_ = false;
        insteon_network_->cv_load_db_.notify_one();
        return;
    }
    std::vector<unsigned char> send_buffer = {0x75};
    send_buffer.push_back(one);
    send_buffer.push_back(two);

    //std::vector<unsigned char> send_buffer = {0x69};
    //insteon_network_->io_service_.post(std::bind(
    //        &type::InternalSend, this, send_buffer));
    insteon_network_->msg_proc_->trySend(send_buffer, false);
}

void
InsteonController::getIMConfiguration() {
    std::vector<unsigned char> send_buffer = {0x73};
    insteon_network_->msg_proc_->trySend(send_buffer);
}

bool InsteonController::enableMonitorMode() {
    std::vector<unsigned char> send_buffer = {0x6B, 0x20};
    if (insteon_network_->msg_proc_->trySend(send_buffer) == EchoStatus::ACK)
        return true;
    return false;
}

void
InsteonController::enterLinkMode(InsteonLinkMode mode, unsigned char group) {
    if (!tryEnterLinkMode(mode, group)) {
        return; // TODO add exception handling
    }
}

void
InsteonController::internalSend(const std::vector<unsigned char>& buffer) {
    insteon_network_->msg_proc_->trySend(buffer);
}

bool
InsteonController::tryEnterLinkMode(InsteonLinkMode mode, unsigned char group) {
    pImpl_->LinkingMode_ = mode;
    std::vector<unsigned char> send_buffer = {0x64, (unsigned char) mode, group};
    if (insteon_network_->msg_proc_->trySend(send_buffer) != insteon::EchoStatus
            ::ACK) {
        return false;
    }
    pImpl_->timer_->Reset(1000 * 60 * 4);
    pImpl_->IsInLinkingMode_ = true;
    return true;
}

void
InsteonController::cancelLinkMode() {
    if (!tryCancelLinkMode()) {
        return; // TODO add exception handling
    }
}

bool
InsteonController::tryCancelLinkMode() {
    pImpl_->timer_->Stop();
    pImpl_->IsInLinkingMode_ = false;
    pImpl_->LinkingMode_ = InsteonLinkMode::Contoller;
    std::vector<unsigned char> send_buffer = {0x65};
    return insteon_network_->msg_proc_->trySend(send_buffer) ==
            insteon::EchoStatus::ACK;
}

void
InsteonController::groupCommand(InsteonControllerGroupCommands command,
        unsigned char group) {
    unsigned char value = 0;
    if (command == InsteonControllerGroupCommands::StopDimming)
        return; // Add exception handling
    if (command == InsteonControllerGroupCommands::On)
        value = 0xFF;
    groupCommand(command, group, value);
}

void
InsteonController::groupCommand(InsteonControllerGroupCommands command,
        unsigned char group, unsigned char value) {
    unsigned char cmd = (unsigned char) command;
    std::vector<unsigned char> send_buffer = {0x61, group, cmd, value};
    insteon_network_->msg_proc_->trySend(send_buffer);
}

bool
InsteonController::tryGroupCommand(InsteonControllerGroupCommands command,
        unsigned char group) {
    return false;
}

bool
InsteonController::tryGroupCommand(InsteonControllerGroupCommands command,
        unsigned char group, unsigned char value) {
    return false;
}

void
InsteonController::onTimerEvent() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    pImpl_->IsInLinkingMode_ = false;
    pImpl_->timer_->Stop();
}

void
InsteonController::onDeviceLinked(std::shared_ptr<
        InsteonDevice>& device) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

void
InsteonController::onDeviceUnlinked(std::shared_ptr<
        InsteonDevice>& device) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

void
InsteonController::onMessage(
        std::shared_ptr<insteon::InsteonMessage> im) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    if (im->properties_.size() > 0) {
        std::ostringstream oss;
        oss << "The following message was received by this PLM\n";
        /*oss << "\t  - " << device_name() << " {0x" << utils::int_to_hex(
                this->insteon_address()) << "}\n";*/
        oss << "\t  - {0x" << utils::ByteArrayToStringStream(
                im->raw_message, 0, im->raw_message.size()) << "}\n";
        for (const auto& it : im->properties_) {
            oss << "\t  " << it.first << ": "
                    << utils::int_to_hex(it.second) << "\n";
        }
        utils::Logger::Instance().Debug(oss.str().c_str());
    }
    int insteon_address = 0;
    switch (im->message_type_) {
        case insteon::InsteonMessageType::DeviceLink:
        {
            insteon_address = im->properties_.find("address")->second;

            std::shared_ptr<InsteonDevice> device;
            device = insteon_network_->addDevice(insteon_address);

            pImpl_->timer_->Stop();
            pImpl_->IsInLinkingMode_ = false;

            if (pImpl_->LinkingMode_ != InsteonLinkMode::Delete)
                onDeviceLinked(device);
            else
                onDeviceUnlinked(device);
        }
            break;
        case insteon::InsteonMessageType::GetIMInfo:
            pImpl_->insteon_identity_.category =
                    im->properties_["device_category"];

            pImpl_->insteon_identity_.sub_category =
                    im->properties_["device_subcategory"];

            pImpl_->insteon_identity_.firmware_version =
                    im->properties_["device_firmware_version"];

            break;
        case insteon::InsteonMessageType::GetIMConfiguration:
            utils::Logger::Instance().Info("IM Configuration flags, "
                    "do something with them");
            break;
        case insteon::InsteonMessageType::DeviceLinkRecord:
        case insteon::InsteonMessageType::ALDBRecord:
            utils::Logger::Instance().Info("ALDB record received");
            //processDatabaseRecord(im);
            break;
        default:
            utils::Logger::Instance().Info("%s\n\t - unexpected message: {%s}\n",
                    FUNCTION_NAME_CSTR,
                    ByteArrayToStringStream(im->raw_message,
                    0, im->raw_message.size()).c_str()
                    );
            break;

    }
}

void
InsteonController::processDatabaseRecord(
        std::shared_ptr<insteon::InsteonMessage> im) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    int address = 0;
    address = im->properties_["link_address"];
    if (address > 0)
        insteon_network_->addDevice(address);
    bool get_next = false;
    int has_flags = im->properties_["link_record_flags"];
    get_next = has_flags > 0;
    if (get_next) {
        unsigned int temp = 0;
        unsigned char one = im->properties_["db_address_MSB"];
        unsigned char two = im->properties_["db_address_LSB"];

        utils::Logger::Instance().Debug("Database record found.\n"
                "\t  Memory location MSB: %d\n"
                "\t  Memory location LSB: %d\n"
                "\t  Link Record Flags: %i\n"
                "\t  Link Group: %d\n"
                "\t  Device Address: %s",
                im->properties_["db_address_MSB"],
                im->properties_["db_address_LSB"],
                im->properties_["link_record_flags"],
                im->properties_["link_group"],
                utils::int_to_hex(im->properties_["link_address"]).c_str());

        temp = (one << 8) | (two);
        temp -= 8;
        one = (temp >> 8) & 0xFF;
        two = temp & 0xFF;
        getDatabaseRecords(one, two);
    } else {
        is_loading_database_ = false;
        insteon_network_->cv_load_db_.notify_one();
    }
}
} // namespace insteon
} // namespace ace
