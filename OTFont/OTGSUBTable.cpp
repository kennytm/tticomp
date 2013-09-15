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
	\file Glyph substitution table
*/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include "OpenTypeFont.h"
#include "OTLayoutTable.h"
#include "OTLayoutInternal.h"
#include "OTException.h"
#include "OTTags.h"

using std::vector;
using util::smart_ptr;
using util::String;

namespace OpenType {

/*** SingleSubstLookup ***/

class SingleSubstLookup1 : public LookupSubTable {
	CoveragePtr coverage;
	Short delta;
public:
	SingleSubstLookup1 (MemoryPen pen, OpenTypeFont &font);
	virtual ~SingleSubstLookup1();
	virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
		OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
};

SingleSubstLookup1::SingleSubstLookup1 (MemoryPen pen, OpenTypeFont &font) {
	MemoryPen start = pen;
	// format id = 1
	pen.readUShort();
	coverage = getCoverageTable (start + pen.readOffset(), font);
	delta = pen.readShort();
	if (ULong (coverage->getMaxGlyphId() + delta >= font.getGlyphNum()))
		throw Exception ("Single substitution resulting glyph ids out of range: " +
			String (coverage->getMaxGlyphId()) + " + " + String (delta) + " >= " +
			String (font.getGlyphNum()));
}

SingleSubstLookup1::~SingleSubstLookup1() {}

void SingleSubstLookup1::apply (OpenTypeText::iterator begin, OpenTypeText::iterator &current,
								OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	if (coverage->isCovered ((*current)->getGlyphId(), NULL))
		current = current.getText().replace (current, (*current)->getGlyphId() + delta);
	current ++;
}


class SingleSubstLookup2 : public LookupSubTable {
	CoveragePtr coverage;
	vector <GlyphId> substitutes;
public:
	SingleSubstLookup2 (MemoryPen pen, OpenTypeFont &font);
	~SingleSubstLookup2();
	virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
		OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
};

SingleSubstLookup2::SingleSubstLookup2 (MemoryPen pen, OpenTypeFont &font) {
	MemoryPen start = pen;
	// format id = 2
	pen.readUShort();
	coverage = getCoverageTable (start + pen.readOffset(), font);
	UShort glyphNum = pen.readUShort();
	if (glyphNum != coverage->getGlyphNum())
		throw Exception ("Invalid number of glyphs in single substitution");
	GlyphId maxGlyphIndex = font.getGlyphNum();
	for (UShort i = 0; i < glyphNum; i ++) {
		GlyphId curGlyph = pen.readUShort();
		if (curGlyph >= maxGlyphIndex)
			throw Exception ("Invalid glyph index in single substitution: " +
				String (curGlyph));
		substitutes.push_back (curGlyph);
	}
}

SingleSubstLookup2::~SingleSubstLookup2() {}

void SingleSubstLookup2::apply (OpenTypeText::iterator begin, OpenTypeText::iterator &current,
								OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	UShort index;
	if (coverage->isCovered ((*current)->getGlyphId(), &index)) {
		assert (index < substitutes.size());
		current = current.getText().replace (current, substitutes [index]);
	}
	current ++;
}

/*** MultipleSubstLookup ****/

class MultipleSubstLookup1 : public LookupSubTable {
	CoveragePtr coverage;
	vector <GlyphIds> sequences;
public:
	MultipleSubstLookup1 (MemoryPen pen, OpenTypeFont &font);
	~MultipleSubstLookup1();
	virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
		OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
};

MultipleSubstLookup1::MultipleSubstLookup1 (MemoryPen pen, OpenTypeFont &font) {
	MemoryPen start = pen;
	// format id = 1
	pen.readUShort();
	coverage = getCoverageTable (start + pen.readOffset(), font);
	UShort sequenceNum = pen.readUShort();
	if (sequenceNum != coverage->getGlyphNum())
		throw Exception ("Invalid number of glyphs in alternate substitution");
	for (UShort i = 0; i < sequenceNum; i ++) {
		GlyphIds sequence;
		MemoryPen sequencePen = start + pen.readOffset();
		GlyphId glyphNum = sequencePen.readUShort();
		GlyphId maxGlyphIndex = font.getGlyphNum();
		for (UShort j = 0; j < glyphNum; j ++) {
			GlyphId curGlyph = sequencePen.readUShort();
			if (curGlyph >= maxGlyphIndex)
				throw Exception ("Invalid glyph index in multiple substitution: " +
					String (curGlyph));
			sequence.push_back (curGlyph);
		}
		sequences.push_back (sequence);
	}
}

MultipleSubstLookup1::~MultipleSubstLookup1() {}

void MultipleSubstLookup1::apply (OpenTypeText::iterator begin, OpenTypeText::iterator &current,
								  OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	UShort index;
	if (coverage->isCovered ((*current)->getGlyphId(), &index)) {
		OpenTypeText::iterator next = current;
		next ++;
		current.getText().replace (current, sequences [index]);
		current = next;
	} else
		current ++;
}

/*** AlternateSubstLookup ***/

class AlternateSubstLookup1 : public LookupSubTable {
	CoveragePtr coverage;
	vector <GlyphIds> sequences;
public:
	AlternateSubstLookup1 (MemoryPen pen, OpenTypeFont &font);
	~AlternateSubstLookup1();
	virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
		OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
};

AlternateSubstLookup1::AlternateSubstLookup1 (MemoryPen pen, OpenTypeFont &font) {
	MemoryPen start = pen;
	// format id = 1
	pen.readUShort();
	coverage = getCoverageTable (start + pen.readOffset(), font);
	UShort sequenceNum = pen.readUShort();
	if (sequenceNum != coverage->getGlyphNum())
		throw Exception ("Invalid number of glyphs in alternate substitution");
	for (UShort i = 0; i < sequenceNum; i ++) {
		GlyphIds sequence;
		MemoryPen sequencePen = start + pen.readOffset();
		UShort glyphNum = sequencePen.readUShort();
		UShort maxGlyphIndex = font.getGlyphNum();
		for (UShort j = 0; j < glyphNum; j ++) {
			GlyphId curGlyph = sequencePen.readUShort();
			if (curGlyph >= maxGlyphIndex)
				throw Exception ("Invalid glyph index in multiple substitution: " +
					String (curGlyph));
			sequence.push_back (curGlyph);
		}
		sequences.push_back (sequence);
	}
}

AlternateSubstLookup1::~AlternateSubstLookup1() {}

void AlternateSubstLookup1::apply (OpenTypeText::iterator begin, OpenTypeText::iterator &current,
								   OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	// do nothing
	current ++;
}

/*** LigatureSubstLookup ***/

class LigatureSubstLookup : public LookupSubTable {
	struct Ligature {
		GlyphId ligatureGlyph;
			// components starting with the second component
		GlyphIds components;
	};
	typedef vector <Ligature> LigatureSet;
	typedef vector <LigatureSet> LigatureSets;

	CoveragePtr coverage;
	LigatureSets ligatureSets;
public:
	LigatureSubstLookup (MemoryPen pen, OpenTypeFont &font);
	~LigatureSubstLookup();
	virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
		OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
};

LigatureSubstLookup::LigatureSubstLookup (MemoryPen pen, OpenTypeFont &font) {
	MemoryPen start = pen;
	// format id = 1
	pen.readUShort();
	coverage = getCoverageTable (start + pen.readOffset(), font);
	UShort ligatureSetNum = pen.readUShort();
	if (ligatureSetNum != coverage->getGlyphNum())
		throw Exception ("Invalid number of ligatures in single substitution");

	GlyphId maxGlyphIndex = font.getGlyphNum();

	for (UShort i = 0; i < ligatureSetNum; i ++) {
		LigatureSet set;
		MemoryPen setPen = start + pen.readOffset();
		MemoryPen setStart = setPen;
		UShort ligatureNum = setPen.readUShort();
		for (UShort j = 0; j < ligatureNum; j ++) {
			Ligature ligature;
			MemoryPen ligaturePen = setStart + setPen.readOffset();
			ligature.ligatureGlyph = ligaturePen.readUShort();
			UShort componentNum = ligaturePen.readUShort();
				for (UShort k = 0; k < componentNum - 1; k ++) {
				GlyphId curGlyph = ligaturePen.readGlyphId();
				if (curGlyph >= maxGlyphIndex)
					throw Exception ("Invalid glyph index in multiple substitution: " +
						String (curGlyph));
				ligature.components.push_back (curGlyph);
			}
			set.push_back (ligature);
		}
		ligatureSets.push_back (set);
	}
}

LigatureSubstLookup::~LigatureSubstLookup() {}

void LigatureSubstLookup::apply (OpenTypeText::iterator begin, OpenTypeText::iterator &current,
								 OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	UShort index;
	if (coverage->isCovered ((*current)->getGlyphId(), &index)) {
		assert (index < ligatureSets.size());
		const LigatureSet &set = ligatureSets [index];
		LigatureSet::const_iterator l;
		for (l = set.begin(); l != set.end(); l ++) {
			OpenTypeText::iterator compChar = current;
			compChar ++;
			GlyphIds::const_iterator component = l->components.begin();
			while (component != l->components.end() && compChar != scopeEnd) {
				if (*component != (*compChar)->getGlyphId())
					break;
				component ++;
				compChar ++;
			}
			if (component == l->components.end()) {
				// Found the right components
				GlyphIds newGlyphIds;
				current = current.getText().replace (current, compChar, l->ligatureGlyph);
				break;
			}
		}
	}
	current ++;
}

/*** GSUBTable ***/

smart_ptr <LookupSubTable> newGSUBSubTable (UShort type, UShort subTableFormat, MemoryPen pen,
										   const LookupList & lookupList, OpenTypeFont &font) {
	switch (type) {
	case 1:
		// Single substitution
		switch (subTableFormat) {
		case 1:
			// Substitute by delta
			return new SingleSubstLookup1 (pen, font);
		case 2:
			return new SingleSubstLookup2 (pen, font);
		default:
			return NULL;
		}

	case 2:
		// Multiple substitution
		switch (subTableFormat) {
		case 1:
			return new MultipleSubstLookup1 (pen, font);
		default:
			return NULL;
		}

	case 3:
		// Alternate substitution
		switch (subTableFormat) {
		case 1:
			return new AlternateSubstLookup1 (pen, font);
		default:
			return NULL;
		}

	case 4:
		// Ligature substitution
		switch (subTableFormat) {
		case 1:
			// Only format defined
			return new LigatureSubstLookup (pen, font);
		default:
			return NULL;
		}

	case 5:
		// Context substitution
		switch (subTableFormat) {
		case 3:
			return new ContextLookup3 (pen, lookupList, font);
		default:
			return NULL;
		}

	case 6:
		// Chaining context substitution
		switch (subTableFormat) {
		case 3:
			return new ChainingContextLookup3 (pen, lookupList, font);
		default:
			return NULL;
		}

	default:
		return NULL;
	}
}

GSUBTable::GSUBTable (OpenTypeFont &aFont, MemoryBlockPtr aMemory) :
LayoutTable (aFont, aMemory, newGSUBSubTable) {
}

GSUBTable::~GSUBTable() {}

Tag GSUBTable::getTag() const {
	return GSUBTag;
}

} // end namespace OpenType
