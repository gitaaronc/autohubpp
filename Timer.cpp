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
#include "include/system/Timer.hpp"

namespace ace {
namespace system {

Timer::Timer(boost::asio::io_service& io_service)
: io_service_(io_service), interval_milliseconds_(0),
is_running_(false), timer_active_(false), timer_event_(false) {
}

Timer::~Timer() {
    if (is_running_) {
        Shutdown();
    }
    if (thread_.joinable()) {
        thread_.join();
    }
}

void
Timer::OnTimer(){
    _timerCallback(); 
}

bool
Timer::Run() {
    if (!_timerCallback || !is_running_) return false;
    while (is_running_) {
        std::unique_lock<std::mutex>lock(timer_mutex_);
        if (timer_active_) {
            if (!timer_cv_.wait_for(lock,
                    std::chrono::milliseconds(interval_milliseconds_),
                    [this]() {
                        return timer_event_ == true;
                    })) {
            {
                std::lock_guard<std::mutex>lock(lock_);
                timer_active_ = false;
            }
            io_service_.post(std::bind(&type::OnTimer, this));
        } else { // timer_event_ = true
                timer_event_ = false; // reset event
            }
        } else { // wait indefinitely or there's an event
            timer_cv_.wait(lock, [this]() {
                return timer_event_ == true;
            });
            timer_event_ = false; // reset event
        }
    }
}

bool
Timer::RunAsync() {
    if (!_timerCallback || is_running_) return false;
    is_running_ = true;
    std::thread process_thread(std::bind(&Timer::Run, this));
    thread_ = std::move(process_thread);
    return true;
}

void
Timer::Shutdown() {
    is_running_ = false;
    Stop();
}

// Stops the timer

void
Timer::Stop() {
    {
        std::lock_guard<std::mutex>lock(lock_);
        timer_active_ = false;
        timer_event_ = true;
    }
    timer_cv_.notify_all();
}

void
Timer::Reset(int milliseconds) {
    {
        std::lock_guard<std::mutex>lock(lock_);
        timer_event_ = true;
        interval_milliseconds_ = milliseconds;
        timer_active_ = true;
    }
    timer_cv_.notify_all();
}
}
}
