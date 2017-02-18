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

#ifndef PROPERTYKEY_HPP
#define	PROPERTYKEY_HPP

#include <vector>
#include <map>

namespace ace {
    namespace insteon {

        // TODO - REMOVE DEPRECATED in favor of map<string,int> vs map<int,int>
        enum class PropertyKey_ : int {
            Address = 1,
                    ButtonRampRate,
                    ButtonOnLevel,
                    Cmd1,
                    Cmd2,
                    Data1,
                    Data2,
                    Data3,
                    Data4,
                    Data5,
                    Data6,
                    Data7,
                    Data8,
                    Data9,
                    Data10,
                    Data11,
                    Data12,
                    Data13,
                    Data14,
                    DevCat,
                    DevEngineVersion,
                    DevFirmwareVersion,
                    DevSubCat,
                    // ---- Responses to 0x1f00 
                    EnableBeepOnButtonPress,
                    EnableBlinkOnError, 
                    EnableBlinkOnTraffic,
                    EnableLED, 
                    EnableProgrammingLock,
                    EnableResumeDim, 
                    EnableLoadSense, 
                    EnableUnknown3,
                    // -----------------------
                    FromAddress,
                    GlobalLedBrightness,
                    Group, // 
                    IMConfigurationFlags,
                    IMSetButtonEvent,
                    IncrementDirection,
                    LightStatus, // current status of relay ON | OFF
                    LinkAddress,
                    LinkData1,
                    LinkData2,
                    LinkData3,
                    LinkDatabaseDelta,
                    LinkGroup,
                    LinkRecordFlags,
                    LinkStatus, //
                    LinkType,
                    MessageFlagsMaxHops,
                    MessageFlagsRemainingHops,
                    MessageFlagsExtendedFlag,
                    MessageFlagsAcknowledge,
                    MessageFlagsGroup,
                    MessageFlagsBroadcast,
                    ResponderCmd1,
                    ResponderCount,
                    ResponderGroup, //
                    ResponderErrorCount,
                    ToAddress,
                    SentCommandOne,
                    SentCommandTwo,
                    SignalToNoiseThreshold,
                    Spare1,
                    Spare2,
                    X10HouseCode,
                    X10UnitCode//
        };

        typedef PropertyKey_ PropertyKey_deprecated; // TODO - REMOVE DEPRECATED
        /**
         * mapPropertyKeyInt
         * 
         * std::map<std::pair(PropertyKey, unsigned int)>
         * 
         * @param PropertyKey           The Property Key
         * @param unsigned int         The integer value 
         */
        // TODO - REMOVE DEPRECATED
        typedef std::map<PropertyKey_deprecated, unsigned int> PropertyKeys_deprecated;
        
        // PropertyKeys string as Key, more room for error vs int
        typedef std::map<std::string, unsigned int> PropertyKeys; 
        
        // TODO - REMOVE DEPRECATED
        class PropertyKeyNames_deprecated {
        public:
            PropertyKeyNames_deprecated();
            std::string GetPropertyName(PropertyKey_deprecated key);
        private:
            std::map<PropertyKey_deprecated, std::string> property_name_;
        };
    } // namespace insteon
} // namespace ace
#endif	/* PROPERTYKEY_HPP */

