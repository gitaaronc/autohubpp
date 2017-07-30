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
#ifndef INSTEONCONTROLLER_IMPL_H
#define	INSTEONCONTROLLER_IMPL_H

#include "../InsteonAddress.h"
#include "../InsteonLinkMode.h"

#include "../../system/Timer.hpp"

#include <memory>
#include <cstdint>

namespace ace {
    namespace system{
        class Timer;
    }
    namespace insteon {
        namespace detail {

            struct InsteonController_impl {

                InsteonController_impl()
                : IsInLinkingMode_(false),
                LinkingMode_(InsteonLinkMode::Contoller){
                }

                InsteonAddress insteon_address_;
                InsteonIdentity insteon_identity_;
                std::unique_ptr<system::Timer> timer_;
                InsteonLinkMode LinkingMode_ ;
                bool IsInLinkingMode_;
            };
        } // namespace detail
    } // namespace network
} // namespace autohub


#endif	/* INSTEONCONTROLLER_IMPL_H */

