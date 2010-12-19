/*
 * extstring.h
 *
 *  Created on: 02.12.2010
 *      Author: uli
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
