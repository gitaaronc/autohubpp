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

#ifndef AUTORESETEVENT_H
#define	AUTORESETEVENT_H

#include <condition_variable>
#include <mutex>

namespace ace {
    namespace system {
        /*
         * AutoResetEvent
         * 
         * A synchronization object that allows one thread to signal one or more 
         * waiting threads that a certain event has occurred.
         * 
         * Designed to mimic System.Threading.AutoResetEvent
         * http://msdn.microsoft.com/en-us/library/system.threading.autoresetevent(v=vs.110).aspx
         */
        class AutoResetEvent{
        public:
            explicit AutoResetEvent(bool initial = false);
            
            void Set(); // sets the state of the event to signaled, allowing one or more threads to proceed
            void Reset(); // sets the state of the event to nonsignaled, causing threads to block
            
            bool WaitOne(); // blocks the current thread until the current wait handle receives a signal
            bool WaitOne(int milliseconds); /*
                                             * blocks the current thread until 
                                             * the current wait handle receives 
                                             * a signal or time interval has expired
                                             */
                
        private:
            AutoResetEvent(const AutoResetEvent&);
            AutoResetEvent& operator=(const AutoResetEvent&); // non-copyable
            bool flag_;
            std::mutex mutex_;
            std::condition_variable signal_;
        };
    } // namespace system
} // namespace ace

#endif	/* AUTORESETEVENT_H */

