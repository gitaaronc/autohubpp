/* 
 * File:   InsteonLinkMode.h
 * Author: aaronc
 *
 * Created on February 10, 2014, 1:25 PM
 */

#ifndef INSTEONLINKMODE_H
#define	INSTEONLINKMODE_H

namespace ace {
    namespace insteon {
        enum class InsteonLinkMode : unsigned char {
            Responder = 0x00,
            Contoller = 0x01,
            Either = 0x02,
            Delete = 0xFF
        };
    }
}

#endif	/* INSTEONLINKMODE_H */

