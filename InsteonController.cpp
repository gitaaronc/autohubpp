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

#include <memory>

namespace ace {
namespace insteon {

InsteonController::InsteonController(InsteonNetwork *network, 
        boost::asio::io_service& io_service)
: insteon_network_(network),
pImpl_(new detail::InsteonController_impl) {

    pImpl_->timer_ = std::move(
            std::unique_ptr<system::Timer>(new system::Timer(io_service)));
    pImpl_->timer_->SetTimerCallback(
            std::bind(&InsteonController::OnTimerEvent, this));
    pImpl_->timer_->RunAsync();
    pImpl_->timer_->Stop();
}

InsteonController::~InsteonController() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

void
InsteonController::SetAddress(int address) {
    pImpl_->insteon_address_.address_high_ = address >> 16 & 0xFF;
    pImpl_->insteon_address_.address_middle_ = address >> 8 & 0xFF;
    pImpl_->insteon_address_.address_low_ = address & 0xFF;
}

int
InsteonController::GetAddress() {
    return pImpl_->insteon_address_.address_high_ << 16 |
            pImpl_->insteon_address_.address_middle_ << 8 |
            pImpl_->insteon_address_.address_low_;
}

void
InsteonController::GetIMConfiguration() {
    std::vector<unsigned char> send_buffer = {0x73};
    insteon_network_->msg_proc_->TrySend(send_buffer);
}

void
InsteonController::EnterLinkMode(InsteonLinkMode mode, unsigned char group) {
    if (!TryEnterLinkMode(mode, group)) {
        return; // TODO add exception handling
    }
}

bool
InsteonController::TryEnterLinkMode(InsteonLinkMode mode, unsigned char group) {
    pImpl_->LinkingMode_ = mode;
    std::vector<unsigned char> send_buffer = {0x64, (unsigned char) mode, group};
    if (insteon_network_->msg_proc_->TrySend(send_buffer) != insteon::EchoStatus
            ::ACK) {
        return false;
    }
    pImpl_->timer_->Reset(1000 * 60 * 4);
    pImpl_->IsInLinkingMode_ = true;
    return true;
}

void
InsteonController::CancelLinkMode() {
    if (!TryCancelLinkMode()) {
        return; // TODO add exception handling
    }
}

bool
InsteonController::TryCancelLinkMode() {
    pImpl_->timer_->Stop();
    pImpl_->IsInLinkingMode_ = false;
    pImpl_->LinkingMode_ = InsteonLinkMode::Contoller;
    std::vector<unsigned char> send_buffer = {0x65};
    return insteon_network_->msg_proc_->TrySend(send_buffer) ==
            insteon::EchoStatus::ACK;
}

void
InsteonController::GroupCommand(InsteonControllerGroupCommands command,
        unsigned char group) {
    unsigned char value = 0;
    if (command == InsteonControllerGroupCommands::StopDimming)
        return; // Add exception handling
    if (command == InsteonControllerGroupCommands::On)
        value = 0xFF;
    GroupCommand(command, group, value);
}

void
InsteonController::GroupCommand(InsteonControllerGroupCommands command,
        unsigned char group, unsigned char value) {
    unsigned char cmd = (unsigned char) command;
    std::vector<unsigned char> send_buffer = {0x61, group, cmd, value};
    insteon_network_->msg_proc_->TrySend(send_buffer);
}

bool
InsteonController::TryGroupCommand(InsteonControllerGroupCommands command,
        unsigned char group) {
    return false;
}

bool
InsteonController::TryGroupCommand(InsteonControllerGroupCommands command,
        unsigned char group, unsigned char value) {
    return false;
}

void
InsteonController::OnTimerEvent() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    pImpl_->IsInLinkingMode_ = false;
    pImpl_->timer_->Stop();
}

void
InsteonController::OnDeviceLinked(std::shared_ptr<
        InsteonDevice>& device) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

void
InsteonController::OnDeviceUnlinked(std::shared_ptr<
        InsteonDevice>& device) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

void
InsteonController::OnMessage(std::shared_ptr<insteon::InsteonMessage> insteon_message) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    int insteon_address = 0;
    if (insteon_message->message_type_ == insteon::InsteonMessageType::DeviceLink) {
        utils::Logger::Instance().Info("DEVICE LINK");

        insteon_address = insteon_message->properties_.find(insteon::PropertyKey
                ::Address)->second;

        std::shared_ptr<InsteonDevice> device;
        if (!insteon_network_->DeviceExists(insteon_address)) {
            device = insteon_network_->AddDevice(insteon_address);
        } else {
            device = insteon_network_->GetDevice(insteon_address);
        }

        pImpl_->timer_->Stop();
        pImpl_->IsInLinkingMode_ = false;

        if (pImpl_->LinkingMode_ != InsteonLinkMode::Delete)
            OnDeviceLinked(device);
        else
            OnDeviceUnlinked(device);
    } else if (insteon_message->message_type_ ==
            insteon::InsteonMessageType::GetIMInfo) {
        pImpl_->insteon_identity_.category =
                insteon_message->properties_[insteon::PropertyKey::DevCat];

        pImpl_->insteon_identity_.sub_category =
                insteon_message->properties_[insteon::PropertyKey::DevSubCat];

        pImpl_->insteon_identity_.firmware_version =
                insteon_message->properties_[insteon::PropertyKey
                ::DevFirmwareVersion];

    } else if (insteon_message->message_type_ ==
            insteon::InsteonMessageType::GetIMConfiguration) {
        utils::Logger::Instance().Info("IM Configuration flags, "
                "do something with them");
    } else {
        utils::Logger::Instance().Warning("Unknown Message received");
    }
}
}
}