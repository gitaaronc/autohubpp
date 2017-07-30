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

#ifndef INSTEONPROTOCOL_HPP
#define INSTEONPROTOCOL_HPP

#include "InsteonMessageType.hpp"
#include "PropertyKey.hpp"

#include <memory>
#include <vector>
#include <cstdint>

namespace ace {
    namespace insteon {
        class InsteonMessage;

        class InsteonProtocol {
        public:
            InsteonProtocol();
            ~InsteonProtocol();
            bool processMessage(const std::vector<uint8_t>& data, uint32_t offset,
                    int& count, std::shared_ptr<InsteonMessage>& insteon_message);

            bool
            processEcho(const std::vector<uint8_t>& data, uint32_t offset,
                    int& count) {
                return true;
            }

        private:
            bool standardMessage(const std::vector<uint8_t>& data, uint32_t offset,
                    uint32_t &count, std::shared_ptr<InsteonMessage>& insteon_message);

            bool extendedMessage(const std::vector<uint8_t>& data, uint32_t offset,
                    uint32_t &count, std::shared_ptr<InsteonMessage>& insteon_message);

            bool aldbRecord(const std::vector<uint8_t>& data, uint32_t offset,
                    uint32_t &count, std::shared_ptr<InsteonMessage>& insteon_message);

            bool deviceLinkMessage(const std::vector<uint8_t>& data, uint32_t offset,
                    uint32_t &count, std::shared_ptr<InsteonMessage>& insteon_message);

            bool deviceLinkRecordMessage(const std::vector<uint8_t>& data,
                    uint32_t offset, int& count,
                    std::shared_ptr<InsteonMessage>& insteon_message);

            bool deviceLinkCleanupMessage(const std::vector<uint8_t>& data,
                    uint32_t offset, int& count,
                    std::shared_ptr<InsteonMessage>& insteon_message);

            bool getIMConfiguration(const std::vector<uint8_t>& data, uint32_t offset,
                    uint32_t &count, std::shared_ptr<InsteonMessage>& insteon_message);

            bool getIMInfo(const std::vector<uint8_t>& data, uint32_t offset,
                    uint32_t &count, std::shared_ptr<InsteonMessage>& insteon_message);

            bool getAddressProperty(std::string key,
                    const std::vector<uint8_t>& data, uint32_t offset,
                    int& count, PropertyKeys& properties);

            bool getMessageFlagProperty(const std::vector<uint8_t>& data,
                    uint32_t offset, uint32_t &count, PropertyKeys& properties);

            InsteonMessageType getMessageType(const std::vector<uint8_t>& data,
                    uint32_t offset, PropertyKeys& properties);

            bool imSetButtonEvent(const std::vector<uint8_t>& data, uint32_t offset,
                    uint32_t &count, std::shared_ptr<InsteonMessage>& insteon_message);

            bool directMessage(const std::vector<uint8_t>& data,
                    uint32_t offset, int& count,
                    std::shared_ptr<InsteonMessage>& insteon_message);
        };
    } // namespace insteon
} // namespace ace
#endif /* INSTEONPROTOCOL_HPP */

