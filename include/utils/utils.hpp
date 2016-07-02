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
#define	MYUTILS_H

#include <sstream>
#include <string>
#include <iomanip>
#include <vector>

namespace ace {
    namespace utils {
        std::vector<unsigned char> ArraySubset(const std::vector<unsigned char>& data, int offset, int count);
        
        bool VectorsEqual(const std::vector<unsigned char>& source_one, const std::vector<unsigned char>& source_two);

        // Thanks to: http://stackoverflow.com/questions/5100718/int-to-hex-string-in-c
        template< typename T >
        std::string int_to_hex(T i) {
            std::stringstream stream;
            stream << "0x"
                    << std::setfill('0') << std::setw(sizeof (T)*2)
                    << std::hex << i;
            return stream.str();
        }
        
        unsigned char GetI2CS(const std::vector<unsigned char>& data, unsigned int first_byte, unsigned int last_byte);
    }
}

#endif	/* UTILS_H */

