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
#include "include/HouseLincServer.hpp"
#include "include/insteon/InsteonNetwork.hpp"
#include "include/autoapi.hpp"
#include "include/DynamicLibrary.hpp"
#include "include/Logger.h"

#include "include/json/json.h"
#include "include/json/json-forwards.h"
#include "include/utils/utils.hpp"

#include <iostream>
#include <fstream>

namespace ace
{

// TODO verify YAML::Node prior to passing to InsteonNetwork constructor

Autohub::Autohub(boost::asio::io_service& io_service, YAML::Node root)
: io_service_(io_service), strand_hub_(io_service), root_node_(root),
insteon_network_(new insteon::InsteonNetwork(io_service, root["INSTEON"])),
wspp_next_id_(0) {
    /*if (root_node_["INSTEON"].IsNull() || !root_node_["INSTEON"].IsDefined())
        throw; // TODO remove throw and improve error handling
     */
}

Autohub::~Autohub() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

void
Autohub::wsppOnOpen(connection_hdl hdl) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    wspp_server::connection_ptr con = wspp_server_.get_con_from_hdl(hdl);
    websocketpp::uri_ptr u = con->get_uri();
    utils::Logger::Instance().Info("wspp connection from: %s",
            con->get_uri()->str().c_str());

    utils::Logger::Instance().Info("wspp request resource: %s",
            u->get_resource().c_str());

    connection_data data;
    data.session_id = wspp_next_id_++;
    data.name = "";
    data.authenticated = false;

    std::lock_guard<std::mutex>lock(wspp_connections_mutex_);
    wspp_connections_[hdl] = data;
}

void
Autohub::wsppOnClose(connection_hdl hdl) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    connection_data& data = get_data_from_hdl(hdl);
    std::lock_guard<std::mutex>lock(wspp_connections_mutex_);
    wspp_connections_.erase(hdl);
}

void
Autohub::wsppOnMessage(connection_hdl hdl,
        wspp_server::message_ptr msg) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    connection_data& data = get_data_from_hdl(hdl);

    if (!data.authenticated) {
        data.name = msg->get_payload();
        data.authenticated = true;
    } else {

    }
    std::stringstream ss;
    ss.str(msg->get_payload());

    Json::Value root;
    Json::Reader reader;
    reader.parse(ss.str(), root);

    std::string event;
    event = root.get("event", "").asString();

    utils::Logger::Instance().Info("wspp received: %s",
            root.toStyledString().c_str());

    if (event.compare("getDeviceList") == 0) {
        Json::Value root;
        root = insteon_network_->serializeJson();
        utils::Logger::Instance().Info(root.toStyledString().c_str());
        msg->set_payload(root.toStyledString());
        wspp_server_.send(hdl, msg);
    } else if (event.compare("device") == 0) {
        strand_hub_.post(std::bind(&type::internalReceiveCommand, this,
                ss.str()));
    }
    //TestPlugin();
}

connection_data&
Autohub::get_data_from_hdl(connection_hdl hdl) {
    std::lock_guard<std::mutex>lock(wspp_connections_mutex_);
    auto it = wspp_connections_.find(hdl);
    if (it == wspp_connections_.end()) {
        throw std::invalid_argument("No Data available for this session");
    }
    return it->second;
}

void
Autohub::burp(std::string burp) {
    std::cout << "BURPPPPP:" << burp << std::endl;
}

void
Autohub::internalReceiveCommand(const std::string json) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    for (const auto& it : dynamicLibraryMap_) {
        std::shared_ptr<DynamicLibrary> ptr = it.second;
        if (ptr) {
            io_service_.post(std::bind(&AutoAPI::InternalReceiveCommand,
                    ptr->get_object(), json));
        }
    }
    insteon_network_->internalReceiveCommand(json);
}

void
Autohub::onUpdateDevice(Json::Value json) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    json["event"] = "deviceUpdate";
    for (const auto& it : wspp_connections_) {
        io_service_.post([ = ]{
            wspp_server_.send(it.first, json.toStyledString(),
            websocketpp::frame::opcode::text);
        });

    }
}

void
Autohub::TestPlugin() {/*
    std::string fileName = "libauto_plug1.so";
    std::string errorString;

    std::shared_ptr<DynamicLibrary> d = LoadLibrary(fileName, errorString);
    if (!d)
        return;

    PLUGINIT plugInit = (PLUGINIT) (d->getSymbol("PlugInit"));
    if (!plugInit)
        return;

    std::shared_ptr<AutoAPI> obj = plugInit(this);
    if (obj) {
        d->set_object(obj);
        dynamicLibraryMap_[obj->name()] = d;
    }*/
}

std::shared_ptr<DynamicLibrary>
Autohub::LoadLibrary(const std::string& path, std::string errorString) {
    std::shared_ptr<DynamicLibrary> d = DynamicLibrary::load(path, errorString);
    if (!d)
        return nullptr;
    return d;
}

void
Autohub::stop() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    insteon_network_->saveDevices();
    wspp_server_.stop_listening();
    {
        std::lock_guard<std::mutex>lock(wspp_connections_mutex_);
        for (const auto& it : wspp_connections_) {
            wspp_server::connection_ptr con = wspp_server_.get_con_from_hdl(it.first);
            con->close(1001, "Server shutting down or restarting!");
        }
    }

    wspp_server_.stop();
    if (wspp_server_thread_.joinable()) {
        wspp_server_thread_.join();
    }
    dynamicLibraryMap_.clear();
}

void
Autohub::start() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);

    wspp_server_.clear_access_channels(websocketpp::log::alevel::all);
    wspp_server_.clear_error_channels(websocketpp::log::elevel::all);

    wspp_server_.init_asio(&io_service_);

    wspp_server_.set_open_handler(bind(&Autohub::wsppOnOpen,
            this, std::placeholders::_1));

    wspp_server_.set_close_handler(bind(&Autohub::wsppOnClose,
            this, std::placeholders::_1));

    wspp_server_.set_message_handler(bind(&Autohub::wsppOnMessage,
            this, std::placeholders::_1,
            std::placeholders::_2));

    insteon_network_->set_update_handler(bind(&type::onUpdateDevice, this,
            std::placeholders::_1));

    insteon_network_->set_houselinc_tx(bind(&type::houselincTx, this,
            std::placeholders::_1));

    if (!insteon_network_->connect()) {
        utils::Logger::Instance().Info("Unable to connect to PLM.\n"
                "Shutting down now\n");
        wspp_server_.stop();
    } else {

        try {
            wspp_server_.listen(
                    root_node_["WEBSOCKET"]["listening_port"].as<int>(9000));
            wspp_server_.start_accept();
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }

        wspp_server_thread_ = std::move(std::thread(
                std::bind(&wspp_server::run,
                &wspp_server_)));

        houselinc_server_ = std::make_unique<server>(io_service_, 9761,
                bind(&type::houselincRx, this, std::placeholders::_1));

        TestPlugin();
    }
}

/*
 * Initial hatchet job to support houselinc application
 * TODO: replace hatchet job with proper implementation
 */
void
Autohub::houselincRx(std::vector<uint8_t> buffer) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::ostringstream oss;
    oss << "The following message was received by the network\n";
    oss << "\t  - {0x" << utils::ByteArrayToStringStream(
            buffer, 0, buffer.size()) << "}\n";
    utils::Logger::Instance().Debug(oss.str().c_str());
    strand_hub_.post(std::bind([this, buffer]() {
        insteon_network_->internalRawCommand(buffer);
    }));
}

/*
 * Initial hatchet job to support houselinc application
 * TODO: replace hatchet job with proper implementation
 */
void
Autohub::houselincTx(std::vector<uint8_t> buffer) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::ostringstream oss;
    oss << "Writing the following command to the Network!\n";
    oss << "\t  - {0x" << utils::ByteArrayToStringStream(
            buffer, 0, buffer.size()) << "}\n";
    utils::Logger::Instance().Debug(oss.str().c_str());
    houselinc_server_->SendData(buffer);
}
} // namespace ace
