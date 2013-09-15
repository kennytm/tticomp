/*
	(c) Copyright 2002, 2003 Rogier van Dalen
	(R.C.van.Dalen@umail.leidenuniv.nl for any comments, questions or bugs)

	This file is part of my OpenType/TrueType Font Tools.

	The OpenType/TrueType Font Tools is free software; you can redistribute
	it and/or modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation; either version 2 of the
	License, or (at your option) any later version.

	The OpenType/TrueType Font Tools is distributed in the hope that it will
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
	Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the OpenType/TrueType Font Tools; if not, write to the Free
	Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
	\file PostScriptNameList reads the Adobe Glyph List and lets an application
	find a postscript name for any unicode value.
	It implements the Adobe recommendation as well (unixxxx)
*/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#ifndef POSTSCRIPTNAMELIST_H
#define POSTSCRIPTNAMELIST_H

#include <vector>
#include "../OTFont/OpenTypeFont.h"

using std::vector;
using namespace OpenType;

class PostscriptMapping {
	ULong unicode;
	util::String name;
public:
	PostscriptMapping(ULong aUnicode, util::String aName);
	~PostscriptMapping();
	ULong getUnicode() { return unicode; }
	util::String getName() { return name; }
};

class PostscriptNameList {
	typedef vector <PostscriptMapping *> MappingList;
	MappingList mappings;
public:
	PostscriptNameList (util::String aFileName);
	~PostscriptNameList();
	util::String getPostscriptName(ULong unicode);
};

#endif // POSTSCRIPTNAMELIST_H
