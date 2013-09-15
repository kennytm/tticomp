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
	\file UnicodeRanges contains Unicode ranges and the way they should be rendered
	using OpenType.
*/

#ifndef UNICODERANGES_H
#define UNICODERANGES_H

#include <vector>
#include "../Util/smart_ptr.h"
#include "../Util/String.h"
#include "../OTFont/OpenType.h"

using std::vector;
using namespace OpenType;

class UnicodeRange {
	ULong startCode, endCode;
	ULong script, language;
	vector<ULong> features;
public:
	UnicodeRange(ULong aStartCode, ULong aEndCode,
		ULong aScript, ULong aLanguage, const vector<ULong> &aFeatures);
	int compare(ULong code);
	ULong getScript();
	ULong getLanguage();
	const vector <ULong> &getFeatures();
};

typedef util::smart_ptr <UnicodeRange> UnicodeRangePtr;
typedef vector <UnicodeRangePtr> RangeVector;

class UnicodeRanges {
	RangeVector ranges;
public:
	UnicodeRanges (util::String fileName);
	~UnicodeRanges();
	RangeVector::iterator begin();
	RangeVector::iterator end();
};

#endif	// UNICODERANGES_H
