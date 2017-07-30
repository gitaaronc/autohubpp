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

#ifndef MYUTILS_H
#define MYUTILS_H

#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <cstdint>

namespace ace {
    namespace utils {
        std::vector<uint8_t> ArraySubset(
                const std::vector<uint8_t>& data, uint32_t offset, uint32_t count);

        bool VectorsEqual(const std::vector<uint8_t>& source_one,
                const std::vector<uint8_t>& source_two);

        // Thanks to: http://stackoverflow.com/questions/5100718/int-to-hex-string-in-c
        template< typename T >
        std::string int_to_hex(T i) {
            std::stringstream stream;
            stream << "0x"
                    << std::setfill('0') << std::setw(sizeof (T)*2)
                    << std::hex << i;
            return stream.str();
        }

        uint8_t GetI2CS(const std::vector<uint8_t>& data,
                uint32_t first_byte, uint32_t last_byte);

        std::string ByteArrayToStringStream(
                const std::vector<uint8_t>& data, uint32_t offset, uint32_t count);
    }
}

#endif /* UTILS_H */

