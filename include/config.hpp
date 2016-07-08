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
static std::string serial_port = "/dev/ttyUSB0";
static int baud_rate = 19200;
}
}

#endif	/* CONFIG_HPP */

