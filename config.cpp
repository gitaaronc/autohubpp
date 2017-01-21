#include <iostream>

#include "include/config.hpp"

namespace ace {
namespace config {
std::string serial_port_ = "";
int baud_rate_ = 19200;
int wspp_port = 9000;
int worker_threads = 10;
int command_delay = 1500; // delay to wait between sending commands
}
}

