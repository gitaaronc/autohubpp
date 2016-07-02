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

#ifndef LOGGER_H
#define	LOGGER_H

#include <fstream>
#include <sstream>

#include <vector>
#include <mutex>

namespace ace {
namespace utils {

class Logger {
public:

    enum LOGGING {
        NONE = 0,
        INFO = 1,
        WARNING = 2,
        DEBUG = 3,
        TRACE = 4,
        VERBOSE = 5,
    };

public:

    static Logger&
    Instance() {
        static Logger m_pInstance;
        return m_pInstance;
    }
    void Debug(const char *data, ...);
    void Info(const char *data, ...);
    //void Message(const std::string os);
    void Warning(const char *data, ...);

    void
    hexdump(const std::string& s,
            const std::vector<unsigned char>& d) {
        Debug(s.data());
        hexdump(d);
    }
    void hexdump(const std::vector<unsigned char> &s,
            unsigned int line_len = 16);
    void hexoutp(const char& c);
    void PrintTime();
    void SetLoggingMode(LOGGING logging_mode);
    void SetLogFile(std::ofstream outputFilestream);

    void
    Trace(const std::string& data) {
        Trace(data.data());
    }
    void Trace(const char *data, ...);
    std::string ByteArrayToStringStream(
            const std::vector<unsigned char>& data, int offset, int count);
protected:
    std::ostringstream oss_;
    std::ofstream ofs_;
private:
    void hexout(const char& c);
    void Output();
    std::mutex lock_;
    std::string Now();
    LOGGING logging_mode_;
private:

    Logger() {
    }

    ~Logger() {
    }
};
} // namespace utils
} // ace



#define S1(x) #x
#define S2(x) S1(x)
#define FUNCTION_LOCATION __FILE__ ":" S2(__LINE__)
#define FUNCTION_NAME std::string(__PRETTY_FUNCTION__) + "\t" \
+std::string(FUNCTION_LOCATION)

#define FUNCTION_NAME_CSTR std::string(FUNCTION_NAME).c_str()
//__FILE__ ":" S2(__LINE__)

#endif	/* LOGGER_H */

