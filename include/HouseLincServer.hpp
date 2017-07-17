/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HouseLincServer.hpp
 * Author: Aaron
 *
 * Created on July 16, 2017, 3:27 PM
 */

#ifndef HOUSELINCSERVER_HPP
#define HOUSELINCSERVER_HPP

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <functional>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class session
: public std::enable_shared_from_this<session> {
public:

    session(tcp::socket socket, std::function<void(
            std::vector<unsigned char>) > on_receive)
    : socket_(std::move(socket)), on_receive_(on_receive) {
    }

    void start() {
        do_read();
    }

private:

    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                [this, self](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        std::vector<unsigned char> data;
                        for(int i = 1; i < length -1; i++){
                                data.push_back(data_[i]);
                        }
                        on_receive_(data);
                        //do_write(length);
                    }
                });
    }

    void do_write(std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
                [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                    if (!ec) {
                        do_read();
                    }
                });
    }

    tcp::socket socket_;

    enum {
        max_length = 1024
    };
    unsigned char data_[max_length];
    std::function<void(std::vector<unsigned char> buffer) > on_receive_;
};

class server {
public:

    server(boost::asio::io_service& io_service, short port,
            std::function<void(std::vector<unsigned char>) > on_receive)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    socket_(io_service), on_receive_(on_receive) {
        do_accept();
    }

private:

    void do_accept() {
        acceptor_.async_accept(socket_,
                [this](boost::system::error_code ec) {
                    if (!ec) {
                        std::make_shared<session>(std::move(socket_), 
                                on_receive_)->start();
                    }

                    do_accept();
                });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::function<void(std::vector<unsigned char> buffer) > on_receive_;
};

#endif /* HOUSELINCSERVER_HPP */

