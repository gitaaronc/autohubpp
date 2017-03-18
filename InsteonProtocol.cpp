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

namespace ace {
namespace insteon {

InsteonProtocol::InsteonProtocol() {
}

InsteonProtocol::~InsteonProtocol() {

}

bool
InsteonProtocol::GetAddressProperty(const std::string key,
        const std::vector<unsigned char>&data, int offset, int& count,
        PropertyKeys& properties) {
    if (data.size() < offset + 3)
        return false;
    int address = data[offset] << 16 | data[offset + 1] << 8 | data[offset + 2];
    properties[key] = address;
    count += 3;
    return true;
}

bool
InsteonProtocol::GetMessageFlagProperty(const std::vector<unsigned char>& data,
        int offset, int& count, PropertyKeys& properties) {
    if (data.size() < offset + 1)
        return false;

    unsigned char messageFlags = data[offset];

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

    count += 1;
    return true;
}

/* GetMessageType
 * 
 * Responsible for decoding the Insteon Message Type
 * eg: Broadcast, All Linking, ACK, NAK
 */
InsteonMessageType
InsteonProtocol::GetMessageType(const std::vector<unsigned char>& data, 
        int offset, PropertyKeys& properties) {
    //0250/26deeb/000001/cb/14/00
    
    unsigned char cmd1 = properties["command_one"];
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
InsteonProtocol::ProcessMessage(const std::vector<unsigned char>& data,
        int offset, int& count, std::shared_ptr<InsteonMessage>& insteon_message) {
    count = 1;
    switch (data[offset]) {
        case 0x50: // receive standard message
            return StandardMessage(data, offset, count, insteon_message);
        case 0x51: // receive extended message
            return ExtendedMessage(data, offset, count, insteon_message);
        case 0x53: // receive all linking complete
            return DeviceLinkMessage(data, offset, count, insteon_message);
        case 0x54: // IM set button pressed
            return IMSetButtonEvent(data, offset, count, insteon_message);
        case 0x57: // receive all link record response
            return DeviceLinkRecordMessage(data, offset, count, insteon_message);
        case 0x58: // receive all link cleanup response
            return DeviceLinkCleanupMessage(data, offset, count, insteon_message);
        case 0x59: // receive database record found
            return ALDBRecord(data, offset, count, insteon_message);
        case 0x60: // get insteon modem info
            return GetIMInfo(data, offset, count, insteon_message);
        case 0x61:
            if (data.size() < offset + count + 3)
                return false;
            count += 3;
            return true;
        case 0x62:
            if (data.size() < offset + count + 6)
                return false;
            count += 6;
            return true;
        case 0x65:
            if (data.size() < offset + count)
                return false;
            return true;
        case 0x73: // get insteon modem configuration
            return GetIMConfiguration(data, offset, count, insteon_message);
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
InsteonProtocol::StandardMessage(const std::vector<unsigned char>& data,
        int offset, int& count,
        std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + count + 9)
        return false;

    unsigned char message_id = data[offset];

    PropertyKeys properties;
    GetAddressProperty("from_address", data, offset + 1, count, properties);
    GetMessageFlagProperty(data, offset + 7, count, properties);
    if (properties.find("message_flags_group")->second
            == 0) {
        GetAddressProperty("to_address", data, offset + 4, count, 
                properties);
    } else {
        count += 3;
    }
    properties["command_one"] = data[offset + 8];
    properties["command_two"] = data[offset + 9];
    count += 2; 

    InsteonMessageType message_type = GetMessageType(data, offset, properties);
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
InsteonProtocol::ExtendedMessage(const std::vector<unsigned char>& data,
        int offset, int& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (!StandardMessage(data, offset, count, insteon_message))
        return false;

    if (data.size() < offset + count + 14) return false;
    insteon_message->properties_["data_one"] = data[offset + 10];
    insteon_message->properties_["data_two"] = data[offset + 11];
    insteon_message->properties_["data_three"] = data[offset + 12];
    insteon_message->properties_["data_four"] = data[offset + 13];
    insteon_message->properties_["data_five"] = data[offset + 14];
    insteon_message->properties_["data_six"] = data[offset + 15];
    insteon_message->properties_["data_seven"] = data[offset + 16];
    count += 7;
    if (insteon_message->message_type_ == InsteonMessageType::DirectMessage
            && insteon_message->properties_["command_one"] == 0x2F){
        GetAddressProperty("ext_link_address", data, offset + 17, count, 
                insteon_message->properties_ );
    } else {
        insteon_message->properties_["data_eight"] = data[offset + 17];
        insteon_message->properties_["data_nine"] = data[offset + 18];
        insteon_message->properties_["data_ten"] = data[offset + 19];
        count += 3;
    }
    insteon_message->properties_["data_eleven"] = data[offset + 20];
    insteon_message->properties_["data_twelve"] = data[offset + 21];
    insteon_message->properties_["data_thirteen"] = data[offset + 22];
    insteon_message->properties_["data_fourteen"] = data[offset + 23];
    count += 4;

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
InsteonProtocol::DeviceLinkMessage(const std::vector<unsigned char>& data,
        int offset, int& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < count + offset + 8)
        return false;

    unsigned char message_id = data[offset];
    
    PropertyKeys properties;
    properties["link_type"] = data[offset + 1];
    properties["link_group"] = data[offset + 2];
    count += 2;
    GetAddressProperty("address", data, offset + 3, count, properties);
    properties["device_category"] = data[offset + 6];
    properties["device_subcategory"] = data[offset + 7];
    properties["device_firmware_version"] = data[offset + 8];
    count += 3;

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
InsteonProtocol::IMSetButtonEvent(const std::vector<unsigned char>& data, 
        int offset, int& count, std::shared_ptr<InsteonMessage>& insteon_message){

    if (data.size() < offset + count + 1) return false;
    
    unsigned char message_id = data[offset];
    
    PropertyKeys properties;
    properties["im_set_button_event"] = data[offset + 1];
    count += 1;
    
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
InsteonProtocol::DeviceLinkRecordMessage(const std::vector<unsigned char>& data,
        int offset, int& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + 8)
        return false;

    unsigned char message_id = data[offset];

    PropertyKeys properties;
    properties["link_record_flags"] = data[offset + 1];
    properties["link_group"] = data[offset + 2];
    count += 2;
    if (!GetAddressProperty("link_address", data, offset + 3, count, properties))
        return false; // count += 3
    properties["link_data_one"] = data[offset + 6];
    properties["link_data_two"] = data[offset + 7];
    properties["link_data_three"] = data[offset + 8];
    count += 3;

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
InsteonProtocol::DeviceLinkCleanupMessage(const std::vector<unsigned char>& data,
        int offset, int& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + count + 1)
        return false;

    PropertyKeys properties;
    unsigned char message_id = data[offset];

    properties["link_status"] = data[offset + 1];
    count += 1;

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
bool InsteonProtocol::ALDBRecord(const std::vector<unsigned char>& data,
        int offset, int& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + count + 2)
        return false;

    PropertyKeys properties;
    unsigned char message_id = data[offset];
    properties["db_address_MSB"] = data[offset + 1];
    properties["db_address_LSB"] = data[offset + 2];
    count += 2;
    if (!DeviceLinkRecordMessage(data, offset + 2, count, insteon_message))
        return false;
    for (const auto& it : insteon_message->properties_){
        properties[it.first] = it.second;
    }
    InsteonMessageType message_type = InsteonMessageType::ALDBRecord;
    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
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
InsteonProtocol::GetIMInfo(const std::vector<unsigned char>& data,
        int offset, int& count,
        std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + count + 6) return false;

    unsigned char message_id = data[offset];

    PropertyKeys properties;
    properties["address"] = data[offset + 1] << 16 | data[offset + 2]
            << 8 | data[offset + 3];
    properties["device_category"] = data[offset + 4];
    properties["device_subcategory"] = data[offset + 5];
    properties["device_firmware_version"] = data[offset + 6];
    count += 6;

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
InsteonProtocol::GetIMConfiguration(const std::vector<unsigned char>& data,
        int offset, int& count, std::shared_ptr<InsteonMessage>& insteon_message) {

    if (data.size() < offset + count + 3) return false;
    unsigned char message_id = data[offset];

    PropertyKeys properties;
    properties["im_configuration_flags"] = data[offset + 1];
    properties["spare_one"] = data[offset + 2];
    properties["spare_two"] = data[offset + 3];
    count += 3;

    InsteonMessageType message_type = InsteonMessageType::GetIMConfiguration;
    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
    return true;
}

} // namespace insteon
} // namespace ace