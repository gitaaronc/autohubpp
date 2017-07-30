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
#include "include/insteon/InsteonProtocol.hpp"
#include "include/insteon/InsteonMessage.hpp"

namespace ace
{
namespace insteon
{

InsteonProtocol::InsteonProtocol() {
}

InsteonProtocol::~InsteonProtocol() {

}

bool
InsteonProtocol::getAddressProperty(const std::string key,
        const std::vector<uint8_t>&data, uint32_t offset, uint32_t& count,
        PropertyKeys& properties) {
    if (data.size() < offset + 3)
        return false;
    uint32_t address = data[offset + count++] << 16 | data[offset + count++] << 8 | data[offset + count++];
    properties[key] = address;
    return true;
}

bool
InsteonProtocol::getMessageFlagProperty(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count, PropertyKeys& properties) {
    if (data.size() < offset + 1)
        return false;

    uint8_t messageFlags = data[offset + count++];

    properties["message_flags_max_hops"] = messageFlags & 0b00000011;
    properties["message_flags_hops_remaining"] =
            (messageFlags & 0b00001100) >> 2;
    properties["message_flags_extended"] =
            (messageFlags & 0b00010000) >> 4;
    properties["message_flags_ack"] =
            (messageFlags & 0b00100000) >> 5; //
    properties["message_flags_group"] =
            (messageFlags & 0b01000000) >> 6; //
    properties["message_flags_broadcast"] =
            (messageFlags & 0b10000000) >> 7; //

    return true;
}

/* GetMessageType
 * 
 * Responsible for decoding the Insteon Message Type
 * eg: Broadcast, All Linking, ACK, NAK
 */
InsteonMessageType
InsteonProtocol::getStandardMessageType(const std::vector<uint8_t>& data,
        uint32_t offset, PropertyKeys& properties) {
    //0250/26deeb/000001/cb/14/00

    uint8_t cmd1 = properties["command_one"];
    bool broadcast = properties["message_flags_broadcast"] != 0; //bit7
    bool group = properties["message_flags_group"] != 0; //bit6
    bool ack = properties["message_flags_ack"] != 0; //bit5

    InsteonMessageType message_type = InsteonMessageType::Other;
    if (ack) {
        message_type = InsteonMessageType::Ack;
    } else if (cmd1 == 0x06 && broadcast && group) {
        message_type = InsteonMessageType::SuccessBroadcast;
        properties["responder_command_one"] = data[offset + 4];
        properties["responder_count"] = data[offset + 5];
        properties["responder_group"] = data[offset + 6];
        properties["responder_error_count"] = data[offset + 9];
    } else if (cmd1 == 0x11 && broadcast && group) {
        message_type = InsteonMessageType::OnBroadcast;
        properties["group"] = data[offset + 5];
    } else if (cmd1 == 0x11 && !broadcast && group) {
        message_type = InsteonMessageType::OnCleanup;
        properties["group"] = data[offset + 9];
    } else if (cmd1 == 0x13 && broadcast && group) {
        message_type = InsteonMessageType::OffBroadcast;
        properties["group"] = data[offset + 5];
    } else if (cmd1 == 0x13 && !broadcast && group) {
        message_type = InsteonMessageType::OffCleanup;
        properties["group"] = data[offset + 9];
    } else if (cmd1 == 0x12 && broadcast && group) {
        message_type = InsteonMessageType::FastOnBroadcast;
        properties["group"] = data[offset + 5];
    } else if (cmd1 == 0x12 && !broadcast && group) {
        message_type = InsteonMessageType::FastOnCleanup;
        properties["group"] = data[offset + 9];
    } else if (cmd1 == 0x14 && broadcast && group) {
        message_type = InsteonMessageType::FastOffBroadcast;
        properties["group"] = data[offset + 5];
    } else if (cmd1 == 0x14 && !broadcast && group) {
        message_type = InsteonMessageType::FastOffCleanup;
        properties["group"] = data[offset + 9];
    } else if (cmd1 == 0x17 && broadcast && group) {
        message_type = InsteonMessageType::IncrementBeginBroadcast;
        properties["group"] = data[offset + 5];
        properties["increment_direction"] = data[offset + 9];
    } else if (cmd1 == 0x18 && broadcast && group) {
        message_type = InsteonMessageType::IncrementEndBroadcast;
        properties["group"] = data[offset + 5];
    } else if (cmd1 == 0x01 || cmd1 == 0x02) {
        message_type = InsteonMessageType::SetButtonPressed;
        properties["device_category"] = data[offset + 4];
        properties["device_subcategory"] = data[offset + 5];
        properties["device_firmware_version"] = data[offset + 6];
    } else if (!broadcast && !group && !ack) {
        message_type = InsteonMessageType::DirectMessage;
    }
    return message_type;
}

/* ProcessMessage
 * Decodes all Insteon Messages into PropertyKeys(PropertyKey, value) PropertyKey.h
 */
bool
InsteonProtocol::processMessage(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count, std::shared_ptr<InsteonMessage>& insteon_message) {
    count = 1;
    switch (data[offset]) {
        case 0x50: // receive standard message
            return standardMessage(data, offset, count, insteon_message);
        case 0x51: // receive extended message
            return extendedMessage(data, offset, count, insteon_message);
        case 0x53: // receive all linking complete
            return deviceLinkMessage(data, offset, count, insteon_message);
        case 0x54: // IM set button pressed
            return imSetButtonEvent(data, offset, count, insteon_message);
        case 0x57: // receive all link record response
            return deviceLinkRecordMessage(data, offset, count, insteon_message);
        case 0x58: // receive all link cleanup response
            return deviceLinkCleanupMessage(data, offset, count, insteon_message);
        case 0x59: // receive database record found
            if (!aldbRecord(data, offset, count, insteon_message)) return false;
            if (!decodeLinkRecord(data, offset, count, insteon_message->properties_)) return false;
            return true;
        case 0x60: // get insteon modem info
            return getIMInfo(data, offset, count, insteon_message);
        case 0x61:
            if (data.size() < offset + count + 3) return false;
            count += 3;
            return true;
        case 0x62:
            return directMessage(data, offset, count, insteon_message);
        case 0x63:
            if (data.size() < offset + count + 2) return false;
            count += 2;
            return true;
        case 0x64:
            if (data.size() < offset + count + 2) return false;
            count += 2;
            return true;
        case 0x65:
            return (data.size() < offset + count) ? false : true;
        case 0x66:
            if (data.size() < offset + count + 3) return false;
            count += 3;
            return true;
        case 0x67:
            if (data.size() < offset + count) return false;
            return true;
        case 0x68:
            if (data.size() < offset + count + 1) return false;
            count += 1;
            return true;
        case 0x69:
            if (data.size() < offset + count) return false;
            return true;
        case 0x6A:
            if (data.size() < offset + count) return false;
            return true;
        case 0x6B: // TODO set IM configuration 
            if (data.size() < offset + count + 1) return false;
            count += 1;
            return true;
        case 0x6C:
            if (data.size() < offset + count) return false;
            return true;
        case 0x6D:
            if (data.size() < offset + count) return false;
            return true;
        case 0x6E:
            if (data.size() < offset + count) return false;
            return true;
        case 0x6F:
            if (data.size() < offset + count + 9) return false;
            count += 9;
            return true;
        case 0x70:
            if (data.size() < offset + count + 1) return false;
            count += 1;
            return true;
        case 0x71:
            if (data.size() < offset + count + 2) return false;
            count += 2;
            return true;
        case 0x72:
            if (data.size() < offset + count) return false;
            return true;
        case 0x73: // get insteon modem configuration
            return getIMConfiguration(data, offset, count, insteon_message);
        case 0x74:
            if (data.size() < offset + count) return false;
            return true;
        case 0x75: // TODO Read 8 bytes from database
            if (data.size() < offset + count + 2) return false;
            count += 2;
            return true;
        case 0x76: // TODO Write 8 bytes to Database
            if (data.size() < offset + count + 10) return false;
            count += 10;
            return true;
        case 0x77:
            if (data.size() < offset + count) return false;
            return true;
        case 0x78:
            if (data.size() < offset + count + 1) return false;
            count += 1;
            return true;
        case 0x79:
            if (data.size() < offset + count + 3) return false;
            count += 3;
            return true;

    }
    return false;
}

/**
 * StandardMessage 0x50
 * @param data
 * @param offset
 * @param count
 * @param insteon_message
 * @return 
 */
bool
InsteonProtocol::standardMessage(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count,
        std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + count + 9)
        return false;

    uint8_t message_id = data[offset];

    PropertyKeys properties;
    getAddressProperty("from_address", data, offset, count, properties);
    getMessageFlagProperty(data, offset, count, properties);
    if (properties.find("message_flags_group")->second
            == 0) {
        getAddressProperty("to_address", data, offset, count,
                properties);
    } else {
        count += 3;
    }
    properties["command_one"] = data[offset + count++];
    properties["command_two"] = data[offset + count++];

    InsteonMessageType message_type = getStandardMessageType(data, offset, properties);
    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
    return true;

}

/**
 * ExtendedMessage 0x51
 * @param data
 * @param offset
 * @param count
 * @param insteon_message
 * @return 
 */
bool
InsteonProtocol::extendedMessage(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (!standardMessage(data, offset, count, insteon_message))
        return false;

    if (data.size() < offset + count + 14) return false;
    insteon_message->properties_["data_one"] = data[offset + count++];
    insteon_message->properties_["data_two"] = data[offset + count++];
    switch (insteon_message->properties_["command_one"]) {
        case 0x2f:
            if (aldbRecord(data, offset, count, insteon_message)) {
                insteon_message->properties_["data_five"] = data[offset + count++];
                if (!decodeLinkRecord(data, offset, count, 
                        insteon_message->properties_)) return false;
                //insteon_message->properties_["data_thirteen"] = data[offset + count++];
                insteon_message->properties_["data_fourteen"] = data[offset + count++];
                return true;
            }
            return false;
    }
    insteon_message->properties_["data_three"] = data[offset + count++];
    insteon_message->properties_["data_four"] = data[offset + count++];
    insteon_message->properties_["data_five"] = data[offset + count++];
    insteon_message->properties_["data_six"] = data[offset + count++];
    insteon_message->properties_["data_seven"] = data[offset + count++];
    insteon_message->properties_["data_eight"] = data[offset + count++];
    insteon_message->properties_["data_nine"] = data[offset + count++];
    insteon_message->properties_["data_ten"] = data[offset + count++];
    insteon_message->properties_["data_eleven"] = data[offset + count++];
    insteon_message->properties_["data_twelve"] = data[offset + count++];
    insteon_message->properties_["data_thirteen"] = data[offset + count++];
    insteon_message->properties_["data_fourteen"] = data[offset + count++];

    return true;
}

/**
 * DeviceLinkMessage 0x53
 * @param data
 * @param offset
 * @param count
 * @param insteon_message
 * @return 
 */
bool
InsteonProtocol::deviceLinkMessage(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < count + offset + 8)
        return false;

    uint8_t message_id = data[offset];

    PropertyKeys properties;
    if (!decodeLinkRecord(data, offset, count, properties)) return false;

    InsteonMessageType message_type = InsteonMessageType::DeviceLink;
    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
    return true;
}

/**
 * IMSetButtonEvent 0x54
 * @param data
 * @param offset
 * @param count
 * @param proeprties
 * @return 
 */
bool
InsteonProtocol::imSetButtonEvent(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + count + 1) return false;

    uint8_t message_id = data[offset];

    PropertyKeys properties;
    properties["im_set_button_event"] = data[offset + count++];

    InsteonMessageType message_type = InsteonMessageType::SetButtonPressed;
    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
    return true;
}

/**
 * DeviceLinkRecordMessage 0x57
 * @param data
 * @param offset
 * @param count
 * @param insteon_message
 * @return 
 */
bool
InsteonProtocol::deviceLinkRecordMessage(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + 8)
        return false;

    uint8_t message_id = data[offset];

    PropertyKeys properties;
    if (!decodeLinkRecord(data, offset, count, properties)) return false;
    
    /*properties["link_record_flags"] = data[offset + count++];
    properties["link_group"] = data[offset + count++];
    if (!getAddressProperty("link_address", data, offset, count, properties))
        return false; // count += 3
    properties["link_data_one"] = data[offset + count++];
    properties["link_data_two"] = data[offset + count++];
    properties["link_data_three"] = data[offset + count++];*/

    InsteonMessageType message_type = InsteonMessageType::DeviceLinkRecord;
    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
    return true;
}

/**
 * DeviceLinkCleanupMessage 0x58
 * @param data
 * @param offset
 * @param count
 * @param insteon_message
 * @return 
 */
bool
InsteonProtocol::deviceLinkCleanupMessage(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + count + 1)
        return false;

    PropertyKeys properties;
    uint8_t message_id = data[offset];

    properties["link_status"] = data[offset + count++];

    InsteonMessageType message_type = InsteonMessageType::DeviceLinkCleanup;
    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
    return true;
}

/**
 * DatabaseRecordFound 0x59
 * @param data
 * @param offset
 * @param count
 * @param insteon_message
 * @return 
 */
bool InsteonProtocol::aldbRecord(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + count + 2)
        return false;

    PropertyKeys properties;
    uint8_t message_id = data[offset];
    properties["db_address_MSB"] = data[offset + count++];
    properties["db_address_LSB"] = data[offset + count++];

    InsteonMessageType message_type = InsteonMessageType::ALDBRecord;
    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
    return true;
}

bool
InsteonProtocol::decodeLinkRecord(const std::vector<uint8_t>& data, uint32_t offset,
        uint32_t& count, PropertyKeys& properties) {
    if (data.size() < offset + count + 8) return false;
    properties["link_type"] = data[offset + count++];
    properties["link_group"] = data[offset + count++];
    getAddressProperty("address", data, offset, count, properties);
    properties["link_data_one"] = data[offset + count++];
    properties["link_data_two"] = data[offset + count++];
    properties["link_data_three"] = data[offset + count++];
    return true;
}

/**
 * GetIMInfo 0x60
 * @param data
 * @param offset
 * @param count
 * @param insteon_message
 * @return 
 */
bool
InsteonProtocol::getIMInfo(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count,
        std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + count + 6) return false;

    uint8_t message_id = data[offset];

    PropertyKeys properties;
    properties["address"] = data[offset + 1] << 16 | data[offset + count++]
            << 8 | data[offset + count++];
    properties["device_category"] = data[offset + count++];
    properties["device_subcategory"] = data[offset + count++];
    properties["device_firmware_version"] = data[offset + count++];

    InsteonMessageType message_type = InsteonMessageType::GetIMInfo;
    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
    return true;
}

/**
 * GetIMConfiguration 0x73
 * @param data
 * @param offset
 * @param count
 * @param insteon_message
 * @return 
 */
bool
InsteonProtocol::getIMConfiguration(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + count + 3) return false;
    uint8_t message_id = data[offset];

    PropertyKeys properties;
    properties["im_configuration_flags"] = data[offset + count++];
    properties["spare_one"] = data[offset + count++];
    properties["spare_two"] = data[offset + count++];

    InsteonMessageType message_type = InsteonMessageType::GetIMConfiguration;
    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
    return true;
}

/**
 * directMessage 0x62
 * @param data
 * @param offset
 * @param count
 * @param insteon_message
 * @return 
 */
bool
InsteonProtocol::directMessage(const std::vector<uint8_t>& data,
        uint32_t offset, uint32_t& count, std::shared_ptr<InsteonMessage>& insteon_message) {
    if (data.size() < offset + count + 7) return false;
    uint8_t message_id = data[offset];

    PropertyKeys properties;
    
    if (!getAddressProperty("from_address", data, offset, count, properties))
        return false;
    
    if (!getMessageFlagProperty(data, offset, count, properties)) return false;
    
    properties["command_one"] = data[offset + count++];
    properties["command_two"] = data[offset + count++];

    if (properties.find("message_flags_extended")->second == 1) {
        if (data.size() < offset + count + 14) return false;
        properties["data_one"] = data[offset + count++];
        properties["data_two"] = data[offset + count++];
        properties["data_three"] = data[offset + count++];
        properties["data_four"] = data[offset + count++];
        properties["data_five"] = data[offset + count++];
        properties["data_six"] = data[offset + count++];
        properties["data_seven"] = data[offset + count++];
        properties["data_eight"] = data[offset + count++];
        properties["data_nine"] = data[offset + count++];
        properties["data_ten"] = data[offset + count++];
        properties["data_eleven"] = data[offset + count++];
        properties["data_twelve"] = data[offset + count++];
        properties["data_thirteen"] = data[offset + count++];
        properties["data_fourteen"] = data[offset + count++];
    }

    InsteonMessageType message_type = InsteonMessageType::DirectMessage;
    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
    return true;
}
} // namespace insteon
} // namespace ace