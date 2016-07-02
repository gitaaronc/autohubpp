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
#include "include/insteon/PropertyKey.hpp"

namespace ace {
    namespace insteon {

        PropertyKeyNames::PropertyKeyNames() {
            property_name_[PropertyKey::Address] = "address";
            property_name_[PropertyKey::ButtonOnLevel] = "button_on_level";
            property_name_[PropertyKey::ButtonRampRate] = "button_on_ramp_rate";
            property_name_[PropertyKey::Cmd1] = "command_one";
            property_name_[PropertyKey::Cmd2] = "command_two";
            property_name_[PropertyKey::Data1] = "data_one";
            property_name_[PropertyKey::Data10] = "data_ten";
            property_name_[PropertyKey::Data11] = "data_eleven";
            property_name_[PropertyKey::Data12] = "data_twelve";
            property_name_[PropertyKey::Data13] = "data_thirteen";
            property_name_[PropertyKey::Data14] = "data_fourteen";
            property_name_[PropertyKey::Data2] = "data_two";
            property_name_[PropertyKey::Data3] = "data_three";
            property_name_[PropertyKey::Data4] = "data_four";
            property_name_[PropertyKey::Data5] = "data_five";
            property_name_[PropertyKey::Data6] = "data_six";
            property_name_[PropertyKey::Data7] = "data_seven";
            property_name_[PropertyKey::Data8] = "data_eight";
            property_name_[PropertyKey::Data9] = "data_nine";
            property_name_[PropertyKey::DevCat] = "device_category";
            property_name_[PropertyKey::DevEngineVersion] 
                    = "device_engine_version";
            property_name_[PropertyKey::DevFirmwareVersion] 
                    = "device_firmware_version";
            property_name_[PropertyKey::DevSubCat] = "device_subcategory";
            property_name_[PropertyKey::EnableBeepOnButtonPress] 
                    = "enable_beep_on_button_press";
            property_name_[PropertyKey::EnableBlinkOnError] 
                    = "enable_blink_on_error";
            property_name_[PropertyKey::EnableBlinkOnTraffic] 
                    = "enable_blink_on_traffic";
            property_name_[PropertyKey::EnableLED] = "enable_led";
            property_name_[PropertyKey::EnableProgrammingLock] 
                    = "enable_programming_lock";
            property_name_[PropertyKey::EnableResumeDim] 
                    = "enable_resume_dim";
            property_name_[PropertyKey::EnableLoadSense] 
                    = "enable_load_sense";
            property_name_[PropertyKey::FromAddress] = "from_address";
            property_name_[PropertyKey::GlobalLedBrightness] 
                    = "global_led_brightness";
            property_name_[PropertyKey::Group] = "group";
            property_name_[PropertyKey::IMConfigurationFlags] 
                    = "im_configuration_flags";
            property_name_[PropertyKey::IMSetButtonEvent]
                    = "im_setbutton_event";
            property_name_[PropertyKey::IncrementDirection] 
                    = "increment_direction";
            property_name_[PropertyKey::LightStatus] = "light_status";
            property_name_[PropertyKey::LinkAddress] = "link_address";
            property_name_[PropertyKey::LinkData1] = "link_data_one";
            property_name_[PropertyKey::LinkData2] = "link_data_two";
            property_name_[PropertyKey::LinkData3] = "link_data_three";
            property_name_[PropertyKey::LinkDatabaseDelta] 
                    = "link_database_delta";
            property_name_[PropertyKey::LinkGroup] = "link_group";
            property_name_[PropertyKey::LinkRecordFlags] = "link_record_flags";
            property_name_[PropertyKey::LinkStatus] = "link_status";
            property_name_[PropertyKey::LinkType] = "link_type";
            property_name_[PropertyKey::MessageFlagsAcknowledge] 
                    = "message_flags_acknowledge";
            property_name_[PropertyKey::MessageFlagsGroup] 
                    = "message_flags_group";
            property_name_[PropertyKey::MessageFlagsBroadcast] 
                    = "message_flags_broadcast";
            property_name_[PropertyKey::MessageFlagsExtendedFlag] 
                    = "message_flags_extended";
            property_name_[PropertyKey::MessageFlagsMaxHops] = "max_hops";
            property_name_[PropertyKey::MessageFlagsRemainingHops] 
                    = "remaining_hops";
            property_name_[PropertyKey::ResponderCmd1] = "responder_command_one";
            property_name_[PropertyKey::ResponderCount] = "responder_count";
            property_name_[PropertyKey::ResponderErrorCount] 
                    = "responder_error_count";
            property_name_[PropertyKey::ResponderGroup] = "responder_group";
            property_name_[PropertyKey::Spare1] = "spare_one";
            property_name_[PropertyKey::Spare2] = "spare_two";
            property_name_[PropertyKey::ToAddress] = "to_address";
            property_name_[PropertyKey::SentCommandOne] = 
                    "sent_command_one"; 
            property_name_[PropertyKey::SentCommandTwo] = 
                    "sent_command_two"; 
            property_name_[PropertyKey::SignalToNoiseThreshold] = 
                    "signal_to_noise_threshold";
            property_name_[PropertyKey::X10HouseCode] = "x10_house_code";
            property_name_[PropertyKey::X10UnitCode] = "x10_unit_code";

        }

        /**
         * GetPropertyName
         * 
         * Returns a string description of the PropertyKey
         * 
         * @param key   PropertyKey to get the description of
         * @return      returns a std::string containing the description
         */
        std::string PropertyKeyNames::GetPropertyName(PropertyKey key) {
            auto it = property_name_.find(key);
            if (it != property_name_.end())
                return it->second;
            return "ERROR: PropertyKey is not mapped to PropertyName";
        }
    } // namespace insteon
} // namespace ace