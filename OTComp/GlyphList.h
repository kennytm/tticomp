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
	\file GlyphList.h defines classes for OpenType Layout glyphs
*/

#ifndef GLYPHLIST_H
#define GLYPHLIST_H

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <vector>
#include "../Util/smart_ptr.h"
#include "../Util/String.h"
#include "../Util/Preprocessor.h"
#include "../OTFont/OpenTypeFont.h"
#include "ReferenceTable.h"
#include "Script.h"

using std::ostream;
using namespace OpenType;

class GlyphGroup;
class GlyphGroupHasher;
typedef smart_ptr <GlyphGroup> GlyphGroupPtr;
class Groups;

typedef std::vector <GlyphId> GlyphIds;

typedef enum {
	gcUndefined = 0,
	gcBaseGlyph = 1,
	gcLigature = 2,
	gcMark = 3,
	gcComponent = 4
} GlyphClass;
typedef std::vector <GlyphClass> GlyphClasses;

void writeToStream (const GlyphIds & glyphIds, ostream & o, OpenTypeFont & font);

class GlyphList : public GlyphIds {
	PreprocessorPosition pos;
public:
	GlyphList (Preprocessor & prep, OpenTypeFont & font, const Groups &groups);
	virtual ~GlyphList();
	void sort();
	void checkDuplicates (OpenTypeFont &font);
};

typedef smart_ptr <GlyphList> GlyphListPtr;


class GlyphGroup {
	String name;
	GlyphListPtr glyphs;
	GlyphClass glyphClass;
public:
	GlyphGroup (Preprocessor & prep, OpenTypeFont & font, const Groups &groups);
	virtual ~GlyphGroup();

	const GlyphListPtr getGlyphs() const { return glyphs; }
	const String getName() const { return name; }
	const GlyphClass getGlyphClass() const { return glyphClass; }
};

class Groups : public std::map <String, GlyphGroupPtr> {
	typedef std::vector <GlyphGroupPtr> MarkGroups;
	MarkGroups markGroups;
public:
	Groups();
	virtual ~Groups();

	UShort getMarkGroupIndex (String name, Preprocessor & prep);
	ReferenceMemoryBlockPtr getMarkClassDef (OpenTypeFont & font) const;
};

ReferenceMemoryBlockPtr getClassDefTable (const GlyphClasses& classes);

template <class Iterator, class GetGlyphId>
ReferenceMemoryBlockPtr getCoverageTable (Iterator begin, Iterator end,
										  GetGlyphId getGlyphId)
{
	Iterator i;
	// Count ranges
	UShort rangeNum = 1;
	for (i = begin + 1; i != end; i ++) {
		assert (getGlyphId (i - 1) < getGlyphId (i));
		if (getGlyphId (i-1) != getGlyphId (i)-1)
			rangeNum ++;
	}

	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);

	if (rangeNum * 3 > end - begin) {
		// Use format 1
		pen.writeUShort (1);
		pen.writeUShort (end - begin);
		for (i = begin; i != end; i ++)
			pen.writeUShort (getGlyphId(i));
	} else {
		// Use format 2
		pen.writeUShort (2);
		pen.writeUShort (rangeNum);
		Iterator first = begin;
		i = begin + 1;
		while (true) {
			if (i == end || getGlyphId (i-1) != getGlyphId (i)-1) {
				// Range found
				// Start
				pen.writeUShort (getGlyphId (first));
				// End
				pen.writeUShort (getGlyphId (i-1));
				// Coverage index
				pen.writeUShort (first - begin);

				first = i;
				if (i == end)
					break;
			}
			i ++;
		}
	}

	return memory;
}

#endif	// GLYPHLIST_H
