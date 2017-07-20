/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HouseLincServer.hpp
 * Author: Aaron
 * 
 * Initial hatchet job to support houselinc application
 * TODO: replace hatchet job with proper implementation
 * 
 * Created on July 16, 2017, 3:27 PM
 */

#ifndef HOUSELINCSERVER_HPP
#define HOUSELINCSERVER_HPP

#include "Logger.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <functional>
#include <vector>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class session
: public std::enable_shared_from_this<session> {
public:

    session(tcp::socket socket, std::function<void(
            std::vector<unsigned char>) > on_receive,
            std::function<void(std::shared_ptr<session>) > on_disconnect)
    : socket_(std::move(socket)), on_receive_(on_receive),
    on_disconnect_(on_disconnect) {
    }

    void start() {
        do_read();
    }

    void do_write(std::vector<unsigned char>buffer) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(buffer, buffer.size()),
                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                    if (!ec) {
                    } else {
                        on_disconnect_(shared_from_this());
                    }
                });
    }

private:

    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                [this, self](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        std::vector<unsigned char> data;
                        for (int i = 1; i < length; i++) {
                            data.push_back(data_[i]);
                        }
                        on_receive_(data);
                        do_read();
                        //do_write(length);
                    } else {
                        on_disconnect_(shared_from_this());
                    }
                });
    }

    tcp::socket socket_;

    enum {
        max_length = 1024
    };
    unsigned char data_[max_length];
    std::function<void(std::vector<unsigned char> buffer) > on_receive_;
    std::function<void(std::shared_ptr<session>) > on_disconnect_;
};

class server {
public:

    server(boost::asio::io_service& io_service, short port,
            std::function<void(std::vector<unsigned char>) > on_receive)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    socket_(io_service), on_receive_(on_receive), client_count_(0) {
        do_accept();
    }

    void SendData(std::vector<unsigned char> buffer) {
        ace::utils::Logger::Instance().Trace(FUNCTION_NAME);
        for (auto client : sessions_) {
            client->do_write(buffer);
        }
    }

private:

    void do_accept() {
        acceptor_.async_accept(socket_,
                [this](boost::system::error_code ec) {
                    if (!ec) {
                        if (client_count_ < 5) {
                            auto client = std::make_shared<session>
                                    (std::move(socket_), on_receive_,
                                    std::bind(&server::on_disconnect, this,
                                    std::placeholders::_1));
                            sessions_.push_back(client);
                            client->start();
                            client_count_++;
                        }
                        socket_.close();
                    }
                    do_accept();
                });
    }

    void on_disconnect(std::shared_ptr<session> client) {
        sessions_.remove(client);
        client_count_--;
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::list<std::shared_ptr<session>> sessions_;
    std::function<void(std::vector<unsigned char> buffer) > on_receive_;
    int client_count_;
};

#endif /* HOUSELINCSERVER_HPP */

