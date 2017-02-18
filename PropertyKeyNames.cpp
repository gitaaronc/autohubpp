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

        PropertyKeyNames_deprecated::PropertyKeyNames_deprecated() {
            property_name_[PropertyKey_deprecated::Address] = "address";
            property_name_[PropertyKey_deprecated::ButtonOnLevel] = "button_on_level";
            property_name_[PropertyKey_deprecated::ButtonRampRate] = "button_on_ramp_rate";
            property_name_[PropertyKey_deprecated::Cmd1] = "command_one";
            property_name_[PropertyKey_deprecated::Cmd2] = "command_two";
            property_name_[PropertyKey_deprecated::Data1] = "data_one";
            property_name_[PropertyKey_deprecated::Data10] = "data_ten";
            property_name_[PropertyKey_deprecated::Data11] = "data_eleven";
            property_name_[PropertyKey_deprecated::Data12] = "data_twelve";
            property_name_[PropertyKey_deprecated::Data13] = "data_thirteen";
            property_name_[PropertyKey_deprecated::Data14] = "data_fourteen";
            property_name_[PropertyKey_deprecated::Data2] = "data_two";
            property_name_[PropertyKey_deprecated::Data3] = "data_three";
            property_name_[PropertyKey_deprecated::Data4] = "data_four";
            property_name_[PropertyKey_deprecated::Data5] = "data_five";
            property_name_[PropertyKey_deprecated::Data6] = "data_six";
            property_name_[PropertyKey_deprecated::Data7] = "data_seven";
            property_name_[PropertyKey_deprecated::Data8] = "data_eight";
            property_name_[PropertyKey_deprecated::Data9] = "data_nine";
            property_name_[PropertyKey_deprecated::DevCat] = "device_category";
            property_name_[PropertyKey_deprecated::DevEngineVersion] 
                    = "device_engine_version";
            property_name_[PropertyKey_deprecated::DevFirmwareVersion] 
                    = "device_firmware_version";
            property_name_[PropertyKey_deprecated::DevSubCat] = "device_subcategory";
            property_name_[PropertyKey_deprecated::EnableBeepOnButtonPress] 
                    = "enable_beep_on_button_press";
            property_name_[PropertyKey_deprecated::EnableBlinkOnError] 
                    = "enable_blink_on_error";
            property_name_[PropertyKey_deprecated::EnableBlinkOnTraffic] 
                    = "enable_blink_on_traffic";
            property_name_[PropertyKey_deprecated::EnableLED] = "enable_led";
            property_name_[PropertyKey_deprecated::EnableProgrammingLock] 
                    = "enable_programming_lock";
            property_name_[PropertyKey_deprecated::EnableResumeDim] 
                    = "enable_resume_dim";
            property_name_[PropertyKey_deprecated::EnableLoadSense] 
                    = "enable_load_sense";
            property_name_[PropertyKey_deprecated::FromAddress] = "from_address";
            property_name_[PropertyKey_deprecated::GlobalLedBrightness] 
                    = "global_led_brightness";
            property_name_[PropertyKey_deprecated::Group] = "group";
            property_name_[PropertyKey_deprecated::IMConfigurationFlags] 
                    = "im_configuration_flags";
            property_name_[PropertyKey_deprecated::IMSetButtonEvent]
                    = "im_set_button_event";
            property_name_[PropertyKey_deprecated::IncrementDirection] 
                    = "increment_direction";
            property_name_[PropertyKey_deprecated::LightStatus] = "light_status";
            property_name_[PropertyKey_deprecated::LinkAddress] = "link_address";
            property_name_[PropertyKey_deprecated::LinkData1] = "link_data_one";
            property_name_[PropertyKey_deprecated::LinkData2] = "link_data_two";
            property_name_[PropertyKey_deprecated::LinkData3] = "link_data_three";
            property_name_[PropertyKey_deprecated::LinkDatabaseDelta] 
                    = "link_database_delta";
            property_name_[PropertyKey_deprecated::LinkGroup] = "link_group";
            property_name_[PropertyKey_deprecated::LinkRecordFlags] = "link_record_flags";
            property_name_[PropertyKey_deprecated::LinkStatus] = "link_status";
            property_name_[PropertyKey_deprecated::LinkType] = "link_type";
            property_name_[PropertyKey_deprecated::MessageFlagsAcknowledge] 
                    = "message_flags_ack";
            property_name_[PropertyKey_deprecated::MessageFlagsGroup] 
                    = "message_flags_group";
            property_name_[PropertyKey_deprecated::MessageFlagsBroadcast] 
                    = "message_flags_broadcast";
            property_name_[PropertyKey_deprecated::MessageFlagsExtendedFlag] 
                    = "message_flags_extended";
            property_name_[PropertyKey_deprecated::MessageFlagsMaxHops] = "message_flags_max_hops";
            property_name_[PropertyKey_deprecated::MessageFlagsRemainingHops] 
                    = "message_Flags_remaining_hops";
            property_name_[PropertyKey_deprecated::ResponderCmd1] = "responder_command_one";
            property_name_[PropertyKey_deprecated::ResponderCount] = "responder_count";
            property_name_[PropertyKey_deprecated::ResponderErrorCount] 
                    = "responder_error_count";
            property_name_[PropertyKey_deprecated::ResponderGroup] = "responder_group";
            property_name_[PropertyKey_deprecated::Spare1] = "spare_one";
            property_name_[PropertyKey_deprecated::Spare2] = "spare_two";
            property_name_[PropertyKey_deprecated::ToAddress] = "to_address";
            property_name_[PropertyKey_deprecated::SentCommandOne] = 
                    "sent_command_one"; 
            property_name_[PropertyKey_deprecated::SentCommandTwo] = 
                    "sent_command_two"; 
            property_name_[PropertyKey_deprecated::SignalToNoiseThreshold] = 
                    "signal_to_noise_threshold";
            property_name_[PropertyKey_deprecated::X10HouseCode] = "x10_house_code";
            property_name_[PropertyKey_deprecated::X10UnitCode] = "x10_unit_code";

        }

        /**
         * GetPropertyName
         * 
         * Returns a string description of the PropertyKey
         * 
         * @param key   PropertyKey to get the description of
         * @return      returns a std::string containing the description
         */
        std::string PropertyKeyNames_deprecated::GetPropertyName(PropertyKey_deprecated key) {
            auto it = property_name_.find(key);
            if (it != property_name_.end())
                return it->second;
            return "ERROR: PropertyKey is not mapped to PropertyName";
        }
    } // namespace insteon
} // namespace ace