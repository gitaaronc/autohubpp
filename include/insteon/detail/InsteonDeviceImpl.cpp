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

#include "InsteonDeviceImpl.hpp"
#include "../../Logger.h"

#include "../../utils/utils.hpp"

namespace ace
{
namespace insteon
{
namespace detail
{

InsteonDeviceImpl::InsteonDeviceImpl(InsteonDevice* pDevice,
        int insteon_address)
: ack_timer_(new system::Timer(pDevice->io_strand_.get_io_service())),
pending_command_(0), pending_command_two_(0), pending_retry_(0),
max_retries_(0), device_(pDevice), insteon_address_(insteon_address),
device_disabled_(false) {

    device_name_ = ace::utils::int_to_hex<int>(insteon_address);
    ack_timer_->SetTimerCallback(
            std::bind(&InsteonDeviceImpl::OnPendingCommandTimeout, this));
    ack_timer_->RunAsync();
    ack_timer_->Stop();
}

InsteonDeviceImpl::~InsteonDeviceImpl() {
}

/**
 * ClearPendingCommand
 * 
 * Clears the pending command and sets it to InsteonDeviceCommands::Null
 */
void
InsteonDeviceImpl::ClearPendingCommand() {
    std::lock_guard<std::mutex>_(command_mutex_);
    ack_timer_->Stop();
    pending_command_ = 0;
    pending_command_two_ = 0;
    ack_event_.Set();
}

/**
 * GetStandardMessage
 * 
 * Creates an Insteon Standard(0x62) Message
 * @param &send_buffer   Reference to a vector of unsigned char
 * @param cmd1:unsigned char    Insteon command field #1
 * @param cmd2:unsigned char    Insteon command field #2
 */
void
InsteonDeviceImpl::GetStandardMessage(
        std::vector<unsigned char>& send_buffer,
        unsigned char cmd1, unsigned char cmd2) {
    send_buffer.clear();
    unsigned char max_hops = 3;
    unsigned char message_flags = 0;
    max_hops = device_->readDeviceProperty("message_flags_max_hops") > 0
            ? device_->readDeviceProperty("message_flags_max_hops") : max_hops;
    message_flags = (max_hops << 2) | max_hops;
    send_buffer = {0x62, HighAddress(), MiddleAddress(), LowAddress(),
        message_flags, cmd1, cmd2};
}

/* 62 26 e4 c2 1f 2e 00 01 06 05 00 00 00 00 00 00 00 00 00 00 c6 */
void
InsteonDeviceImpl::GetExtendedMessage(
        std::vector<unsigned char>& send_buffer, unsigned char cmd1,
        unsigned char cmd2, unsigned char d1, unsigned char d2, unsigned char d3,
        unsigned char d4, unsigned char d5, unsigned char d6, unsigned char d7,
        unsigned char d8, unsigned char d9, unsigned char d10, unsigned char d11,
        unsigned char d12, unsigned char d13) {
    send_buffer.clear();
    unsigned char max_hops = 3;
    unsigned char message_flags = 0;
    max_hops = device_->readDeviceProperty("message_flags_max_hops") > 0
            ? device_->readDeviceProperty("message_flags_max_hops") : max_hops;
    message_flags = 16 | (max_hops << 2) | max_hops;
    send_buffer = {0x62, HighAddress(), MiddleAddress(), LowAddress(),
        message_flags, cmd1, cmd2, d1, d2, d3, d4, d5, d6, d7, d8,
        d9, d10, d11, d12, d13};

    send_buffer.insert(send_buffer.end(), utils::GetI2CS(send_buffer,
            5, send_buffer.size()));
}

/**
 * PendingCommandAck
 * 
 * When a command is pending, determines whether the current message completes the pending command
 * @param insteon_message
 */
void
InsteonDeviceImpl::CommandAckProcessor(
        const std::shared_ptr<InsteonMessage>& insteon_message) {

    unsigned char recvCmdOne = static_cast<unsigned char> (
            insteon_message->properties_["command_one"]);
    unsigned char recvCmdTwo = static_cast<unsigned char> (
            insteon_message->properties_["command_two"]);
    unsigned char sentCmdOne = 0x00;
    {
        std::lock_guard<std::mutex>_(command_mutex_);
        sentCmdOne = pending_command_;
    }
    max_retries_ = 3;
    device_->ackOfDirectCommand(sentCmdOne, recvCmdOne, recvCmdTwo);
    ClearPendingCommand();
}

bool
InsteonDeviceImpl::TryCommandInternal(unsigned char command_one,
        unsigned char command_two) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::vector<unsigned char> send_buffer;
    GetStandardMessage(send_buffer, command_one, command_two);
    EchoStatus status = msgProc_->trySend(send_buffer);
    return TryProcessEcho(status);
}

bool
InsteonDeviceImpl::TryProcessEcho(EchoStatus status) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    if (status == EchoStatus::ACK) { // got the echo
        utils::Logger::Instance().Debug("%s\n\t - reset ACK timer",
                FUNCTION_NAME_CSTR);
        ack_timer_->Reset(4583); // now we wait for the ack
        return true;
    } else if (status == EchoStatus::NAK) { // NAK or NONE received
        utils::Logger::Instance().Debug("%s\n\t - got NAK from PLM",
                FUNCTION_NAME_CSTR);
    } else {
        utils::Logger::Instance().Debug("%s\n\t - got nothing from PLM",
                FUNCTION_NAME_CSTR);
    }
    ClearPendingCommand(); // clear the command and move on
    return false;
}

/**
 * TrySendReceive
 * 
 * Tries to send a RAW INSTEON message and waits for a response.
 * Typical response message would be of type 0x50 or 0x51.
 * 
 * @param send_buffer RAW INSTEON data
 * @param retry_on_nak True if we should try sending more than once.
 * @param receive_message_id The type of INSTEON message we are waiting for (ie: 0x50)
 * @param properties The properties of the process INSTEON message
 * @return Returns EchoStatus value, ie: ACK or NAK
 */
EchoStatus
InsteonDeviceImpl::TrySendReceive(
        std::vector<unsigned char> send_buffer,
        bool retry_on_nak, unsigned char receive_message_id,
        PropertyKeys& properties) {
    return msgProc_->trySendReceive(send_buffer, retry_on_nak,
            receive_message_id, properties);
}

/**
 * WaitAndSetPendingCommand
 * 
 * blocks the current thread if a command is pending, then sets
 * the current command as pending command
 * 
 * NOTE: does not apply to all commands
 * 
 * @param command   Insteon command #1 field
 * @param value     Insteon command #2 field
 */
void
InsteonDeviceImpl::WaitAndSetPendingCommand(unsigned char command_one,
        unsigned char command_two) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    { // <<-- TODO build scoped lock MACRO to prevent the need of typing this
        std::lock_guard<std::mutex>_(command_mutex_);
        if (pending_command_ == 0) {
            pending_command_ = command_one;
            pending_command_two_ = command_two;
            pending_retry_ = 0;
            return; // new command, return and send it.
        }
    }

    // We sit here until the previous command completes
    if (ack_event_.WaitOne()) {
        utils::Logger::Instance().Debug("%s\n\t - pending command cleared.",
                FUNCTION_NAME_CSTR);
    }
    WaitAndSetPendingCommand(command_one, command_two);
}

/**
 * OnPendingCommandTimeOut
 * 
 * Callback function for impl_->ack_timer_
 * Any calls to impl_->ack_time_->Stop() will prevent this loop from running
 */
void
InsteonDeviceImpl::OnPendingCommandTimeout() {
    //ack_timer_->Stop();
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    unsigned char command_two = 0;
    unsigned char command_one = 0;
    {
        std::lock_guard<std::mutex>_(command_mutex_);
        command_one = pending_command_;
        command_two = pending_command_two_;
    }
    if (++pending_retry_ <= max_retries_) {
        //pending_retry_ += 1;
        utils::Logger::Instance().Info("%s\n\t  - retrying failed command for"
                " device %s{%s}\n\t  - command {0x%02x,0x%02x}",
                FUNCTION_NAME_CSTR,
                device_name_.c_str(),
                utils::int_to_hex(insteon_address_).c_str(),
                command_one, command_two);
        device_->io_strand_.post(std::bind(&InsteonDeviceImpl::TryCommandInternal, 
                this, command_one, command_two));
    } else {
        max_retries_ = max_retries_ - 1 > 0 ? max_retries_ -= 1 : 0;

        utils::Logger::Instance().Info("%s{%s} is not responding "
                "- max retries exceeded\n\t  - disabling device",
                device_name_.c_str(),
                utils::int_to_hex(insteon_address_).c_str());

        device_disabled_ = true;
        ClearPendingCommand();
    }
}

}
}
}
