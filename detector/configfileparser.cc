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
#include "stringtools.h"

using namespace std;

std::string cConfigFileParser::mWhiteSpace = " \t\n";

void cConfigFileParser::AddKey (Key &kl, const string &key, const string &val)
{
    Key::iterator iter;
    stringList vl;
    iter = kl.find(key);
    if (iter != kl.end()) {
        vl = iter->second;
    }

    vl.push_back(val);
    kl[key] = vl;

}

void cConfigFileParser::AddSection (string sectionname,
                                         string key,
                                         const string val)
{
    Section::iterator iter;
    Key kl;
    sectionname = StringTools::ToUpper(sectionname);
    key = StringTools::ToUpper(key);
    iter = mSections.find(sectionname);
    if (iter != mSections.end()) {
        kl = iter->second;
    }

    AddKey(kl, key, val);
    mSections[sectionname] = kl;
}

bool cConfigFileParser::isSection (const string &token, string &section)
{
    size_t last = token.length()-1;
    if ((token.at(0) == '[') && (token.at(last) == ']')) {
        section = token.substr (1, last-1);
        return true;
    }
    return false;
}

stringQueue cConfigFileParser::TokenizeLine (const string line)
{
    stringQueue q;
    string newstr;
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

bool cConfigFileParser::ParseLine (const string line)
{
    string token;
    string key;
    bool keysep = false;
    stringQueue q = TokenizeLine (line);

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
bool cConfigFileParser::Parse(const string fname)
{
    ifstream file;
    string line;

    mCurrSection.clear();
    file.open(fname.c_str());
    if (!file.is_open()) {
        string err = "Can not open file " + fname;
        mLogger->logmsg (LOGLEVEL_ERROR,err.c_str());
        return false;
    }

    while (getline (file, line)) {
        if (!ParseLine (line)) {
            return false;
        }
    }
    if (!file.eof()) {
        string err = "Read error on file " + fname;
        mLogger->logmsg (LOGLEVEL_ERROR,err.c_str());
        return false;
    }
    return true;
}

// Find a key in a given section and return the values as list.
// Returns true if the key was found.
bool cConfigFileParser::GetValues (const string sectionname,
                                       const string key,
                                       stringList &values)
{
    Section::iterator seciter;
    Key::iterator keyiter;
    Key kl;

    string section = StringTools::ToUpper(sectionname);
    string k = StringTools::ToUpper(key);
    values.clear();
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

// Return all keys in a given section and return the values as list.
// Returns true if the section was found.
bool cConfigFileParser::GetKeys (const string sectionname,
                                     stringList &values)
{
    Section::iterator seciter;
    Key::iterator keyiter;
    Key kl;

    string section = StringTools::ToUpper(sectionname);

    values.clear();
    seciter = mSections.find(section);
    if (seciter == mSections.end()) {
        string err = "Can not find section " + section;
        mLogger->logmsg(LOGLEVEL_ERROR,err.c_str());
        return false;
    }
    values.clear();
    kl = seciter->second;
    for (keyiter = kl.begin(); keyiter != kl.end(); keyiter++) {
        values.push_back(keyiter->first);
    }

    return true;
}
// Return a a key in a given section as single value (not split into a key list)

bool cConfigFileParser::GetSingleValue (const string sectionname,
                                             const string key,
                                             string &value)
{
    stringList vals;

    string plugin;
    if (!GetValues(sectionname, key, vals)) {
        return false;
    }
    if (vals.size() != 1) {
        string err = "More than one argument to " + sectionname +  " key " + key;
        mLogger->logmsg(LOGLEVEL_ERROR,err.c_str());
        return false;
    }
    value = vals.front();
    return true;
}


// Get first section of a config file, returning the section name
bool cConfigFileParser::GetFirstSection (Section::iterator &iter,
                                              string &sectionname)
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
                                             string &sectionname)
{
    iter++;
    if (iter == mSections.end()) {
        return false;
    }
    sectionname = iter->first;
    return true;
}

// Helper function to validate that a section contains only required and
// optional keywords and that all required keywords are present.

bool cConfigFileParser::CheckSection (const string sectionname,
                                      const stringSet required,
                                      const stringSet optional)
{
    stringList::iterator it;
    stringList keys;

    map <string,bool> reqmap;
    map <string,bool>::iterator reqit;

// Check that required keywords are available
    stringSet::iterator req;
    for (req = required.begin(); req != required.end(); req++) {
        string s = *req;
        reqmap[s] = false;
    }
    // Check for required and optional keywords.
    if (!GetKeys(sectionname, keys)) {
        mLogger->logmsg(LOGLEVEL_ERROR, "Invalid section %s", sectionname.c_str());
        return false;
    }

    for (it = keys.begin(); it != keys.end(); it++) {
        string s = *it;
        bool inreq = (reqmap.find(s) != reqmap.end());
        bool inopt = (optional.find(s) != optional.end());
        if ((!inreq) && (!inopt)) {
            mLogger->logmsg(LOGLEVEL_ERROR, "Invalid keyword %s in section %s",
                            s.c_str(),
                            sectionname.c_str());
            return false;
        }
        if (inreq) {
            reqmap[s] = true;
        }
    }

    for (reqit = reqmap.begin(); reqit != reqmap.end(); reqit++) {
        if (!reqit->second) {
             mLogger->logmsg(LOGLEVEL_ERROR, "Required keyword %s missing in section %s",
                            reqit->first.c_str(),
                            sectionname.c_str());
            return false;
        }
    }
    return true;
}
