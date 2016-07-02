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

/* ProcessMessage
 * Decodes all Insteon Messages into PropertyKeys(PropertyKey, value) PropertyKey.h
 */
bool
InsteonProtocol::ProcessMessage(const std::vector<unsigned char>& data,
        int offset, int& count, std::shared_ptr<InsteonMessage>& insteon_message) {
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
        case 0x60: // get insteon modem info
            return GetIMInfo(data, offset, count, insteon_message);
        case 0x73: // get insteon modem configuration
            return GetIMConfiguration(data, offset, count, insteon_message);
    }
    count = 0;
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
    count = 0;
    if (data.size() < offset + 10)
        return false;

    unsigned char message_id = data[offset];
    PropertyKeys properties;

    GetAddressProperty(PropertyKey::FromAddress, data, offset + 1, count, properties);
    GetMessageFlagProperty(data, offset, count, properties);
    if (properties.find(PropertyKey::MessageFlagsGroup)->second
            == 0) {
        GetAddressProperty(PropertyKey::ToAddress, data, offset + 4, count, 
                properties);
    }
    properties[PropertyKey::Cmd1] = data[offset + 8];
    properties[PropertyKey::Cmd2] = data[offset + 9];
    count = 10;

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

    count = 0;
    if (data.size() < offset + 24) return false;

    StandardMessage(data, offset, count, insteon_message);
    insteon_message->properties_[PropertyKey::Data1] = data[offset + 10];
    insteon_message->properties_[PropertyKey::Data2] = data[offset + 11];
    insteon_message->properties_[PropertyKey::Data3] = data[offset + 12];
    insteon_message->properties_[PropertyKey::Data4] = data[offset + 13];
    insteon_message->properties_[PropertyKey::Data5] = data[offset + 14];
    insteon_message->properties_[PropertyKey::Data6] = data[offset + 15];
    insteon_message->properties_[PropertyKey::Data7] = data[offset + 16];
    insteon_message->properties_[PropertyKey::Data8] = data[offset + 17];
    insteon_message->properties_[PropertyKey::Data9] = data[offset + 18];
    insteon_message->properties_[PropertyKey::Data10] = data[offset + 19];
    insteon_message->properties_[PropertyKey::Data11] = data[offset + 20];
    insteon_message->properties_[PropertyKey::Data12] = data[offset + 21];
    insteon_message->properties_[PropertyKey::Data13] = data[offset + 22];
    insteon_message->properties_[PropertyKey::Data14] = data[offset + 23];
    count = 24;
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

    count = 0;
    if (data.size() < offset + 9)
        return false;

    unsigned char message_id = data[offset];
    InsteonMessageType message_type = InsteonMessageType::DeviceLink;
    PropertyKeys properties;
    properties[PropertyKey::LinkType] = data[offset + 1];
    properties[PropertyKey::LinkGroup] = data[offset + 2];
    GetAddressProperty(PropertyKey::Address, data, offset + 3, count, properties);
    properties[PropertyKey::DevCat] = data[offset + 6];
    properties[PropertyKey::DevSubCat] = data[offset + 7];
    properties[PropertyKey::DevFirmwareVersion] = data[offset + 8];
    count = 9;

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
    count = 0;
    if (data.size() <= offset + 9)
        return false;

    PropertyKeys properties;
    unsigned char message_id = data[offset];
    InsteonMessageType message_type = InsteonMessageType::DeviceLinkRecord;

    properties[PropertyKey::LinkRecordFlags] = data[offset + 1];
    properties[PropertyKey::LinkGroup] = data[offset + 2];
    properties[PropertyKey::LinkAddress] = data[offset + 3] << 16
            | data[offset + 4] << 8 | data[offset + 5];
    properties[PropertyKey::LinkData1] = data[offset + 6];
    properties[PropertyKey::LinkData2] = data[offset + 7];
    properties[PropertyKey::LinkData3] = data[offset + 8];
    count = 9;

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

    count = 0;
    if (data.size() <= offset + 2)
        return false;

    PropertyKeys properties;
    unsigned char message_id = data[offset];
    InsteonMessageType message_type = InsteonMessageType::DeviceLinkCleanup;

    properties[PropertyKey::LinkStatus] = data[offset + 1];
    count = 2;

    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));
    return true;
}

bool
InsteonProtocol::GetAddressProperty(PropertyKey key,
        const std::vector<unsigned char>&data, int offset, int& count,
        PropertyKeys& properties) {

    count = 0;
    if (data.size() <= offset + 2)
        return false;
    int address = data[offset + 0] << 16 | data[offset + 1] << 8 | data[offset + 2];
    properties[key] = address;
    count = 2;
    return true;
}

bool
InsteonProtocol::GetMessageFlagProperty(const std::vector<unsigned char>& data,
        int offset, int& count, PropertyKeys& properties) {

    count = 0;
    if (data.size() <= offset + 7)
        return false;

    unsigned char messageFlags = data[offset + 7];

    properties[PropertyKey::MessageFlagsMaxHops] = messageFlags & 0b00000011;
    properties[PropertyKey::MessageFlagsRemainingHops] = 
            (messageFlags & 0b00001100) >> 2;
    properties[PropertyKey::MessageFlagsExtendedFlag] = 
            (messageFlags & 0b00010000) >> 4;
    properties[PropertyKey::MessageFlagsAcknowledge] =
            (messageFlags & 0b00100000) >> 5; //
    properties[PropertyKey::MessageFlagsGroup] =
            (messageFlags & 0b01000000) >> 6; //
    properties[PropertyKey::MessageFlagsBroadcast] =
            (messageFlags & 0b10000000) >> 7; //

    count = 7;
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
    
    unsigned char cmd1 = properties[PropertyKey::Cmd1];
    bool broadcast = properties[PropertyKey::MessageFlagsBroadcast] != 0; //bit7
    bool group = properties[PropertyKey::MessageFlagsGroup] != 0; //bit6
    bool ack = properties[PropertyKey::MessageFlagsAcknowledge] != 0; //bit5

    InsteonMessageType message_type = InsteonMessageType::Other;
    if (ack) {
        message_type = InsteonMessageType::Ack;
    } else if (cmd1 == 0x06 && broadcast && group) {
        message_type = InsteonMessageType::SuccessBroadcast;
        properties[PropertyKey::ResponderCmd1] = data[offset + 4];
        properties[PropertyKey::ResponderCount] = data[offset + 5];
        properties[PropertyKey::ResponderGroup] = data[offset + 6];
        properties[PropertyKey::ResponderErrorCount] = data[offset + 9];
    } else if (cmd1 == 0x11 && broadcast && group) {
        message_type = InsteonMessageType::OnBroadcast;
        properties[PropertyKey::Group] = data[offset + 5];
    } else if (cmd1 == 0x11 && !broadcast && group) {
        message_type = InsteonMessageType::OnCleanup;
        properties[PropertyKey::Group] = data[offset + 9];
    } else if (cmd1 == 0x13 && broadcast && group) {
        message_type = InsteonMessageType::OffBroadcast;
        properties[PropertyKey::Group] = data[offset + 5];
    } else if (cmd1 == 0x13 && !broadcast && group) {
        message_type = InsteonMessageType::OffCleanup;
        properties[PropertyKey::Group] = data[offset + 9];
    } else if (cmd1 == 0x12 && broadcast && group) {
        message_type = InsteonMessageType::FastOnBroadcast;
        properties[PropertyKey::Group] = data[offset + 5];
    } else if (cmd1 == 0x12 && !broadcast && group) {
        message_type = InsteonMessageType::FastOnCleanup;
        properties[PropertyKey::Group] = data[offset + 9];
    } else if (cmd1 == 0x14 && broadcast && group) {
        message_type = InsteonMessageType::FastOffBroadcast;
        properties[PropertyKey::Group] = data[offset + 5];
    } else if (cmd1 == 0x14 && !broadcast && group) {
        message_type = InsteonMessageType::FastOffCleanup;
        properties[PropertyKey::Group] = data[offset + 9];
    } else if (cmd1 == 0x17 && broadcast && group) {
        message_type = InsteonMessageType::IncrementBeginBroadcast;
        properties[PropertyKey::Group] = data[offset + 5];
        properties[PropertyKey::IncrementDirection] = data[offset + 9];
    } else if (cmd1 == 0x18 && broadcast && group) {
        message_type = InsteonMessageType::IncrementEndBroadcast;
        properties[PropertyKey::Group] = data[offset + 5];
    } else if (cmd1 == 0x01 || cmd1 == 0x02) {
        message_type = InsteonMessageType::SetButtonPressed;
        properties[PropertyKey::DevCat] = data[offset + 4];
        properties[PropertyKey::DevSubCat] = data[offset + 5];
        properties[PropertyKey::DevFirmwareVersion] = data[offset + 6];
    }
    return message_type;
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

    count = 0;
    if (data.size() < offset + 5) return false;
    unsigned char message_id = data[offset];
    InsteonMessageType message_type = InsteonMessageType::GetIMConfiguration;
    PropertyKeys properties;
    properties[PropertyKey::IMConfigurationFlags] = data[offset + 1];
    properties[PropertyKey::Spare1] = data[offset + 2];
    properties[PropertyKey::Spare2] = data[offset + 3];
    count = 5;

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
    count = 0;
    if (data.size() < offset + 2) return false;
    
    unsigned char message_id = data[offset];
    InsteonMessageType message_type = InsteonMessageType::SetButtonPressed;
    PropertyKeys properties;
    properties[PropertyKey::IMSetButtonEvent] = data[offset + 1];
    count = 2;
    
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

    count = 0;
    if (data.size() < offset + 8) return false;

    unsigned char message_id = data[offset];
    InsteonMessageType message_type = InsteonMessageType::GetIMInfo;
    PropertyKeys properties;
    properties[PropertyKey::Address] = data[offset + 1] << 16 | data[offset + 2]
            << 8 | data[offset + 3];
    properties[PropertyKey::DevCat] = data[offset + 4];
    properties[PropertyKey::DevSubCat] = data[offset + 5];
    properties[PropertyKey::DevFirmwareVersion] = data[offset + 6];
    count = 8;

    insteon_message.reset(new InsteonMessage(message_id, message_type, properties));

    return true;
}
} // namespace insteon
} // namespace ace