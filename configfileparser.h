/*
 * configfileparser.h: Class for parsing a configuration file.
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef CONFIGFILEPARSER_H_
#define CONFIGFILEPARSER_H_

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <algorithm>

#include "extstring.h"
#include "logger.h"

typedef std::vector<cExtString> ValueList;
typedef std::map<cExtString, ValueList> Key;
typedef std::map<cExtString, Key> Section;

class cConfigFileParser {
private:
    Section sections;
    cExtString mCurrSection;
    cLogger mLogger;

    void AddKey (Key &, const cExtString &, const cExtString &);
    void AddSection (cExtString , cExtString , const cExtString);
    bool isSection (const cExtString &token, cExtString &section);
    bool isComment (const cExtString &token) {return (token.at(0) == ';');}
    bool ParseLine (const cExtString &line);

public:
    cConfigFileParser() {}
    cConfigFileParser(cLogger l) {mLogger = l;}
    bool Parse (const cExtString filename);
    bool GetFirstSection (Section::iterator &iter,cExtString &sectionname);
    bool GetNextSection (Section::iterator &iter, cExtString &sectionname);
    bool GetValues (const cExtString sectionname, const cExtString key,
                      ValueList &values);
    bool GetSingleValue (const cExtString sectionname, const cExtString key,
                           cExtString &values);
};

#endif /* CONFIGFILEPARSER_H_ */
