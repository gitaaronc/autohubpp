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
#ifndef INSTEONADDRESS_H
#define	INSTEONADDRESS_H

namespace ace {
    namespace insteon {
        struct InsteonAddress{
            InsteonAddress()
            : address_high_(0), address_middle_(0), address_low_(0){
            }
            unsigned char address_high_ : 8;
            unsigned char address_middle_ : 8;
            unsigned char address_low_ : 8;
            void setAddress(int value){
                address_high_ = value >> 16 & 0xFF;
                address_middle_ = value >> 8 & 0xFF;
                address_low_ = value & 0xFF;
            }
            int getAddress(){
                return address_high_ << 16 | address_middle_ << 8 | address_low_;
            }
        };
        
        struct InsteonIdentity{
            unsigned char category : 8;
            unsigned char sub_category : 8;
            unsigned char firmware_version : 8;
            InsteonIdentity()
            : category(0), sub_category(0), firmware_version(0){}
            void setIdentity(int value){
                category = value >> 16 & 0xFF;
                sub_category = value >> 8 & 0xFF;
                firmware_version = value & 0xFF;
            }
            int getIdentity(){
                return category << 16 | sub_category << 8 | firmware_version;
            }
        };
    }
}

#endif	/* INSTEONADDRESS_H */

