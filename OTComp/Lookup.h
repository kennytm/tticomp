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
	\file Lookup.h defines OpenType Layout lookups.
*/

#ifndef LOOKUP_H
#define LOOKUP_H

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <vector>
#include "../Util/Preprocessor.h"
#include "../OTFont/OpenTypeFont.h"
#include "ReferenceTable.h"
#include "GlyphList.h"

using namespace OpenType;
using std::vector;

typedef vector <ReferenceMemoryBlockPtr> ReferenceMemoryBlocks;

struct LookupSubTable {
	PreprocessorPosition pos;
	String name;

	enum LookupPlace {
		unknownLookup, GSUBLookup, GPOSLookup
	};
	LookupPlace place;
	enum LookupType {
		unknownType = 0,
			singleSubstitution = 1, multipleSubstitution = 2, alternateSubstitution = 3,
			ligatureSubstitution = 4, contextSubstitution = 5, chainingContextSubstitution = 6,
			singlePositioning = 1, pairPositioning = 2, markToBaseAttachment = 4,
			markToLigatureAttachment = 5, markToMarkAttachment = 6, contextPositioning = 7,
			chainingContextPositioning = 8
	};
	LookupType type;
	UShort lookupFlag;
	ReferenceMemoryBlocks memory;

	LookupSubTable (const PreprocessorPosition & _pos) : pos (_pos) {}
	LookupSubTable (const LookupSubTable & t) : pos (t.pos), name (t.name), place (t.place),
		type (t.type), lookupFlag (t.lookupFlag), memory (t.memory) {}
};

typedef smart_ptr <LookupSubTable> LookupSubTablePtr;
typedef std::vector <LookupSubTablePtr> LookupSubTables;

LookupSubTablePtr getLookup (Preprocessor & prep, OpenTypeFont & font, Groups & groups);
void writeReferenceLookups (const LookupSubTables & lookups, Bools & lookupsUsed,
							LookupSubTable::LookupPlace place);

/*** Anchor ***/

class Anchor {
	PreprocessorPosition pos;
	bool contourPoint;
	Short x, y;

	struct ContourPointAnchor {
		GlyphPtr glyph;
		Short x, y;
		ReferenceMemoryPen pointIndexPen;
	};
	typedef std::vector <ContourPointAnchor> ContourPointAnchors;
	static ContourPointAnchors contourPointAnchors;
	static bool compareContourPointAnchors (const ContourPointAnchor &a1, const ContourPointAnchor &a2);
public:
	Anchor (Preprocessor & prep);
	~Anchor() {}

	Short getX() const { return x; }
	Short getY() const { return y; }

	bool operator == (const Anchor & a) const {
		return contourPoint == a.contourPoint && x == a.x && y == a.y;
	}

	bool operator != (const Anchor & a) const {
		return contourPoint != a.contourPoint || x != a.x || y != a.y;
	}

	ReferenceMemoryBlockPtr getTable (GlyphPtr glyph);

	static void writeContourPointAnchors (OpenTypeFont & font);
};

typedef smart_ptr <Anchor> AnchorPtr;
typedef std::vector <AnchorPtr> Anchors;


#endif	// LOOKUP_H
