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
#ifndef INSTEONCONTROLLER_H
#define	INSTEONCONTROLLER_H

#include "InsteonLinkMode.h"
#include "InsteonControllerGroupCommands.h"
#include "PropertyKey.hpp"

//#include "InsteonDevice.hpp"
//#include "InsteonMessage.hpp"

#include <boost/asio/io_service.hpp>

namespace ace {
namespace insteon {
class InsteonDevice;
class InsteonNetwork;
class InsteonMessage;
namespace detail {
class InsteonController_impl;
}

class InsteonController {
public:
    InsteonController() = delete;
    InsteonController(InsteonNetwork *network,
            boost::asio::io_service& io_service);
    ~InsteonController();
public:

    int GetAddress(); 

    void EnterLinkMode(InsteonLinkMode mode, unsigned char group);
    
    void CancelLinkMode();

    void GroupCommand(InsteonControllerGroupCommands command,
            unsigned char group);

    void GroupCommand(InsteonControllerGroupCommands command,
            unsigned char group, unsigned char value);
    
    void GetIMConfiguration();
    
    void OnMessage(std::shared_ptr<InsteonMessage>
            insteon_message);
    
private:
    void OnTimerEvent(); 

    void OnDeviceLinked(std::shared_ptr<InsteonDevice>& device); 

    void OnDeviceUnlinked(std::shared_ptr<InsteonDevice>& device); 

    void SetAddress(int address); 

    bool TryEnterLinkMode(InsteonLinkMode mode, unsigned char group); 

    bool TryCancelLinkMode(); 

    bool TryGroupCommand(InsteonControllerGroupCommands command,
            unsigned char group);

    bool TryGroupCommand(InsteonControllerGroupCommands command,
            unsigned char group, unsigned char value);

    std::unique_ptr<detail::InsteonController_impl> pImpl_;

    InsteonNetwork *insteon_network_;
    PropertyKeys controller_properties_;
};
} // namespace network
} // namespace ace

#endif	/* INSTEONCONTROLLER_H */

