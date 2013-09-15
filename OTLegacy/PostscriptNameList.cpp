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

#include <cassert>
#include <iostream>
#include <algorithm>
#include "PostscriptNameList.h"
#include "Decompositions.h"

using std::cout;
using std::endl;
using std::sort;
using util::String;

/*** PostscriptMapping ***/

PostscriptMapping::PostscriptMapping(ULong aUnicode, String aName)
: unicode (aUnicode), name (aName) {}

PostscriptMapping::~PostscriptMapping() {}

/*** PostscriptNameList ***/

bool codeOrder(PostscriptMapping *m1, PostscriptMapping *m2) {
	return m1->getUnicode() < m2->getUnicode();
}

PostscriptNameList::PostscriptNameList(String fileName) {
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
		if (*curPos != '#')
		{
			char name[256];
			char *curNamePos = &name[0];
			do {
				*curNamePos = *curPos;
				curNamePos ++;
				curPos ++;
			} while (*curPos != ';');
			*curNamePos = '\0';
			curPos ++;
			ULong code = getHex(curPos);
			mappings.push_back(new PostscriptMapping(code, name));
		}
		findNewLine(curPos);
	}
	
	delete[] buffer;
	sort(mappings.begin(), mappings.end(), codeOrder);
}

PostscriptNameList::~PostscriptNameList() {
	MappingList::iterator i;
	for (i = mappings.begin(); i < mappings.end(); i++)
		delete *i;
}

String PostscriptNameList::getPostscriptName(ULong unicode) {
	MappingList::iterator guess, lower, upper;
	lower = mappings.begin();
	upper = mappings.end();
	
	while (lower != upper) {
		guess = lower + (upper - lower) / 2;
		ULong guessCode = (*guess)->getUnicode();
		if (unicode < guessCode)
			upper = guess;
		else {
			if (guessCode < unicode)
				lower = guess + 1;
			else
				return (*guess)->getName();
		}
	}

	// unicode was not found
	return "uni" + getCode (unicode);
}

