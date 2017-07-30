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
#include "include/Logger.h"

#include <chrono>
#include <bitset>
#include <iostream>
#include <iomanip>

#include <memory>
#include <cstdarg>

#include <sys/time.h>

namespace ace {
namespace utils {

/*
std::bitset<3> x(messageFlags >> 5);
std::stringstream stream;
stream << "\033[1;31mMessage Flags: 0b" << x << "\033[0m";*/

std::string
Logger::ByteArrayToStringStream(
        const std::vector<uint8_t>& data, uint32_t offset, uint32_t count) {
    std::stringstream strStream;
    std::cout << FUNCTION_NAME << std::endl;
    for (int i = offset; i < offset + count; ++i) {
        if (i < data.size()) {
            strStream << std::hex << std::setw(2) << std::setfill('0')
                    << (unsigned int) data[i];
        }
    }
    return strStream.str();
}

void
Logger::PrintTime() {
    std::lock_guard<std::mutex>lock(lock_);
    oss_ << "\033[1;31m[TIME]    \033[0m" << Now() << std::endl;
    Output();
}

/*
        void Logger::Message(const std::string os) {
            lock_.lock();
            oss_ << os << std::endl;
            Output();
        }*/

void
Logger::hexout(const char& c) {
    uint8_t uc = static_cast<uint8_t> (c);
    oss_ << std::setw(2) << std::setfill('0') << (unsigned int) uc
            << ' ';
}

void
Logger::hexoutp(const char& c) {
    //lock_.lock();
    uint8_t uc = static_cast<uint8_t> (c);
    std::ios::fmtflags f(oss_.flags());
    oss_ << std::hex << std::setw(2) << std::setfill('0')
            << (unsigned int) uc;
    oss_.flags(f);
    //Output();
}

void
Logger::hexdump(const std::vector<uint8_t> &s,
        uint32_t line_len) {
    if (logging_mode_ < LOGGING::VERBOSE)
        return;
    lock_.lock();
    oss_ << "\033[1;36m[HEX DUMP] Displaying: " << s.size()
            << " bytes. " << Now() << std::endl;
    const std::string::size_type slen(s.size());
    uint32_t i(0);
    std::string::size_type pos(0);
    const std::streamsize lines(slen / line_len);
    const uint32_t chars(slen % line_len);
    std::ios::fmtflags f(oss_.flags());

    oss_ << ":------: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"
            "  0123456789ABCDEF\n";
    oss_ << std::hex;
    for (std::streamsize line = 0; line < lines; ++line) { // complete lines(s)
        oss_ << std::setw(8) << std::setfill('0') << (16 * line) << ' ';
        for (i = 0; i < line_len; ++i) {
            hexout(s[pos++]);
        }
        oss_ << '\n';
    }
    if (chars) { // not a complete line
        oss_ << std::setw(8) << std::setfill('0') << (lines * 16) << ' ';
        for (i = 0; i < chars; ++i) { // not a complete line
            hexout(s[pos++]);
        }
        for (i = 0; i < (line_len - chars); ++i) { // used for padding
            oss_ << "   ";
        }
    }
    if (i)
        oss_ << '\n';
    oss_.flags(f);
    oss_ << "\033[0m";
    Output();
}

void
Logger::Output() {
    //std::cout << "\033[1;31m" << oss_.str() << "\033[0m";
    std::cout << oss_.str();
    if (!oss_.str().empty()) {
        oss_.str(std::string());
    }
    lock_.unlock();
}

std::string
Logger::Now() {
    /*
    time_t t = time(0);
    char buffer[9] = {0};

    strftime(buffer, 9, "%H:%M:%S.%s", localtime(&t));
     */
    char buffer[30] = {0};
    struct timeval tv;
    time_t curtime;

    gettimeofday(&tv, nullptr);
    curtime = tv.tv_sec;

    strftime(buffer, 30, "%H:%M:%S", localtime(&curtime));
    char bufferTwo[60] = {0};
    sprintf(bufferTwo, "%s:%ld", buffer,
            std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count());
    return std::string(bufferTwo);
}

void
Logger::SetLoggingMode(LOGGING logging_mode) {
    std::lock_guard<std::mutex>lock(lock_);
    logging_mode_ = logging_mode;
}

void
Logger::SetLogFile(std::ofstream outputFilestream) {
    //ofs_.swap(outputFilestream);
}

void
Logger::Debug(const char *data, ...) {
    std::lock_guard<std::mutex>lock(lock_);
    char buffer[512] = {};
    if (data == nullptr)
        return;

    if (logging_mode_ < LOGGING::DEBUG)
        return;

    va_list args;
    va_start(args, data);
    vsnprintf(buffer, sizeof (buffer), data, args);
    va_end(args);
    oss_ << "\033[1;36m[DEBUG]   \033[0m" << buffer << std::endl;
    Output();
}

void
Logger::Info(const char *data, ...) {
    std::lock_guard<std::mutex>lock(lock_);
    char buffer[512] = {};
    if (data == nullptr)
        return;

    if (logging_mode_ < LOGGING::INFO)
        return;

    va_list args;
    va_start(args, data);
    vsnprintf(buffer, sizeof (buffer), data, args);
    va_end(args);
    oss_ << "\033[1;31m[INFO]    \033[0m\033[1;32m" << buffer << "\033[0m" 
            << std::endl;
    Output();
}

void
Logger::Trace(const char *data, ...) {
    std::lock_guard<std::mutex>lock(lock_);
    char buffer[512] = {};
    if (data == nullptr)
        return;

    if (logging_mode_ < LOGGING::TRACE)
        return;

    va_list args;
    va_start(args, data);
    vsnprintf(buffer, sizeof (buffer), data, args);
    va_end(args);
    oss_ << "\033[1;32m[TRACE]   \033[0m" << buffer << std::endl;
    //oss_ << buffer << std::endl;
    Output();
}

void
Logger::Warning(const char *data, ...) {
    std::lock_guard<std::mutex>lock(lock_);
    char buffer[512] = {};
    if (data == nullptr)
        return;

    if (logging_mode_ < LOGGING::WARNING)
        return;

    va_list args;
    va_start(args, data);
    vsnprintf(buffer, sizeof (buffer), data, args);
    va_end(args);
    oss_ << "\033[1;31m[WARNING] \033[0m" << buffer << std::endl;
    Output();
}
} // namespace utils
} // ace

