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
#include "include/system/AutoResetEvent.hpp"

namespace ace {
    namespace system {
        AutoResetEvent::AutoResetEvent(bool initial)
                : flag_(initial){
            
        }
        
        /**
         * Set
         * 
         * Sets the state of the event to signaled, allowing
         * one or more thread to be processed.
         */
        void AutoResetEvent::Set(){
            std::lock_guard<std::mutex> _(mutex_);
            flag_ = true;
            signal_.notify_one();
        }
        
        /**
         * Reset
         * 
         * Sets the state of the event to nonsignaled, causing
         * threads to block.
         */
        void AutoResetEvent::Reset(){
            std::lock_guard<std::mutex> _(mutex_);
            flag_ = false;
        }
        
        /**
         * WaitOne
         * 
         * Causes thread to be blocked indefinitely or until
         * an event is signaled.
         * 
         * @return boolean value
         * 
         * Will not return unless a call to Set is made, causing
         * the event to be signaled. Will only return true.
         */
        bool AutoResetEvent::WaitOne(){
            std::unique_lock<std::mutex> lock(mutex_);
            while (!flag_)
                signal_.wait(lock);
            flag_ = false;
            return true;
        }
        
        /**
         * WaitOne
         * 
         * Causes the current thread to block until an event is signaled or
         * time specified in milleseconds has passed
         * 
         * @param milliseconds:int      The number of milliseconds to wait
         * @return boolean
         * 
         * True if event was signaled by call to Set
         * False if event was signaled by timed wait
         */
        bool AutoResetEvent::WaitOne(uint32_t milliseconds){
            std::unique_lock<std::mutex>lock(mutex_);
            flag_ = false;
            return signal_.wait_for(lock, std::chrono::milliseconds(milliseconds), 
                    [this](){return flag_ == true;});
        }
        
    } // namespace system
} // namespace autohub
