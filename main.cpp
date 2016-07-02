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

#include "include/Autohub.hpp"
#include "include/insteon/InsteonNetwork.hpp"
#include "include/Logger.h"

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <thread>

#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>

int main(int argc, char** argv) {
    ace::utils::Logger::Instance().SetLoggingMode(
            ace::utils::Logger::VERBOSE);

    ace::utils::Logger::Instance().Debug("Using Boost version: %d.%d.%d", 
            BOOST_VERSION / 100000, (BOOST_VERSION / 100) % 1000, 
            BOOST_VERSION % 100);
    
    boost::asio::io_service io_service;
    boost::thread_group threadpool;
    boost::asio::io_service::work work(io_service);

    ace::Autohub autohub(io_service);

    boost::asio::signal_set sig_set(io_service, SIGTERM, SIGINT);
    sig_set.async_wait(std::bind([&io_service, &autohub]() {
        autohub.stop();
        io_service.stop();
    }));

    for (int c = 0; c < 50; c++) {
        ace::utils::Logger::Instance().Debug("Starting Thread: %d", (c + 1));
        threadpool.create_thread([&io_service]() {
            io_service.run();
        });
    }

    autohub.start();

    threadpool.join_all();
    
    return 0;
}

