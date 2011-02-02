/*
 * mstdtypes.h: Standard typedefs for all media testers
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef STDTYPES_H_
#define STDTYPES_H_

#include <string>
#include <list>
#include <map>
#include <queue>
#include <algorithm>
#include <set>

typedef std::queue<std::string> stringQueue;
typedef std::list<std::string> stringList;
typedef std::set<std::string> stringSet;
typedef std::map<std::string, stringList> Key;
typedef std::map<std::string, Key> Section;

#endif /* STDTYPES_H_ */
