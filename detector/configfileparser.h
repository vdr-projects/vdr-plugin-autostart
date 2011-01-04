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
#include <queue>
#include <algorithm>

#include "stringtools.h"
#include "logger.h"

typedef std::queue<std::string> stringQueue;
typedef std::vector<std::string> stringVector;
typedef std::map<std::string, stringVector> Key;
typedef std::map<std::string, Key> Section;

class cConfigFileParser {
private:
    static std::string mWhiteSpace;
    Section mSections;
    std::string mCurrSection;
    cLogger *mLogger;

    void AddKey (Key &, const std::string &, const std::string &);
    void AddSection (std::string , std::string , const std::string);
    bool isSection (const std::string &token, std::string &section);
    bool isComment (const std::string &token) {return (token.at(0) == ';');}
    bool ParseLine (const std::string line);
    stringQueue TokenizeLine (const std::string line);
public:

    cConfigFileParser(cLogger *l) {mLogger = l;}
    bool Parse (const std::string filename);
    bool GetFirstSection (Section::iterator &iter, std::string &sectionname);
    bool GetNextSection (Section::iterator &iter, std::string &sectionname);
    bool GetValues (const std::string sectionname, const std::string key,
                      stringVector &values);
    bool GetSingleValue (const std::string sectionname, const std::string key,
                           std::string &values);
};

#endif /* CONFIGFILEPARSER_H_ */
