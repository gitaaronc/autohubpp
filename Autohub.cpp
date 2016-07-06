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
#include "include/autoapi.hpp"
#include "include/DynamicLibrary.hpp"
#include "include/Logger.h"

#include "include/json/json.h"
#include "include/json/json-forwards.h"

#include <iostream>
#include <fstream>
#include <corvusoft/restbed/service.hpp>
#include <corvusoft/restbed/status_code.hpp>
#include <corvusoft/restbed/session.hpp>

namespace ace {

Autohub::Autohub(boost::asio::io_service& io_service)
: io_service_(io_service), insteon_network_(new insteon::InsteonNetwork(io_service)),
wspp_next_id_(0) {
}

Autohub::~Autohub() {
    stop();
    utils::Logger::Instance().Trace(FUNCTION_NAME);
}

void
Autohub::wsppOnOpen(connection_hdl hdl) {
    wspp_server::connection_ptr con = wspp_server_.get_con_from_hdl(hdl);
    websocketpp::uri_ptr u = con->get_uri();
    utils::Logger::Instance().Debug("CONNECTION URI: %s",
            con->get_uri()->str().c_str());

    utils::Logger::Instance().Debug("REQUESTED RESOURCE: %s", 
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
    utils::Logger::Instance().Debug("String Stream: %s", ss.str().c_str());
    utils::Logger::Instance().Debug("===================\n The required path is:"
    " %s", event.c_str());

    if (event.compare("getDeviceList") == 0) {
        Json::Value root;
        root = insteon_network_->SerializeJson();
        utils::Logger::Instance().Info(root.toStyledString().c_str());
        msg->set_payload(root.toStyledString());
        wspp_server_.send(hdl, msg);
    } else if (event.compare("device") == 0) {
        io_service_.post(std::bind(&type::InternalReceiveCommand, this,
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
Autohub::InternalReceiveCommand(const std::string json) {
    for (const auto& it : dynamicLibraryMap_) {
        std::shared_ptr<DynamicLibrary> ptr = it.second;
        if (ptr) {
            io_service_.post(std::bind(&AutoAPI::InternalReceiveCommand,
                    ptr->get_object(), json));
        }
    }
    insteon_network_->InternalReceiveCommand(json);
}

void
Autohub::OnUpdateDevice(Json::Value json) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    std::ostringstream buf;
    json["event"] = "deviceUpdate";
    for (const auto& it : wspp_connections_) {
        // Emit is a work around. I gave up casting a function pointer of an 
        // overloaded function (&wspp_server::send) to work with bind
        io_service_.post(std::bind(&Autohub::wsppEmit, this, it.first,
                json.toStyledString()));
    }
    std::ostringstream ss;
    Json::FastWriter fw;
    ss << "data: " << fw.write(json) << "\n\n";
    for (const auto& it : rest_sessions_) {
        it.second->yield(ss.str(), [ ](
                const std::shared_ptr< restbed::Session > session) {

        });
    }
}

// Emit is a work around. I gave up casting a function pointer of an 
// overloaded function (&wspp_server::send) to work with bind
// TODO: omit Emit!!

void
Autohub::wsppEmit(websocketpp::connection_hdl hdl, const std::string& buf) {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    wspp_server_.send(hdl, buf, websocketpp::frame::opcode::text);
}

void
Autohub::TestPlugin() {
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
    }
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
    restbed_.stop();
    wspp_server_.stop();
    if (wspp_server_thread_.joinable()) {
        wspp_server_thread_.join();
    }
    dynamicLibraryMap_.clear();
}

void
Autohub::start() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    insteon_network_->set_update_handler(bind(&type::OnUpdateDevice, this,
            std::placeholders::_1));
    insteon_network_->Connect();
    // wspp_server_.init_asio(); << self managed io_service
    
    wspp_server_.clear_access_channels(websocketpp::log::alevel::all);
    wspp_server_.clear_error_channels(websocketpp::log::elevel::all);
    
    wspp_server_.init_asio(&io_service_);
            
    wspp_server_.set_open_handler(bind(&Autohub::wsppOnOpen,
            this, std::placeholders::_1));

    wspp_server_.set_close_handler(bind(&Autohub::wsppOnClose,
            this, std::placeholders::_1));

    wspp_server_.set_message_handler(bind(&Autohub::wsppOnMessage,
            this, std::placeholders::_1, std::placeholders::_2));

    wspp_server_.listen(8080);
    wspp_server_.start_accept();

    wspp_server_thread_ = std::move(std::thread(
            std::bind(&wspp_server::run, &wspp_server_)));
    
    TestPlugin();
    startRestbed(); // running in this thread
}

void
Autohub::startRestbed() {
    utils::Logger::Instance().Trace(FUNCTION_NAME);
    auto resource = std::make_shared< restbed::Resource >();
    resource->set_path("/static/{filename: [a-z]*\\.html}");
    resource->set_method_handler("GET",
            std::bind(&Autohub::restGetHtmlHandler, this,
            std::placeholders::_1));
    restbed_.publish(resource);

    auto resource_api = std::make_shared<restbed::Resource>();
    resource_api->set_path("/api/v2/oauth2/token/");
    resource_api->set_method_handler("POST",
            std::bind(&Autohub::restGetAuthToken, this, std::placeholders::_1));
    restbed_.publish(resource_api);

    auto devices = std::make_shared<restbed::Resource>();
    devices->set_path("/api/v2/devices/");
    devices->set_method_handler("GET",
            std::bind(&Autohub::restGetDevices, this, std::placeholders::_1));
    restbed_.publish(devices);

    auto device = std::make_shared<restbed::Resource>();
    device->set_path("/api/v2/devices/{device: .*}");
    device->set_method_handler("GET",
            std::bind(&Autohub::restGetDevice, this, std::placeholders::_1));
    restbed_.publish(device);

    auto commands = std::make_shared<restbed::Resource>();
    commands->set_path("/api/v2/commands");
    commands->set_method_handler("POST",
            std::bind(&Autohub::restPostCommand, this, std::placeholders::_1));
    restbed_.publish(commands);

    auto events = std::make_shared<restbed::Resource>();
    events->set_path("/api/v2/events");
    events->set_method_handler("GET",
            std::bind(&Autohub::restEvents, this, std::placeholders::_1));
    restbed_.publish(events);

    /*auto ssl_settings = std::make_shared<restbed::SSLSettings>();
    ssl_settings->set_http_disabled(true);
    ssl_settings->set_private_key(Uri("file://server.key"));
    ssl_settings->set_certificate(Uri("file://server.crt"));
    ssl_settings->set_temporary_diffie_hellman(Uri("file:://dh768.pem"));*/

    auto settings = std::make_shared< restbed::Settings >();
    settings->set_port(8000);
    //settings->set_ssl_settings(ssl_settings);

    restbed_.start(settings);
}

void
Autohub::restEvents(const std::shared_ptr<restbed::Session> session) {
    rest_sessions_[session->get_origin()] = session;
    utils::Logger::Instance().Debug(session->get_origin().c_str());
    const std::multimap<std::string, std::string> headers{
        { "Content-Type", "text/event-stream"},
        { "Cache-Control", "no-cache"},
        { "Connection", "keep-alive"}
    };
    std::ostringstream ss;
    ss << "event: device_update\n"
            << "data: This is an update\n\n";

    session->yield(restbed::OK, ss.str(), headers, [ ](
            const std::shared_ptr< restbed::Session > session) {

    });
}

void
Autohub::restGetDevice(const std::shared_ptr<restbed::Session> session) {
    const auto request = session->get_request();
    const std::string device = request->get_path_parameter("device");
    int device_id = 0;
    device_id = std::stoi(device);
    Json::Value root;
    root = insteon_network_->SerializeJson(device_id);
    const std::multimap<std::string, std::string> headers{
        { "Content-Type", "text/html"},
        { "Cache-Control", "no-store"},
        { "Pragma", "no-cache"},
        { "Content-Length", std::to_string(root.toStyledString().length())}

    };
    session->close(restbed::OK, root.toStyledString(), headers);
}

void
Autohub::restGetDevices(const std::shared_ptr<restbed::Session> session) {
    Json::Value root;
    root = insteon_network_->SerializeJson();
    const std::multimap<std::string, std::string> headers{
        { "Content-Type", "text/html"},
        { "Cache-Control", "no-store"},
        { "Pragma", "no-cache"},
        { "Content-Length", std::to_string(root.toStyledString().length())}

    };
    session->close(restbed::OK, root.toStyledString(), headers);
}

void
Autohub::restGetHtmlHandler(const std::shared_ptr<restbed::Session> session) {
    const auto request = session->get_request();
    const std::string filename = request->get_path_parameter("filename");

    std::ifstream stream("/var/www/html/" + filename, std::ifstream::in);

    if (stream.is_open()) {
        const std::string body = std::string(
                std::istreambuf_iterator< char >(stream),
                std::istreambuf_iterator< char >());

        const std::multimap< std::string, std::string > headers{
            { "Content-Type", "text/html"},
            { "Content-Length", std::to_string(body.length())}
        };

        session->close(restbed::OK, body, headers);
    } else {
        session->close(restbed::NOT_FOUND);
    }
}

void
Autohub::restGetAuthToken(const std::shared_ptr<restbed::Session> session) {
    std::ostringstream ss;
    ss << "{\r\n"
            "\"access_token\" : \"test\",\r\n"
            "\"refresh_token\" : \"test\",\r\n"
            "\"token_type\" : \"Bearer\",\r\n"
            "\"expires_in\" : \"7200\"\r\n"
            "}\r\n";
    const std::multimap<std::string, std::string> headers{
        { "Content-Type", "text/html"},
        { "Cache-Control", "no-store"},
        { "Pragma", "no-cache"},
        { "Content-Length", std::to_string(ss.str().length())}

    };
    session->close(restbed::OK, ss.str(), headers);
}

void
Autohub::restPostCommand(const std::shared_ptr<restbed::Session> session) {
    {
        const auto request = session->get_request();
        const auto headers = request->get_headers();
        for (const auto& it : headers) {
            utils::Logger::Instance().Debug(it.second.c_str());
        }
        int content_length = 0;
        request->get_header("Content-Length", content_length);
        session->fetch(content_length, std::bind(&Autohub::restProcessJson, this,
                std::placeholders::_1, std::placeholders::_2));
    }
    std::ostringstream oss;
    oss << "{\r\n"
            "\"id\" : 123,\r\n"
            "\"status\" : \"succeeded\",\r\n"
            "\"link\" : \"/api/v2/commands/123\"\r\n"
            "}\r\n";
    const std::multimap<std::string, std::string> headers{
        { "Content-Type", "application/json"},
        { "Cache-Control", "no-store"},
        { "Pragma", "no-cache"},
        { "Content-Length", std::to_string(oss.str().length())}

    };
    session->close(restbed::ACCEPTED, oss.str(), headers);
}

void
Autohub::restProcessJson(const std::shared_ptr<restbed::Session> session,
        const restbed::Bytes& body) {
    std::string test(body.begin(), body.end());
    Json::Value root;
    Json::Reader reader;
    bool parsed = reader.parse(test.c_str(), root);
    if (!parsed) {
        utils::Logger::Instance().Info("Failed to parse %s", 
                reader.getFormattedErrorMessages().c_str());
        return;
    }
    io_service_.post(std::bind(&type::InternalReceiveCommand, this,
            test));
    utils::Logger::Instance().Debug(root.toStyledString().c_str());
}
} // namespace ace