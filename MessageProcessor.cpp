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
#include "include/insteon/MessageProcessor.hpp"
#include "include/insteon/InsteonMessage.hpp"
#include "include/utils/utils.hpp"
#include "include/Logger.h"
#include "include/system/Timer.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <future>
#include <chrono>
#include <algorithm>

#include <unistd.h>

#include <thread>

namespace ace
{
namespace insteon
{

MessageProcessor::MessageProcessor(boost::asio::io_service& io_service,
        YAML::Node config)
: io_service_(io_service), io_strand_(io_service), config_(config) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

MessageProcessor::~MessageProcessor() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

bool
MessageProcessor::connect() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);

    std::string plm_type = config_["type"].as<std::string>("serial");
    std::string host;
    uint32_t port = 0;
    std::unique_ptr<io::IOPort> io;

    std::transform(plm_type.begin(), plm_type.end(),
            plm_type.begin(), ::tolower);

    if (plm_type.compare("hub") == 0) {
        io = std::move(std::unique_ptr<io::IOPort>(
                new io::SocketPort(io_service_)));
        host = config_["hub_ip"].as<std::string>("127.0.0.1");
        port = config_["hub_port"].as<int>(9761);
    } else {
        io = std::move(std::unique_ptr<io::IOPort>(
                new io::SerialPort(io_service_)));
        host = config_["serial_port"].as<std::string>("/dev/ttyUSB0");
        port = config_["baud_rate"].as<int>(9600);
    }

    io_port_ = std::move(io);
    io_port_->set_recv_handler(std::bind(
            &type::onReceive, this));

    if (io_port_->open(host, port)) {
        return true;
    }
    return false;

}

void
MessageProcessor::onReceive() {
    std::lock_guard<std::mutex>_(lock_data_processor_);
    processData();
}

void
MessageProcessor::processData() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    // force other thread to wait
    //std::lock_guard<std::mutex>_(lock_data_processor_);
    std::vector<uint8_t> read_buffer;
    {
        // move the object buffer to the back of local buffer
        std::lock_guard<std::mutex>_(lock_buffer_);
        if (buffer_.size() > 0) {
            for (const auto& it : buffer_) {
                read_buffer.push_back(it);
            }
            buffer_.resize(0);
            std::vector<uint8_t>().swap(buffer_);
        }
    }
    // get data from the io port while looking for a complete message
    if ((io_port_->recv_buffer(read_buffer) > 0) || (read_buffer.size() > 0)) {
        uint32_t count = 0;
        uint32_t offset = 0;
        uint32_t last = 0;
        while (offset < read_buffer.size()) { // iterate buffer looking for STX
            if (read_buffer[offset++] == 0x02) { // got STX
                if (last != offset - 1) {
                    utils::Logger::Instance().Info(
                            "%s\n\t  - skipping %d bytes "
                            "[last:offset][%d:%d]: {%s}\n", FUNCTION_NAME_CSTR,
                            offset - last - 1, last, offset - 2,
                            utils::ByteArrayToStringStream(read_buffer,
                            last, offset - last - 1).c_str()
                            );
                }
                do { // try to make sense of the data
                    if (processMessage(read_buffer, offset, count)) {
                        utils::Logger::Instance().Info("%s\n"
                                "\t  - message parsed [begin:end]"
                                "[%d:%d]: {%s}", FUNCTION_NAME_CSTR,
                                offset - 1, offset - 1 + count,
                                utils::ByteArrayToStringStream(read_buffer,
                                offset - 1, count + 1).c_str());
                        offset += count;
                        last = offset;
                        break; // found a message, get out of do while loop
                    } else { // still looking
                        std::vector<uint8_t> more_data{};

                        utils::Logger::Instance().Info(
                                "%s\n\t  - working with %d bytes: (%d:%d) {%s}\n",
                                FUNCTION_NAME_CSTR, read_buffer.size() - last,
                                last, read_buffer.size() - 1,
                                utils::ByteArrayToStringStream(read_buffer,
                                last, read_buffer.size() - last).c_str()
                                );

                        readData(more_data, 1, false);
                        if (more_data.size() == 0) {
                        } else { // TODO prevent to much buffer growth
                            offset--;
                            for (const auto& it : more_data)
                                read_buffer.push_back(it);
                        }
                    }
                } while (++offset < read_buffer.size());
            }
        }
        if (last != offset) {
            utils::Logger::Instance().Info(
                    "%s\n\t  - discarding %d bytes: (%d:%d) {%s}\n",
                    FUNCTION_NAME_CSTR, read_buffer.size() - last, last,
                    read_buffer.size() - 1, utils::ByteArrayToStringStream(
                    read_buffer, last, read_buffer.size() - last).c_str()
                    );
        }
    } else {
        ace::utils::Logger::Instance().Warning("%s\n \t  - %s",
                FUNCTION_NAME_CSTR, "nothing to do");
    }
}

EchoStatus
MessageProcessor::processEcho(uint32_t echo_length) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    EchoStatus status = EchoStatus::None;

    std::vector<uint8_t> read_buffer;
    readData(read_buffer, echo_length, true);
    if (read_buffer.size() == 0) {
        return EchoStatus::None;
    }

    if (read_buffer[0] == 0x15) {
        if (read_buffer.size() > 1) {
            lock_buffer_.lock();
            auto it = read_buffer.begin() + 1;
            for (; it != read_buffer.end(); ++it)
                buffer_.push_back(*it);
            lock_buffer_.unlock();
            processData();
        }
        return EchoStatus::NAK;

    }
    uint32_t offset = 0;
    while (offset < read_buffer.size())
        if (read_buffer[offset++] == 0x02)
            break;
    if (offset >= read_buffer.size()) {
        return EchoStatus::Unknown;
    }

    if (offset > 1) {
        utils::Logger::Instance().Info(
                "%s\n\t  - skipping bytes between: [last:offset][%d:%d] {%s}\n",
                FUNCTION_NAME_CSTR, offset, utils::ByteArrayToStringStream(
                read_buffer, 0, offset - 1).c_str()
                );
    }
    uint32_t count = 0;
    if (processMessage(read_buffer, offset, count, true)) {
        uint32_t j = offset + count; // < echo_length ? echo_length - 1 : offset + count ;
        uint8_t result = read_buffer[j - 1];
        if (read_buffer.size() > j) {
            lock_buffer_.lock();
            auto it = read_buffer.begin() + j;
            for (; it != read_buffer.end(); ++it)
                buffer_.push_back(*it);
            lock_buffer_.unlock();
            processData();
        }
        if (result == 0x06) {
            status = EchoStatus::ACK;
        } else if (result == 0x15) {
            status = EchoStatus::NAK;
        } else {
            status = EchoStatus::Unknown;
        }
    } else {
        status = EchoStatus::Unknown;
    }
    return status;
}

bool
MessageProcessor::processMessage(const std::vector<uint8_t>& read_buffer,
        uint32_t offset, uint32_t& count, bool is_echo) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::shared_ptr<InsteonMessage> insteon_message
            = std::make_shared<InsteonMessage>();
    if (insteon_protocol_.processMessage(read_buffer, offset, count,
            insteon_message)) {
        time_of_last_command_ = std::chrono::system_clock::now();
        auto it = read_buffer.begin() + offset - 1;
        for (; it < read_buffer.begin() + offset + count; it++)
            insteon_message->raw_message.push_back(*it); // copy the buffer
        if (offset + count < read_buffer.size()) {
            auto response = *(read_buffer.begin() + offset + count);
            if (response == 0x06 || response == 0x15) {
                insteon_message->raw_message.push_back(response);
                count++;
            }
        } else if (is_echo) {
            return false;
        }
        updateWaitItems(insteon_message);
        if (msg_handler_)
            io_strand_.post(std::bind(msg_handler_, insteon_message));
        return true;
    }
    return false;
}

void
MessageProcessor::readData(std::vector<uint8_t>& return_buffer,
        uint32_t bytes_expected, bool is_echo) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::vector<uint8_t> read_buffer;
    io_port_->recv_with_timeout(read_buffer, 250);

    if (is_echo && read_buffer.size() > 0 && read_buffer[0] == 0x15) {
        for (const auto& it : read_buffer)
            return_buffer.push_back(it);
        return;
    }

    if (bytes_expected > 0) {

        uint32_t count = 0;
        do {
            if (read_buffer.size() < bytes_expected) {
                // try one more time
                io_port_->recv_with_timeout(read_buffer, 50);
                count++;
            } else {
                break;
            }
        } while (count < 3);

        if (is_echo && (read_buffer.size() > 0) && (read_buffer[0] == 0x15)) {
            for (const auto& it : read_buffer)
                return_buffer.push_back(it);
            return;
        }

        if (read_buffer.size() < bytes_expected) {
            if (read_buffer.size() > 0) {
            } else {
            }
        }
    }
    for (const auto& it : read_buffer)
        return_buffer.push_back(it);
}

EchoStatus
MessageProcessor::send(std::vector<uint8_t> send_buffer,
        bool retry_on_nak, uint32_t echo_length) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::lock_guard<std::mutex>_(lock_data_processor_);
    std::ostringstream oss;
    processData(); // process any remaining data

    EchoStatus status = EchoStatus::None;
    send_buffer.insert(send_buffer.begin(), 0x02);
    uint32_t retry = 0;
    do {
        if (retry == 0) {
            utils::Logger::Instance().Info("%s\n\t - sending %d bytes: {%s}\n",
                    FUNCTION_NAME_CSTR, send_buffer.size(),
                    utils::ByteArrayToStringStream(send_buffer, 0,
                    send_buffer.size()).c_str());
        } else {
            utils::Logger::Instance().Info("%s\n\t - retrying %d bytes: {%s}\n",
                    FUNCTION_NAME_CSTR, send_buffer.size(),
                    utils::ByteArrayToStringStream(send_buffer, 0,
                    send_buffer.size()).c_str());
        }
        time_of_last_command_ = std::chrono::system_clock::now();
        oss.str(std::string());
        io_port_->send_buffer(send_buffer);
        status = processEcho(echo_length + 2); // +2 because the STX is not included
        if (status == EchoStatus::ACK) {
            break;
        }
        if (status == EchoStatus::NAK && !retry_on_nak) {
            break;
        }
        if (status == EchoStatus::NAK) {
            oss << "\t  - EchoStatus::NAK received, sleeping for 240ms\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(240));
        }
        retry++;
    } while (retry < 3 && retry_on_nak);
    utils::Logger::Instance().Info("%s\n%s", FUNCTION_NAME_CSTR,
            oss.str().c_str());
    time_of_last_command_ = std::chrono::system_clock::now();
    return status;
}

/**
 * TrySend
 * @param send_buffer
 * @param retry_on_nak
 * @return 
 */
EchoStatus
MessageProcessor::trySend(const std::vector<uint8_t>& send_buffer,
        bool retry_on_nak) {
    return trySend(send_buffer, retry_on_nak, send_buffer.size());
}

/**
 * TrySend
 * @param send_buffer
 * @param retry_on_nak
 * @param echo_length
 * @return 
 */
EchoStatus
MessageProcessor::trySend(const std::vector<uint8_t>& send_buffer,
        bool retry_on_nak, uint32_t echo_length) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    // force other sending threads to wait for us to finish
    //std::lock_guard<std::mutex>lock(lock_io_);
    EchoStatus status = EchoStatus::None;
    io_port_->set_recv_handler(nullptr); // prevent io from calling a handler

    auto duration = config_["command_delay"].as<int>(500);
    auto start = std::chrono::system_clock::now();
    auto difference = std::chrono::duration_cast<std::chrono::milliseconds>
            (start - time_of_last_command_).count();
    while (difference < duration) {
        utils::Logger::Instance().Info("%s\n\t  - sleeping for %ims before "
                "sending", FUNCTION_NAME_CSTR, int(duration - difference));
        std::this_thread::sleep_for(
                std::chrono::milliseconds(duration - difference));
        start = std::chrono::system_clock::now();
        difference = std::chrono::duration_cast<std::chrono::milliseconds>
                (start - time_of_last_command_).count();
    }

    status = send(send_buffer, retry_on_nak, echo_length);
    io_port_->set_recv_handler(std::bind(
            &type::onReceive, this));
    io_port_->async_read_some();
    return status;
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
MessageProcessor::trySendReceive(const std::vector<uint8_t>& send_buffer,
        uint8_t triesLeft, uint8_t receive_message_id, PropertyKeys&
        properties) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);

    std::shared_ptr<WaitItem>item(new WaitItem(receive_message_id));
    properties.clear();
    {
        std::lock_guard<std::mutex>lock(mutex_wait_list_);
        // always push to the back as IM responses should be in order of sent message
        wait_list_.push_back(item);
    }
    EchoStatus status = trySend(send_buffer, triesLeft >= 0);
    if (status == EchoStatus::ACK) { // got the echo
        if (!item->insteon_message_) { // still need ACK
            if (item->wait_event_.WaitOne(4000)) { // wait here for ACK
                utils::Logger::Instance().Info("%s\n\t  - ACK received",
                        FUNCTION_NAME_CSTR);
            } else { // timeout signaled, no event
                utils::Logger::Instance().Info("%s\n\t  - Timeout signaled: no "
                        "ACK received\n\t  - Retrying command", FUNCTION_NAME_CSTR);
                if (--triesLeft >= 0) {
                    {
                        std::lock_guard<std::mutex>lock(mutex_wait_list_);
                        wait_list_.remove(item);
                    }
                    return trySendReceive(send_buffer, triesLeft,
                            receive_message_id, properties);
                }

            }
        }
        if (item->insteon_message_) {
            properties = item->insteon_message_->properties_;
        }
    }
    {
        std::lock_guard<std::mutex>lock(mutex_wait_list_);
        wait_list_.remove(item);
    }
    return status;
}

void
MessageProcessor::updateWaitItems(const std::shared_ptr<InsteonMessage>& iMsg) {
    std::lock_guard<std::mutex>lock(mutex_wait_list_);
    auto it = wait_list_.begin();
    for (; it != wait_list_.end(); it++) {
        if (iMsg->message_id_ == (*it)->message_id_) {
            if (!(*it)->insteon_message_) {
                (*it)->insteon_message_ = iMsg;
                (*it)->message_received_ = true;
                (*it)->wait_event_.Set();
            }
        }
    }
    utils::Logger::Instance().Trace("%s\n\t  - remaining items %zu",
            FUNCTION_NAME_CSTR, wait_list_.size());
}

void
MessageProcessor::set_message_handler(msg_handler handler) {
    msg_handler_ = handler;
}

}
/* namespace insteon*/
} // namespace ace