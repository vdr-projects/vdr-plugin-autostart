/*
 * extstring: Extended string function
 *
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef EXTSTRING_H_
#define EXTSTRING_H_

#include <string>

class cExtString : public std::string
{
public:
    cExtString ToUpper (void) const {
        cExtString str(this);
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }
    cExtString(const cExtString &str) : std::string(str.c_str()) {};
    cExtString(const cExtString *str) : std::string(str->c_str()) {};
    cExtString(const char *str) : std::string(str) {};
    cExtString() : std::string() {};
    cExtString(const std::string &str) : std::string(str) {};
};

#endif /* EXTSTRING_H_ */
