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
	\file Glyph positioning table
*/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include "OTLayoutTable.h"
#include "OTLayoutInternal.h"
#include "OTException.h"
#include "OTTags.h"

using std::vector;
using util::smart_ptr;
using util::String;

namespace OpenType {

class PositioningValue {
	Short  xPlacement, yPlacement, xAdvance, yAdvance;
public:
	typedef enum {
		flXPlacement = 0x0001,
		flYPlacement = 0x0002,
		flXAdvance = 0x0004,
		flYAdvance = 0x0008,
		flXPlaDevice = 0x0010,
		flYPlaDevice = 0x0020,
		flXAdvDevice = 0x0040,
		flYAdvDevice = 0x0080
	} Flags;

	PositioningValue (Flags flags, MemoryPen &pen);
	virtual ~PositioningValue();
	
	void apply (OpenTypeText::iterator c) const;
};

typedef smart_ptr <PositioningValue> PosValuePtr;
typedef vector <PosValuePtr> PosValues;

PositioningValue::PositioningValue (Flags flags, MemoryPen &pen) {
	if (flags & flXPlacement)
		xPlacement = pen.readShort();
	else
		xPlacement = 0;

	if (flags & flYPlacement)
		yPlacement = pen.readShort();
	else
		yPlacement = 0;

	if (flags & flXAdvance)
		xAdvance = pen.readShort();
	else
		xAdvance = 0;

	if (flags & flYAdvance)
		yAdvance = pen.readShort();
	else
		yAdvance = 0;

	// Ignore device tables
	if (flags & flXPlaDevice)
		pen.readOffset();
	if (flags & flYPlaDevice)
		pen.readOffset();
	if (flags & flXAdvDevice)
		pen.readOffset();
	if (flags & flYAdvDevice)
		pen.readOffset();
}

PositioningValue::~PositioningValue() {}

void PositioningValue::apply (OpenTypeText::iterator c) const {
	(*c)->move (xPlacement, yPlacement, xAdvance, yAdvance);
}

/*** Anchor ***/

// The Anchor base class is defined in LayoutTable.h

Anchor::Anchor (MemoryPen &pen) {
	// format = 1, 2, 3
	pen.readUShort();
	x = pen.readShort();
	y = pen.readShort();
}

Anchor::~Anchor() {}

Short Anchor::getX() const {
	return x;
}

Short Anchor::getY() const {
	return y;
}

class SimpleAnchor : public Anchor {
public:
	SimpleAnchor (MemoryPen pen) : Anchor (pen) {}
	virtual ~SimpleAnchor() {}

	virtual UShort getContourPoint() const { return 0xFFFF; }
};

class PointAnchor : public Anchor {
	UShort contourPoint;
public:
	PointAnchor (MemoryPen pen);
	virtual ~PointAnchor();

	virtual UShort getContourPoint() const;
};

PointAnchor::PointAnchor (MemoryPen pen) : Anchor (pen) {
	contourPoint = pen.readUShort();
}

PointAnchor::~PointAnchor() {}

UShort PointAnchor::getContourPoint() const {
	return contourPoint;
}

AnchorPtr getAnchor (MemoryPen pen) {
	UShort format = MemoryPen (pen).readUShort();
	switch (format) {
	case 1:
		// Design units
	case 3:
		// Design units and device table (device table is not used)
		return new SimpleAnchor (pen);
	case 2:
		// Design units and contour point
		return new PointAnchor (pen);
	default:
		throw Exception ("Invalid anchor format " + String (format));
	}
}

/*** SinglePosLookup ***/

class SinglePosLookup1 : public LookupSubTable {
	CoveragePtr coverage;
	PosValuePtr value;
public:
	SinglePosLookup1 (MemoryPen pen, OpenTypeFont &font);
	~SinglePosLookup1();

	virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
		OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
};

SinglePosLookup1::SinglePosLookup1 (MemoryPen pen, OpenTypeFont &font) {
	MemoryPen start = pen;
	// format id = 1
	pen.readUShort();
	coverage = getCoverageTable (start + pen.readOffset(), font);
	PositioningValue::Flags flags = (PositioningValue::Flags) pen.readUShort();
	value = new PositioningValue (flags, pen);
}

SinglePosLookup1::~SinglePosLookup1() {}

void SinglePosLookup1::apply (OpenTypeText::iterator begin, OpenTypeText::iterator &current,
							  OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	if (coverage->isCovered ((*current)->getGlyphId(), NULL))
		value->apply (current);
	current ++;
}

class SinglePosLookup2 : public LookupSubTable {
	CoveragePtr coverage;
	PosValues values;
public:
	SinglePosLookup2 (MemoryPen pen, OpenTypeFont &font);
	~SinglePosLookup2();
	virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
		OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
};

SinglePosLookup2::SinglePosLookup2 (MemoryPen pen, OpenTypeFont &font) {
	MemoryPen start = pen;
	// format id = 2
	pen.readUShort();
	coverage = getCoverageTable (start + pen.readOffset(), font);
	PositioningValue::Flags valueFormat = (PositioningValue::Flags) pen.readUShort();
	UShort valueNum = pen.readUShort();
	if (valueNum != coverage->getGlyphNum())
		throw Exception (
			"Number of glyphs in coverage table does not equal number in single positioning lookup");
	for (UShort i = 0; i < valueNum; i ++)
		values.push_back (new PositioningValue (valueFormat, pen));
}

SinglePosLookup2::~SinglePosLookup2() {}

void SinglePosLookup2::apply (OpenTypeText::iterator begin, OpenTypeText::iterator &current,
							  OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	UShort index;
	if (coverage->isCovered ((*current)->getGlyphId(), &index))
		values[index]->apply (current);
	current ++;
}

/*** PairPosLookup ***/

class PairPosLookup1 : public LookupSubTable {
	typedef struct {
		GlyphId secondGlyph;
		PosValuePtr v1, v2;
	} PairValue;
	typedef vector <PairValue> PairSet;
	typedef vector <PairSet> PairSets;
	PairSets pairSets;

	CoveragePtr coverage;
	bool secondEmpty;
public:
	PairPosLookup1 (MemoryPen pen, OpenTypeFont &font);
	~PairPosLookup1();
	virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
		OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
};

PairPosLookup1::PairPosLookup1 (MemoryPen pen, OpenTypeFont &font) {
	MemoryPen start = pen;
	// format = 1
	pen.readUShort();
	coverage = getCoverageTable (start + pen.readOffset(), font);
	PositioningValue::Flags format1, format2;
	format1 = (PositioningValue::Flags) pen.readUShort();
	format2 = (PositioningValue::Flags) pen.readUShort();
	secondEmpty = format2 == 0;

	UShort pairSetNum = pen.readUShort();
	if (pairSetNum != coverage->getGlyphNum())
		throw Exception (
			"Number of glyphs in coverage table does not equal number in single positioning lookup");

	for (UShort i = 0; i < pairSetNum; i ++) {
		MemoryPen pairSetPen = start + pen.readOffset();
		PairSet pairSet;
		UShort pairValueNum = pairSetPen.readUShort();
		for (UShort j = 0; j < pairValueNum; j ++) {
			PairValue value;
			value.secondGlyph = pairSetPen.readUShort();
			value.v1 = new PositioningValue (format1, pairSetPen);
			value.v2 = new PositioningValue (format2, pairSetPen);
			pairSet.push_back (value);
		}
		pairSets.push_back (pairSet);
	}
}

PairPosLookup1::~PairPosLookup1() {}

void PairPosLookup1::apply (OpenTypeText::iterator begin, OpenTypeText::iterator &current,
							OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	UShort index;
	if (coverage->isCovered ((*current)->getGlyphId(), &index)) {
		OpenTypeText::iterator second = current;
		second ++;
		if (second != scopeEnd) {
			const PairSet &set = pairSets [index];
			PairSet::const_iterator s;
			for (s = set.begin(); s !=  set.end(); ++ s) {
				if (s->secondGlyph == (*second)->getGlyphId()) {
					// Found the sequence
					s->v1->apply (current);
					s->v2->apply (second);
					if (!secondEmpty)
						// Skip next character for this lookup
						current ++;
					break;
				}
			}
		}
	}
	current ++;
}

class PairPosLookup2 : public LookupSubTable {
	typedef struct {
		PosValuePtr v1, v2;
	} Values;
	typedef vector <Values> Class1Set;
	typedef vector <Class1Set> PairSets;
	PairSets pairSets;

	ClassDefPtr class1, class2;

	CoveragePtr coverage;
	bool secondEmpty;
public:
	PairPosLookup2 (MemoryPen pen, OpenTypeFont &font);
	~PairPosLookup2();

	virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
		OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
};

PairPosLookup2::PairPosLookup2 (MemoryPen pen, OpenTypeFont &font) {
	MemoryPen start = pen;
	// format = 2
	pen.readUShort();
	coverage = getCoverageTable (start + pen.readOffset(), font);
	PositioningValue::Flags format1, format2;
	format1 = (PositioningValue::Flags) pen.readUShort();
	format2 = (PositioningValue::Flags) pen.readUShort();
	secondEmpty = format2 == 0;

	class1 = getClassDefTable (start + pen.readOffset());
	class2 = getClassDefTable (start + pen.readOffset());

	UShort class1Count = pen.readUShort();
	UShort class2Count = pen.readUShort();

	if (class1Count != class1->getClassNum() || class2Count != class2->getClassNum())
		throw Exception (
			"Number of glyphs in coverage table does not equal number in single positioning lookup");

	for (UShort i = 0; i < class1Count; i ++) {
		Class1Set set;
		for (UShort j = 0; j < class2Count; j ++) {
			Values v;
			v.v1 = new PositioningValue (format1, pen);
			v.v2 = new PositioningValue (format2, pen);
			set.push_back (v);
		}
		pairSets.push_back (set);
	}
}

PairPosLookup2::~PairPosLookup2() {}

void PairPosLookup2::apply (OpenTypeText::iterator begin, OpenTypeText::iterator &current,
							OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	if (coverage->isCovered ((*current)->getGlyphId(), NULL)) {
		UShort index1 = class1->getClass ((*current)->getGlyphId());
		OpenTypeText::iterator second = current;
		second ++;
		if (second != scopeEnd) {
			UShort index2 = class2->getClass ((*second)->getGlyphId());
			const Values &v = (pairSets [index1]) [index2];
			v.v1->apply (current);
			v.v2->apply (second);
			if (!secondEmpty)
				// Skip next character for this lookup
				current ++;
		}
	}
	current ++;
}

/*** MarkArray ***/

typedef struct {
	UShort markClass;
	AnchorPtr markAnchor;
} MarkRecord;

typedef vector <MarkRecord> MarkArray;

MarkArray getMarkArray (MemoryPen pen) {
	MemoryPen start = pen;
	MarkArray markArray;

	UShort markRecordNum = pen.readUShort();
	markArray.reserve (markRecordNum);
	for (UShort i = 0; i < markRecordNum; i++) {
		MarkRecord r;
		r.markClass = pen.readUShort();
		r.markAnchor = getAnchor (start + pen.readOffset());
		markArray.push_back (r);
	}

	return markArray;
}

/*** MarkToBaseLookup ***/

class MarkToBaseLookup1 : public LookupSubTable {
	typedef vector <AnchorPtr> BaseRecord;
	typedef vector <BaseRecord> BaseArray;

	CoveragePtr markCoverage;
	CoveragePtr baseCoverage;

	MarkArray markArray;
	BaseArray baseArray;

	bool markToMark;
public:
	MarkToBaseLookup1 (MemoryPen pen, OpenTypeFont &font, bool aMarkToMark);
	virtual ~MarkToBaseLookup1();

	virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
		OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
};

MarkToBaseLookup1::MarkToBaseLookup1 (MemoryPen pen, OpenTypeFont &font, bool aMarkToMark)
: markToMark (aMarkToMark) {
	MemoryPen start = pen;
	// format = 1
	pen.readUShort();
	markCoverage = getCoverageTable (start + pen.readOffset(), font);
	baseCoverage = getCoverageTable (start + pen.readOffset(), font);

	UShort markClassNum = pen.readUShort();
	
	markArray = getMarkArray (start + pen.readOffset());
	if (markArray.size() != markCoverage->getGlyphNum())
		throw Exception (
			"Number of glyphs in mark array does not equal number of glyphs in coverage table");

	MemoryPen baseArrayPen = start + pen.readOffset();
	MemoryPen baseArrayStart = baseArrayPen;
	UShort baseNum = baseArrayPen.readUShort();
	if (baseNum != baseCoverage->getGlyphNum())
		throw Exception (
			"Number of glyphs in base array does not equal number of glyphs in coverage table");

	for (UShort i = 0; i < baseNum; i ++) {
		BaseRecord r;
		for (UShort j = 0; j < markClassNum; j ++)
			r.push_back (getAnchor (baseArrayStart + baseArrayPen.readOffset()));
		baseArray.push_back (r);
	}
}

MarkToBaseLookup1::~MarkToBaseLookup1() {}

void MarkToBaseLookup1::apply (OpenTypeText::iterator begin, OpenTypeText::iterator &mark,
							   OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	// We should never use the first character as a mark because there
	// is nothing in front of it.
	if (mark != begin) {
		UShort markIndex;
		if (markCoverage->isCovered ((*mark)->getGlyphId(), &markIndex)) {
			OpenTypeText::iterator base = mark;
			do {
				base --;
			} while (base != begin &&
				(!markToMark && (*base)->getGlyphClass() == OpenTypeChar::gcMark));

			UShort baseIndex;
			if (baseCoverage->isCovered ((*base)->getGlyphId(), &baseIndex)) {
				// Found mark and base
				const MarkRecord &markRecord = markArray.at (markIndex);
				AnchorPtr baseAnchor = (baseArray.at (baseIndex)).at (markRecord.markClass);
				(*mark)->attach (*markRecord.markAnchor, base, *baseAnchor);
			}
		}
	}
	mark ++;
}

/*** MarkToLigatureLookup ***/

class MarkToLigatureLookup1 : public LookupSubTable {
	typedef vector <AnchorPtr> ComponentAnchors;
	typedef vector <ComponentAnchors> LigatureMarkAnchors;
	typedef vector <LigatureMarkAnchors> LigatureAnchors;

	CoveragePtr markCoverage;
	CoveragePtr ligatureCoverage;

	MarkArray markArray;
	LigatureAnchors ligatureAnchors;
public:
	MarkToLigatureLookup1 (MemoryPen pen, OpenTypeFont &font);
	virtual ~MarkToLigatureLookup1();

	virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
		OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
};

MarkToLigatureLookup1::MarkToLigatureLookup1 (MemoryPen pen, OpenTypeFont &font)
{
	MemoryPen start = pen;
	// format = 1
	pen.readUShort();
	markCoverage = getCoverageTable (start + pen.readOffset(), font);
	ligatureCoverage = getCoverageTable (start + pen.readOffset(), font);

	UShort markClassNum = pen.readUShort();
	
	markArray = getMarkArray (start + pen.readOffset());
	if (markArray.size() != markCoverage->getGlyphNum())
		throw Exception (
			"Number of glyphs in mark array does not equal number of glyphs in coverage table");

	MemoryPen ligatureArrayPen = start + pen.readOffset();
	MemoryPen ligatureArrayStart = ligatureArrayPen;
	UShort ligatureNum = ligatureArrayPen.readUShort();
	if (ligatureNum != ligatureCoverage->getGlyphNum())
		throw Exception (
			"Number of glyphs in ligature array does not equal number of glyphs in coverage table");

	for (UShort i = 0; i < ligatureNum; i ++) {
		LigatureMarkAnchors ligatureMarkAnchors;
		MemoryPen ligatureAttachPen = ligatureArrayStart + ligatureArrayPen.readOffset();
		MemoryPen ligatureAttachStart = ligatureAttachPen;
		for (UShort componentNum = ligatureAttachPen.readUShort(); componentNum; componentNum --) {
			ComponentAnchors componentAnchors;
			for (UShort j = 0; j < markClassNum; j ++) {
				Offset anchorOffset = ligatureAttachPen.readOffset();
				if (anchorOffset)
					componentAnchors.push_back (getAnchor (ligatureAttachStart +
						anchorOffset));
				else
					componentAnchors.push_back (NULL);
			}
			ligatureMarkAnchors.push_back (componentAnchors);
		}
		ligatureAnchors.push_back (ligatureMarkAnchors);
	}
}

MarkToLigatureLookup1::~MarkToLigatureLookup1() {}

void MarkToLigatureLookup1::apply (OpenTypeText::iterator begin, OpenTypeText::iterator &mark,
								   OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	// We should never use the first character as a mark because there
	// is nothing in front of it.
	if (mark != begin) {
		UShort markIndex;
		if (markCoverage->isCovered ((*mark)->getGlyphId(), &markIndex)) {
			OpenTypeText::iterator ligature = mark;
			do {
				ligature --;
			} while (ligature != begin &&
				(*ligature)->getGlyphClass() == OpenTypeChar::gcMark);

			UShort ligatureIndex;
			if (ligatureCoverage->isCovered ((*ligature)->getGlyphId(), &ligatureIndex)) {
				// Found ligature and base
				assert (markIndex < markArray.size());
				const MarkRecord &markRecord = markArray.at (markIndex);

				UShort appliesTo = (*mark)->getAppliesTo();
				const LigatureMarkAnchors &ligatureMarkAnchors = ligatureAnchors.at (ligatureIndex);
				if (appliesTo >= ligatureMarkAnchors.size())
					throw Exception ("Ligature component count " + String (ligatureMarkAnchors.size()) +
						" too low to attach mark " + String (appliesTo) + " components back");
				AnchorPtr ligatureAnchor = (ligatureMarkAnchors.at (ligatureMarkAnchors.size() - appliesTo - 1))
					.at (markRecord.markClass);

				// Ligature anchors may be NULL as well
				if (ligatureAnchor)
					(*mark)->attach (*markRecord.markAnchor, ligature, *ligatureAnchor);
			}
		}
	}
	mark ++;
}

/*** GPOSTable ***/

smart_ptr <LookupSubTable> newGPOSSubTable (UShort type, UShort subTableFormat, MemoryPen pen,
										   const LookupList &lookupList, OpenTypeFont &font) {
	switch (type) {
	case 1:
		// Single positioning
		switch (subTableFormat) {
		case 1:
			return new SinglePosLookup1 (pen, font);
		case 2:
			return new SinglePosLookup2 (pen, font);
		default:
			return NULL;
		}

	case 2:
		// Pair positioning
		switch (subTableFormat) {
		case 1:
			return new PairPosLookup1 (pen, font);
		case 2:
			return new PairPosLookup2 (pen, font);
		default:
			return NULL;
		}

	case 4:
		// Mark to base attachment
		switch (subTableFormat) {
		case 1:
			return new MarkToBaseLookup1 (pen, font, false);
		default:
			return NULL;
		}

	case 5:
		// Mark to ligature attachment
		switch (subTableFormat) {
		case 1:
			return new MarkToLigatureLookup1 (pen, font);
		default:
			return NULL;
		}

	case 6:
		// Mark to mark attachment
		switch (subTableFormat) {
		case 1:
			return new MarkToBaseLookup1 (pen, font, true);
		default:
			return NULL;
		}

	case 7:
		// Contextual positioning
		switch (subTableFormat) {
		case 3:
			return new ContextLookup3 (pen, lookupList, font);
		default:
			return NULL;
		}

	case 8:
		// Chained contextual positioning
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

GPOSTable::GPOSTable (OpenTypeFont &aFont, MemoryBlockPtr aMemory) :
LayoutTable (aFont, aMemory, newGPOSSubTable) {}

GPOSTable::~GPOSTable() {}

Tag GPOSTable::getTag() const {
	return GPOSTag;
}

} // end namespace OpenType
