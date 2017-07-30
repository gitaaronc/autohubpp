/* 
 * File:   InsteonLinkMode.h
 * Author: aaronc
 *
 * Created on February 10, 2014, 1:25 PM
 */

#ifndef INSTEONLINKMODE_H
#define	INSTEONLINKMODE_H

#include <cstdint>

namespace ace {
    namespace insteon {
        enum class InsteonLinkMode : uint8_t {
            Responder = 0x00,
            Contoller = 0x01,
            Either = 0x02,
            Delete = 0xFF
        };
    }
}

#endif	/* INSTEONLINKMODE_H */

