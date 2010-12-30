/*
 * configfileparser.cc: Class for parsing a configuration file.
 *
 * The config file has the following syntax:
 * [SECTION NAME]
 * <KEY> = <VALUE> <VALUE> <VALUE>
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */
#include "configfileparser.h"

using namespace std;

std::string cConfigFileParser::mWhiteSpace = " \t\n";

void cConfigFileParser::AddKey (Key &kl, const cExtString &key, const cExtString &val)
{
    Key::iterator iter;
    cExtStringVector vl;
    iter = kl.find(key);
    if (iter != kl.end()) {
        vl = iter->second;
    }

    vl.push_back(val);
    kl[key] = vl;

}

void cConfigFileParser::AddSection (cExtString sectionname,
                                         cExtString key,
                                         const cExtString val)
{
    Section::iterator iter;
    Key kl;
    sectionname = sectionname.ToUpper();
    key = key.ToUpper();
    iter = mSections.find(sectionname);
    if (iter != mSections.end()) {
        kl = iter->second;
    }

    AddKey(kl, key, val);
    mSections[sectionname] = kl;
}

bool cConfigFileParser::isSection (const cExtString &token, cExtString &section)
{
    size_t last = token.length()-1;
    if ((token.at(0) == '[') && (token.at(last) == ']')) {
        section = token.substr (1, last-1);
        return true;
    }
    return false;
}

cExtStringQueue cConfigFileParser::TokenizeLine (const cExtString line)
{
    cExtStringQueue q;
    cExtString newstr;
    size_t i;
    char c;
    bool apo_active = false;

    for (i = 0; i < line.size(); i++ )
    {
        c = line.at(i);
        if (isspace(c) && (!apo_active)) {
            if (!newstr.empty()) {
                q.push(newstr);
                newstr.clear();
            }
        }
        else if (c == '"') {
            if (apo_active) {
                q.push(newstr);
                newstr.clear();
                apo_active = false;
            }
            else
            {
                apo_active = true;
            }
        }
        else {
            newstr += c;
        }
    }
    if (!newstr.empty()) {
        q.push(newstr);
        newstr.clear();
    }
    return q;
}

bool cConfigFileParser::ParseLine (const cExtString line)
{
    stringstream buffer(line);
    cExtString token;
    cExtString key;
    bool keysep = false;
    cExtStringQueue q = TokenizeLine (line);

    while (!q.empty()) {
        token = q.front();
        q.pop();
        /* Comment found ? */
        if (isComment(token)) {
            return true;
        }
        /* Is a section ? */
        if (isSection(token, mCurrSection)) {
            if (!q.empty()) {
                token = q.front();
                q.pop();

                if (isComment(token)) {
                    return true;
                } else {
                    mLogger->logmsg(LOGLEVEL_ERROR, "Syntax error on section");
                    return false;
                }
            }

            return true;
        }
        /* Read key */
        if ((!keysep) && (token != "=")) {
            if (key.empty()) {
                key = token;
            }
            else {
                mLogger->logmsg (LOGLEVEL_ERROR,"Missing =");
                return false;
            }
        }
        else if (token == "=") {
            if (keysep) {
                mLogger->logmsg (LOGLEVEL_ERROR,"Duplicate =");
                return false;
            }
            keysep = true;
        }
        /* Read Values of key */
        else {
            AddSection (mCurrSection, key, token);
        }
    }
    return true;
}

// Parse the config file
bool cConfigFileParser::Parse(const cExtString fname)
{
    ifstream mFile;
    string line;

    mCurrSection.clear();
    mFile.open(fname.c_str());
    if (!mFile.is_open()) {
        string err = "Can not open file " + fname;
        mLogger->logmsg (LOGLEVEL_ERROR,err.c_str());
        return false;
    }

    while (getline (mFile, line)) {
        if (!ParseLine (line)) {
            return false;
        }
    }
    if (!mFile.eof()) {
        string err = "Read error on file " + fname;
        mLogger->logmsg (LOGLEVEL_ERROR,err.c_str());
        return false;
    }
    return true;
}

// Find a key in a given section and return the values as Value List.
// Returns true if the key was found.
bool cConfigFileParser::GetValues (const cExtString sectionname,
                                       const cExtString key,
                                       cExtStringVector &values)
{
    Section::iterator seciter;
    Key::iterator keyiter;
    Key kl;

    cExtString section = sectionname.ToUpper();
    cExtString k = key.ToUpper();
    seciter = mSections.find(section);
    if (seciter == mSections.end()) {
        string err = "Can not find section " + section;
        mLogger->logmsg(LOGLEVEL_ERROR,err.c_str());
        return false;
    }
    kl = seciter->second;
    keyiter = kl.find(k);
    if (keyiter == kl.end()) {
        mLogger->logmsg(LOGLEVEL_ERROR,"Can not find key %s\n",k.c_str());
        return false;
    }
    values = keyiter->second;
    return true;
}

// Return a a key in a given section as single value (not split into a key list)

bool cConfigFileParser::GetSingleValue (const cExtString sectionname,
                                             const cExtString key,
                                             cExtString &value)
{
    cExtStringVector vals;

    cExtString plugin;
    if (!GetValues(sectionname, key, vals)) {
        return false;
    }
    if (vals.size() != 1) {
        string err = "More than one argument to " + sectionname +  " key " + key;
        mLogger->logmsg(LOGLEVEL_ERROR,err.c_str());
        return false;
    }
    value = vals[0];
    return true;
}


// Get first section of a config file, returning the section name
bool cConfigFileParser::GetFirstSection (Section::iterator &iter,
                                              cExtString &sectionname)
{

    if (mSections.empty()) {
        return false;
    }
    iter = mSections.begin();
    sectionname = iter->first;
    return true;
}

// Get next section, returning the section name
// Returns false if no next section exist
bool cConfigFileParser::GetNextSection (Section::iterator &iter,
                                             cExtString &sectionname)
{
    iter++;
    if (iter == mSections.end()) {
        return false;
    }
    sectionname = iter->first;
    return true;
}

