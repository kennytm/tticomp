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

#include <iostream>
#include "UnicodeRanges.h"
#include "Decompositions.h"

using std::cout;
using std::endl;
using util::String;

/*** UnicodeRange ***/

UnicodeRange::UnicodeRange(ULong aStartCode, ULong aEndCode,
						   ULong aScript, ULong aLanguage, const vector<ULong> &aFeatures) {
	startCode = aStartCode;
	endCode = aEndCode;
	script = aScript;
	language = aLanguage;
	features = aFeatures;
}

int UnicodeRange::compare(ULong code) {
	if (code < startCode)
		return -1;
	if (code > endCode)
		return 1;
	return 0;
}

ULong UnicodeRange::getScript() {
	return script;
}

ULong UnicodeRange::getLanguage() {
	return language;
}

const vector<ULong> &UnicodeRange::getFeatures() {
	return features;
}

/*** UnicodeRanges ***/

UnicodeRanges::UnicodeRanges(String fileName) {
	FILE *file = fopen(fileName.getCString(), "rb");
	if (file==NULL)
	{
		cout << "File not found." << endl;
		return;
	}
	
	// obtain file size.
	fseek (file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);
	
	// allocate memory to contain the whole file.
	Byte *buffer = new Byte[size+1];
	buffer[size] = '\0';
	
	// copy the file into the buffer.
	if (fread(buffer, 1, size, file) != size) {
		cout << "Error reading file." << endl;
		delete[] buffer;
		return;
	}
	// terminate
	fclose (file);
	
	/*** the whole file is loaded in the buffer. ***/
	Byte *curPos = buffer;
	while (curPos < buffer+size) {
		ULong startCode, endCode;
		startCode = getHex(curPos);
		if (*curPos != '-')
			endCode = startCode;
		else {
			curPos ++;
			endCode = getHex(curPos);
		}
		curPos ++;
		if (*curPos=='/') {
			vector<ULong> features;
			ranges.push_back (new UnicodeRange(startCode, endCode, 0, 0, features));
		} else {
			ULong script = getTag(curPos);
			curPos++;
			ULong language = getTag(curPos);
			curPos++;
			vector<ULong> features;
			while (isTagCharacter(*curPos)) {
				ULong feature = getTag(curPos);
				features.push_back(feature);
				curPos ++;
			}
			ranges.push_back(new UnicodeRange(startCode, endCode, script, language, features));
			features.clear();
		}

		findNewLine(curPos);
	}
	
	delete[] buffer;
}

UnicodeRanges::~UnicodeRanges() {}

RangeVector::iterator UnicodeRanges::begin() {
	return ranges.begin();
}

RangeVector::iterator UnicodeRanges::end() {
	return ranges.end();
}


