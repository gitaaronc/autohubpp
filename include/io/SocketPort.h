/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SocketPort.h
 * Author: Aaron
 *
 * Created on February 20, 2017, 5:42 PM
 */

#ifndef SOCKETPORT_H
#define SOCKETPORT_H

#include "ioport.hpp"

#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

namespace ace {
    namespace io {
        typedef std::shared_ptr<boost::asio::ip::tcp::socket> socket_port_ptr;
        typedef std::function<void() > recv_handler;

        class SocketPort : public IOPort {
        public:
            typedef IOPort base;
            typedef SocketPort type;

            SocketPort(boost::asio::io_service& ios) : base(), socket_io_(ios),
            recv_buffer_has_data_(false) {
                socket_port_ = std::make_shared<boost::asio::ip::tcp::socket>
                        (socket_io_);
            }

            SocketPort() = delete;

            ~SocketPort() {
                close();
            }

            bool open(const std::string host, int) override;

            void async_read_some() override;

            void
            set_recv_handler(std::function<void() > fp) override {
                m_recv_handler = fp;
            }

            void close();

            std::size_t recv_with_timeout(std::vector<unsigned char>& buffer,
                    int msTimeout = 50) override;

            unsigned int recv_buffer(std::vector<unsigned char>& buffer) override;

            unsigned int send_buffer(std::vector<unsigned char>& buffer) override;
        protected:

            void on_async_receive_some(const boost::system::error_code& ec,
                    size_t bytes_transferred);

            void on_async_receive_more(const boost::system::error_code& ec,
                    size_t bytes_transferred) ;

        private:
            boost::asio::io_service& socket_io_;
            recv_handler m_recv_handler;
            socket_port_ptr socket_port_;
            std::vector<unsigned char> incoming_buffer_;
            std::vector<unsigned char> recv_buffer_;
            bool recv_buffer_has_data_;
            std::mutex recv_buffer_mutex_;
        };
    } // namespace io
} // namespace ace
#endif /* SOCKETPORT_H */

