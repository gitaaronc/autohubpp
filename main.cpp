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
#include "include/config.hpp"

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <thread>

#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>

#include <yaml-cpp/yaml.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "No arguments found on the command line.\n";
        std::cout << "Expecting configuration file on the command line.\n";
        std::cout << "eg: autohubpp /etc/autohubpp.yaml\n";
        return 0;
    }
    std::string config_file_;
    config_file_.assign(argv[1]);
    YAML::Node config;
    try {
        config = YAML::LoadFile(config_file_);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
        return 0;
    }

    // Simple global configuration
    // TODO Create a configuration class. Most likely a singleton class
    if (config["logging_mode"]) {
        std::cout << "LOGGING MODE: " << config["logging_mode"].as<std::string>()
                << std::endl; // still need to map to enum
    }

    if (config["worker_threads"]) {
        ace::config::worker_threads = config["worker_threads"].as<int>();
    }

    if (config["PLM"]["serial_port"])
        ace::config::serial_port_.assign(
            config["PLM"]["serial_port"].as<std::string>());

    if (config["PLM"]["baud_rate"])
        ace::config::baud_rate_ = config["PLM"]["baud_rate"].as<int>();

    if (config["WEBSOCKET"]["listening_port"])
        ace::config::wspp_port =
            config["WEBSOCKET"]["listening_port"].as<int>();

    if (config["INSTEON"]["command_delay"])
        ace::config::command_delay =
            config["INSTEON"]["command_delay"].as<int>();

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

    for (int c = 0; c < ace::config::worker_threads; c++) {
        ace::utils::Logger::Instance().Debug("Starting Thread: %d", (c + 1));
        threadpool.create_thread([&io_service]() {
            io_service.run();
        });
    }

    autohub.start();

    threadpool.join_all();

    return 0;
}

