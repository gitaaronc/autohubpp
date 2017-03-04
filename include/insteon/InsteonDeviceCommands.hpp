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

#ifndef INSTEONDEVICECOMMANDS_H
#define	INSTEONDEVICECOMMANDS_H

namespace ace {
namespace insteon {
// A set of commands that can be sent to an INSTEON device

enum class InsteonDeviceCommand_ : unsigned char {
    //Reserved = 0x00,
    ProductDataRequest = 0x03,
            EnterLinkingMode = 0x09,
            EnterUnlinkingMode = 0x0A,
            GetInsteonEngineVersion = 0x0D,
            Ping = 0x0F,
            IDRequest = 0x10,
            On = 0x11,
            FastOn = 0x12,
            Off = 0x13,
            FastOff = 0x14,
            Brighten = 0x15,
            Dim = 0x16,
            StartDimming = 0x17,
            StopDimming = 0x18,
            LightStatusRequest = 0x19,
            Beep = 0x30,
            GetOperatingFlags = 0x1F,
            ExtendedGetSet = 0x2E,
            ALDBReadWrite = 0x2F,
            GetAllProperties = 0xFF,
};

typedef InsteonDeviceCommand_ InsteonDeviceCommand;

}
}

#endif	/* INSTEONDEVICECOMMANDS_H */

