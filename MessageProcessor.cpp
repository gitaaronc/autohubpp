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

#include <boost/format.hpp>
#include <boost/log/trivial.hpp>

#include <unistd.h>

#include <thread>

namespace ace {
namespace insteon {

MessageProcessor::MessageProcessor(boost::asio::io_service& io_service,
        YAML::Node config)
: io_service_(io_service), config_(config) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

MessageProcessor::~MessageProcessor() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

std::string
MessageProcessor::ByteArrayToStringStream(
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

bool
MessageProcessor::Connect() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);

    std::string plm_type = config_["PLM"]["type"].as<std::string>("serial"); 
    std::string host;
    int port = 0;
    std::unique_ptr<io::IOPort> io;

    std::transform(plm_type.begin(), plm_type.end(), 
            plm_type.begin(), ::tolower);
    
    if (plm_type.compare("hub") == 0){
        io = std::move(std::unique_ptr<io::IOPort>(
                new io::SocketPort(io_service_)));
        host = config_["PLM"]["hub_ip"].as<std::string>("127.0.0.1");
        port = config_["PLM"]["hub_port"].as<int>(9761);
    } else {
        io = std::move(std::unique_ptr<io::IOPort>(
                new io::SerialPort(io_service_)));
        host = config_["PLM"]["serial_port"].as<std::string>("/dev/ttyUSB0");
        port = config_["PLM"]["baud_rate"].as<int>(9600);
    }
    
    data_port_ = std::move(io);
    data_port_->set_recv_handler(std::bind(
            &type::ProcessData, this));
    
    if (data_port_->open(host, port)) {
        data_port_->async_read_some();
    }
    //    std::vector<unsigned char> temp1 = { 0x60 };
    //    io_service_.post(std::bind(&InsteonNetwork::Send, this, temp1));
    //    std::vector<unsigned char> temp2 = { 0x6B, 0x48 };
    //    io_service_.post(std::bind(&InsteonNetwork::Send, this, temp2));
}

void
MessageProcessor::ProcessData() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::ostringstream oss;
    oss << FUNCTION_NAME << std::endl;
    std::vector<unsigned char> read_buffer;
    {
        std::lock_guard<std::mutex>_(buffer_lock_);
        if (buffer_.size() > 0) {
            for (const auto& it : buffer_) {
                read_buffer.push_back(it);
            }
            buffer_.resize(0);
            std::vector<unsigned char>().swap(buffer_);
        }
    }
    if (data_port_->recv_buffer(read_buffer) > 0) {
        int count = 0;
        int offset = 0;
        int last = 0;
        while (offset < read_buffer.size()) {
            if (read_buffer[offset++] == 0x02) { // got start of text
                if (last != offset - 1) {
                    oss << boost::format(
                            "Skipping Bytes: [last:offset][%d:%d] {%s}\n")
                            % last % offset
                            % ByteArrayToStringStream(
                            read_buffer, last, offset - last);
                }
                do {
                    if (ProcessMessage(read_buffer, offset, count)) {
                        oss << boost::format("Message: {%s}\n")
                                % ByteArrayToStringStream(read_buffer,
                                offset - 1, offset + count);

                        offset += count;
                        last = offset;

                        break;
                    } else {
                        std::vector<unsigned char> more_data{};
                        oss << boost::format("We have %d bytes: (%d:%d) "
                                "{%s}\n") % (read_buffer.size()) % last
                                % (read_buffer.size() - last - 1)
                                % ByteArrayToStringStream(read_buffer, last,
                                read_buffer.size() - last);

                        ReadData(more_data, 1, false);
                        if (more_data.size() == 0) {
                        } else {
                            offset--;
                            for (const auto& it : more_data)
                                read_buffer.push_back(it);
                        }
                    }
                } while (++offset < read_buffer.size());
            }
        }
        //offset = offset > read_buffer.size() ? read_buffer.size() : offset;
        if (last != offset) {
            oss << boost::format("Discarding %d bytes: (%d:%d) {%s}\n")
                    % (offset - last) % last % (offset - 1)
                    % ByteArrayToStringStream(read_buffer, last, offset);
        }
    } else {
    }
    utils::Logger::Instance().Info(oss.str().c_str());
}

bool
MessageProcessor::ProcessEcho(
        const std::vector<unsigned char>& message_buffer,
        int offset, int& count) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);

    std::vector<unsigned char> message = utils::ArraySubset(
            message_buffer, offset, sent_message_.size());

    std::shared_ptr<InsteonMessage> insteon_message(
            std::make_shared<InsteonMessage>());

    if (insteon_protocol_.ProcessMessage(message_buffer, offset, count,
            insteon_message)) {
        return true;
    } else if (ace::utils::VectorsEqual(sent_message_, message)) {
        count = sent_message_.size();
        return true;
    }
    return false;
}

EchoStatus
MessageProcessor::ProcessEcho(int echo_length) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    EchoStatus status = EchoStatus::None;

    std::vector<unsigned char> read_buffer;
    ReadData(read_buffer, echo_length, true);
    if (read_buffer.size() == 0) {
        return EchoStatus::None;
    }

    if (read_buffer[0] == 0x15) {
        if (read_buffer.size() > 1) {
            buffer_lock_.lock();
            auto it = read_buffer.begin() + 1;
            for (; it != read_buffer.end(); ++it)
                buffer_.push_back(*it);
            buffer_lock_.unlock();
            ProcessData();
        }
        return EchoStatus::NAK;

    }
    int offset = 0;
    while (offset < read_buffer.size())
        if (read_buffer[offset++] == 0x02)
            break;
    if (offset >= read_buffer.size()) {
        return EchoStatus::Unknown;
    }

    if (offset > 1) {
        std::ostringstream oss;
        oss << boost::format(
                "Skipping Bytes: [last:offset][%d:%d] {%s}\n")
                % offset
                % ByteArrayToStringStream(
                read_buffer, 0, offset - 1);
        utils::Logger::Instance().Info(oss.str().c_str());
    }
    int count = 0;
    if (ProcessEcho(read_buffer, offset, count)) {
        int j = offset + count;
        unsigned char result = j < read_buffer.size() ? read_buffer[j] : 0x00;
        j += 1;
        if (read_buffer.size() > j) {
            buffer_lock_.lock();
            auto it = read_buffer.begin() + j;
            for (; it != read_buffer.end(); ++it)
                buffer_.push_back(*it);
            buffer_lock_.unlock();
            ProcessData();
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
MessageProcessor::ProcessMessage(const std::vector<unsigned char>& read_buffer,
        int offset, int& count) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::shared_ptr<InsteonMessage> insteon_message
            = std::make_shared<InsteonMessage>();
    if (insteon_protocol_.ProcessMessage(read_buffer, offset, count,
            insteon_message)) {
        time_of_last_command_ = std::chrono::system_clock::now();
        auto it = read_buffer.begin() + offset - 1;
        for (; it != read_buffer.begin() + offset + count; ++it)
            insteon_message->raw_message.push_back(*it); // copy the message
        UpdateWaitItems(insteon_message);
        if (msg_handler_)
            io_service_.post(std::bind(msg_handler_, insteon_message));
        return true;
    }
    return false;
}

void
MessageProcessor::ReadData(std::vector<unsigned char>& return_buffer,
        int bytes_expected, bool is_echo) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::vector<unsigned char> read_buffer;
    data_port_->recv_with_timeout(read_buffer, 250);

    if (is_echo && read_buffer.size() > 0 && read_buffer[0] == 0x15) {
        for (const auto& it : read_buffer)
            return_buffer.push_back(it);
        return;
    }

    if (bytes_expected > 0) {

        int count = 0;
        do {
            if (read_buffer.size() < bytes_expected) {
                // try one more time
                data_port_->recv_with_timeout(read_buffer, 50);
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
MessageProcessor::Send(std::vector<unsigned char> send_buffer,
        bool retry_on_nak, int echo_length) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::ostringstream oss;
    ProcessData(); // process any remaining data

    EchoStatus status = EchoStatus::None;
    send_buffer.insert(send_buffer.begin(), 0x02);
    int retry = 0;
    do {
        if (retry == 0) {
            oss << boost::format("Sending %d bytes: "
                    "{%s}\n") % (send_buffer.size())
                    % ByteArrayToStringStream(send_buffer, 0,
                    send_buffer.size());
        } else {
            oss << boost::format("Retrying %d bytes: "
                    "{%s}\n") % (send_buffer.size())
                    % ByteArrayToStringStream(send_buffer, 0,
                    send_buffer.size());
        }
        time_of_last_command_ = std::chrono::system_clock::now();
        utils::Logger::Instance().Info(oss.str().c_str());
        oss.str(std::string());
        oss.clear();
        data_port_->send_buffer(send_buffer);
        status = ProcessEcho(echo_length + 2); // +2 because the STX is not included
        if (status == EchoStatus::ACK) {
            oss << "EchoStatis::ACK\n";
            break;
        }
        if (status == EchoStatus::NAK && !retry_on_nak) {
            oss << "EchoStatis::NAK, no retry on NAK\n";
            break;
        }
        if (status == EchoStatus::NAK) {
            oss << "EchoStatis::NAK, sleeping for 240ms\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(240));
        }
        retry++;
    } while (retry < 3 && retry_on_nak);
    utils::Logger::Instance().Info(oss.str().c_str());
    time_of_last_command_ = std::chrono::system_clock::now();
    return status;
}

/**
 * TrySend
 * @param send_buffer The data to send
 * @return 
 */
EchoStatus
MessageProcessor::TrySend(const std::vector<unsigned char>& send_buffer) {
    return TrySend(send_buffer, true);
}

/**
 * TrySend
 * @param send_buffer
 * @param retry_on_nak
 * @return 
 */
EchoStatus
MessageProcessor::TrySend(const std::vector<unsigned char>& send_buffer,
        bool retry_on_nak) {
    return TrySend(send_buffer, retry_on_nak, send_buffer.size());
}

/**
 * TrySend
 * @param send_buffer
 * @param retry_on_nak
 * @param echo_length
 * @return 
 */
EchoStatus
MessageProcessor::TrySend(const std::vector<unsigned char>& send_buffer,
        bool retry_on_nak, int echo_length) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::lock_guard<std::mutex>lock(io_lock_);
    EchoStatus status = EchoStatus::None;
    data_port_->set_recv_handler(nullptr);


    auto duration = config_["PLM"]["command_delay"].as<int>(1500);
    auto start = std::chrono::system_clock::now();
    auto difference = std::chrono::duration_cast<std::chrono::milliseconds>
            (start - time_of_last_command_).count();
    while (difference < duration) {
        utils::Logger::Instance().Debug("Sleeping for %d ms before sending",
                duration - difference);
        std::this_thread::sleep_for(
                std::chrono::milliseconds(duration - difference));
        start = std::chrono::system_clock::now();
        difference = std::chrono::duration_cast<std::chrono::milliseconds>
                (start - time_of_last_command_).count();
    }

    sent_message_ = send_buffer;
    status = Send(send_buffer, retry_on_nak, echo_length);
    sent_message_.empty();
    data_port_->set_recv_handler(std::bind(
            &type::ProcessData, this));
    data_port_->async_read_some();
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
MessageProcessor::TrySendReceive(const std::vector<unsigned char>& send_buffer,
        bool retry_on_nak, unsigned char receive_message_id, PropertyKeys&
        properties) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);

    std::shared_ptr<WaitItem>item(new WaitItem(receive_message_id));
    properties.clear();
    {
        std::lock_guard<std::mutex>lock(mutex_wait_list_);
        // always push to the back as IM responses should be in order of sent message
        wait_list_.push_back(item);
    }
    EchoStatus status = TrySend(send_buffer, retry_on_nak);
    if (status == EchoStatus::ACK) { // got the echo
        if (!item->insteon_message_) { // still need ACK
            if (item->wait_event_.WaitOne(4000)) { // wait here for ACK
                utils::Logger::Instance().Info("ACK received");
            } else { // timeout signaled, no event
                utils::Logger::Instance().Info("Timeout signaled: no ACK received");
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
MessageProcessor::UpdateWaitItems(const std::shared_ptr<InsteonMessage>& iMsg) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
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
}

void
MessageProcessor::set_message_handler(msg_handler handler) {
    msg_handler_ = handler;
}

}
/* namespace insteon*/
} // namespace ace