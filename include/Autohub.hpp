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

#ifndef AUTOHUB_HPP
#define AUTOHUB_HPP

#include <memory>

#include <boost/asio/io_service.hpp>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>

#include <restbed>
#include <yaml-cpp/yaml.h>

typedef websocketpp::server<websocketpp::config::asio> wspp_server;
using websocketpp::connection_hdl;

using websocketpp::lib::bind;
using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace Json {
    class Value;
}
namespace ace {
    class DynamicLibrary;
    namespace insteon {
        class InsteonNetwork;
    }

    struct connection_data {
        int session_id;
        std::string name;
        bool authenticated;
    };

    class Autohub {
        typedef Autohub type;
    public:
        Autohub() = delete;
        Autohub(boost::asio::io_service& io_service, YAML::Node root);
        ~Autohub();
        void burp(std::string burp); // plugin test
        void start();
        void stop();
    private:
        void wsppOnOpen(connection_hdl hdl);
        void wsppOnClose(connection_hdl hdl);
        void wsppOnMessage(connection_hdl hdl, wspp_server::message_ptr msg);
        connection_data& get_data_from_hdl(connection_hdl hdl);

        void internalReceiveCommand(const std::string json);
        void onUpdateDevice(Json::Value json);

        std::shared_ptr<DynamicLibrary> LoadLibrary(const std::string& path,
                std::string errorString);
        void TestPlugin();
    private:
        boost::asio::io_service& io_service_;
        boost::asio::strand strand_hub_;
        std::unique_ptr<insteon::InsteonNetwork> insteon_network_;

        std::string yaml_config_file_;
        YAML::Node root_node_;

        wspp_server wspp_server_;
        typedef std::map<connection_hdl, connection_data,
        std::owner_less<connection_hdl>> con_list;
        con_list wspp_connections_;
        std::mutex wspp_connections_mutex_;
        std::thread wspp_server_thread_;
        int wspp_next_id_;

        restbed::Service restbed_;
        void startRestbed();
        void restEvents(const std::shared_ptr<restbed::Session> session);
        void restGetDevice(const std::shared_ptr<restbed::Session> session);
        void restGetDevices(const std::shared_ptr<restbed::Session> session);
        void restGetHtmlHandler(const std::shared_ptr<restbed::Session> session);
        void restGetAuthToken(const std::shared_ptr<restbed::Session> session);
        void restPostCommand(const std::shared_ptr<restbed::Session> session);
        void restProcessJson(const std::shared_ptr<restbed::Session> session,
                const restbed::Bytes& body);
        std::map<std::string, std::shared_ptr<restbed::Session> > rest_sessions_;

        void houseLincRx(std::vector<unsigned char> buffer);
        void houseLincTx(std::vector<unsigned char> buffer);
        std::map<std::string, std::shared_ptr<DynamicLibrary>> dynamicLibraryMap_;
    };
}

#endif /* AUTOHUB_HPP */

