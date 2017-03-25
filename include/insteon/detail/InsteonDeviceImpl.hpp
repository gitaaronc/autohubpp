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

#ifndef INSTEONDEVICEBASEIMPL_H
#define	INSTEONDEVICEBASEIMPL_H

#include "../EchoStatus.hpp"
#include "../PropertyKey.hpp"
#include "../InsteonDevice.hpp"
#include "../MessageProcessor.hpp"
#include "../InsteonMessage.hpp"
#include "../PropertyKey.hpp"

#include "../../system/Timer.hpp"
#include "../../system/AutoResetEvent.hpp"

#include <memory>
#include <mutex>

namespace ace {
namespace insteon {
namespace detail {

class InsteonDeviceImpl {
    typedef InsteonDeviceImpl type;
public:
    InsteonDeviceImpl(InsteonDevice* pDevice, int insteon_address);
    ~InsteonDeviceImpl();

    void ClearPendingCommand();

    void GetStandardMessage(std::vector<unsigned char>& send_buffer,
            unsigned char cmd1, unsigned char cmd2);
    void GetExtendedMessage(std::vector<unsigned char>& send_buffer,
            unsigned char cmd1, unsigned char cmd2,
            unsigned char d1 = 0, unsigned char d2 = 0, unsigned char d3 = 0,
            unsigned char d4 = 0, unsigned char d5 = 0, unsigned char d6 = 0,
            unsigned char d7 = 0, unsigned char d8 = 0, unsigned char d9 = 0,
            unsigned char d10 = 0, unsigned char d11 = 0, unsigned char d12 = 0,
            unsigned char d13 = 0);
    void CommandAckProcessor(const std::shared_ptr<InsteonMessage>&
            insteon_message);

    void
    set_message_proc(std::shared_ptr<MessageProcessor> messenger) {
        msgProc_ = messenger;
    }

    bool TryCommandInternal(unsigned char command_one, unsigned char command_two);
    EchoStatus TrySendReceive(std::vector<unsigned char> send_buffer,
            bool retry_on_nak, unsigned char receive_message_id,
            PropertyKeys& properties);

    void WaitAndSetPendingCommand(unsigned char command_one,
            unsigned char command_two);


    void OnPendingCommandTimeout();

    bool TryProcessEcho(EchoStatus status = EchoStatus::Unknown);
    
    unsigned char
    HighAddress() {
        return insteon_address_ >> 16 & 0xFF;
    }

    unsigned char
    MiddleAddress() {
        return insteon_address_ >> 8 & 0xFF;
    }

    unsigned char
    LowAddress() {
        return insteon_address_ & 0xFF;
    }

    InsteonDevice* device_;
    std::shared_ptr<MessageProcessor> msgProc_;

    system::AutoResetEvent ack_event_;
    std::unique_ptr<system::Timer> ack_timer_;

    std::mutex command_mutex_;
    unsigned char pending_command_;
    unsigned char pending_command_two_;
    int pending_retry_;
    int max_retries_;
    
    int insteon_address_;
    std::string device_name_;
    bool device_disabled_;
    
};
}
}
}

#endif	/* INSTEONDEVICEBASEIMPL_H */

