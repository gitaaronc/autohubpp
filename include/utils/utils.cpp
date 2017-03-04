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
#include "utils.hpp"

namespace ace
{
namespace utils
{

std::vector<unsigned char>
ArraySubset(const std::vector<unsigned char>& data, int offset, int count) {
    std::vector<unsigned char> return_buffer;
    if (count > data.size() - offset)
        count = data.size() - offset;
    return_buffer.insert(return_buffer.end(), data.begin() + offset,
            data.begin() + offset + count);
    return return_buffer;
}

/**
 * Checks whether or not the contents of two unsigned char vector arrays are equal
 * NOTE: Will return true if both vectors are zero size
 * @param source_one
 * @param source_two
 * @return true if contents are the same
 */
bool
VectorsEqual(const std::vector<unsigned char>& source_one,
        const std::vector<unsigned char>& source_two) {
    if (source_one.size() != source_two.size()) return false;
    return std::equal(source_one.begin(), source_one.end(), source_two.begin());
}

unsigned char
GetI2CS(const std::vector<unsigned char>& data, unsigned int first_byte,
        unsigned int last_byte) {
    unsigned char count = last_byte - first_byte;
    unsigned char i2cs = 0;
    if ((data.size() < count) || (count <= 0)) return 0;
    for (int i = first_byte; i < last_byte; i++) {
        i2cs += data[i];
    }
    i2cs = (~(i2cs) + 1) & 0xFF;
    return i2cs;
}

std::string
ByteArrayToStringStream(
        const std::vector<unsigned char>& data, int offset, int count) {
    std::stringstream strStream;
    for (int i = offset; i < offset + count; ++i) {
        if (i < data.size()) {
            strStream << std::hex << std::setw(2) << std::setfill('0')
                    << (unsigned int) data[i];
        }
    }
    return strStream.str();
}

}
}


