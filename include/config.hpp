/* 
 * File:   config.hpp
 * Author: aaronc
 *
 * Created on July 7, 2016, 7:07 PM
 */

#ifndef CONFIG_HPP
#define	CONFIG_HPP

#include <iostream>

namespace ace {
namespace config {
extern std::string serial_port_; // port number/name
extern int baud_rate_; // serial/USB PLM baud rate
extern int wspp_port; // websocket listening port
extern int command_delay; // to to wait between sending commands
extern int worker_threads; // the number of worker threads
}
}

#endif	/* CONFIG_HPP */

