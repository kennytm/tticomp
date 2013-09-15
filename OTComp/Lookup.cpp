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

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

#include "../OTFont/OTGlyph.h"

#include "Lookup.h"
#include "GlyphList.h"
#include "Identifier.h"

using std::endl;
using std::vector;
using std::list;
using std::sort;
using std::stable_sort;
using std::pair;

void checkType (PreprocessorPosition & pos, LookupSubTablePtr table,
				LookupSubTable::LookupType type)
{
	if (table->type == 0)
		table->type = type;
	else {
		if (type != table->type) {
			pos.prep.startError (pos) << "Lookup types may not be mixed within lookup." << endl;
			throw pos;
		}
	}
}

Short getInt (Preprocessor &prep) {
	String buffer = prep.get();

	bool negative;
	if (buffer == "-") {
		negative = true;
		buffer = prep.get();
	} else {
		negative = false;
		if (buffer == "+")
			buffer = prep.get();
	}

	UShort number = 0;
	for (String::const_iterator i = buffer.begin(); i != buffer.end(); i ++) {
		if (*i < '0' || *i > '9') {
			prep.startError () << "Number expected instead of \"" << buffer << "\"." << endl;
			return number;
		}
		number *= 10;
		number += (*i - '0');
	}

	if (negative)
		return -number;
	else
		return number;
}

/*** Position ***/

class Position {
public:
	enum Format {
		None = 0,
		XPlacement = 0x0001,
			YPlacement = 0x0002,
			XAdvance = 0x0004,
			YAdvance = 0x0008
	};
private:
	Short x, y, xAdvance, yAdvance;
	Format format;

public:
	Position (Preprocessor & prep);
	Position ();
	~Position() {}

	Format getFormat() const { return format; }
	void write (MemoryWritePen &pen);

	bool operator < (const Position & p) const;
	bool operator > (const Position & p) const;
	bool operator == (const Position & p) const;
	bool operator != (const Position & p) const;
};

typedef smart_ptr <Position> PositionPtr;

Position::Position() : x (0), y (0), xAdvance (0), yAdvance (0), format (None) {}

Position::Position (Preprocessor &prep) : x (0), y (0), xAdvance (0), yAdvance (0), format (None) {
	String buffer = prep.peek();
	if (buffer != ",") {
		format = (Format) (format | XPlacement);
		x = getInt (prep);
		buffer = prep.peek();
		if (buffer != ",")
			return;
	}
	prep.deleteToken();

	buffer = prep.peek();
	if (buffer != ",") {
		format = (Format) (format | YPlacement);
		y = getInt (prep);
		buffer = prep.peek();
		if (buffer != ",")
			return;
	}
	prep.deleteToken();

	buffer = prep.peek();
	if (buffer != ",") {
		format = (Format) (format | XAdvance);
		xAdvance = getInt (prep);
		buffer = prep.peek();
		if (buffer != ",")
			return;
	}
	prep.deleteToken();

	buffer = prep.peek();
	if (buffer != ",") {
		format = (Format) (format | YAdvance);
		yAdvance = getInt (prep);
		buffer = prep.peek();
		if (buffer != ",")
			return;
	}
	prep.deleteToken();
}

void Position::write (MemoryWritePen &pen) {
	if (format & XPlacement)
		pen.writeShort (x);
	if (format & YPlacement)
		pen.writeShort (y);
	if (format & XAdvance)
		pen.writeShort (xAdvance);
	if (format & YAdvance)
		pen.writeShort (yAdvance);
}

bool Position::operator < (const Position & p) const {
	assert (format == p.format);
	if (x < p.x)
		return true;
	if (x > p.x)
		return false;
	if (y < p.y)
		return true;
	if (y > p.y)
		return false;
	if (xAdvance < p.xAdvance)
		return true;
	if (xAdvance > p.xAdvance)
		return false;
	return yAdvance < p.yAdvance;
}

bool Position::operator > (const Position & p) const {
	assert (format == p.format);
	if (x > p.x)
		return true;
	if (x < p.x)
		return false;
	if (y > p.y)
		return true;
	if (y < p.y)
		return false;
	if (xAdvance > p.xAdvance)
		return true;
	if (xAdvance < p.xAdvance)
		return false;
	return yAdvance > p.yAdvance;
}

bool Position::operator == (const Position & p) const {
	assert (format == p.format);
	return x == p.x && y == p.y && xAdvance == p.xAdvance && yAdvance == p.yAdvance;
}

bool Position::operator != (const Position & p) const {
	assert (format == p.format);
	return x != p.x || y != p.y || xAdvance != p.xAdvance || yAdvance != p.yAdvance;
}

/*** Anchor ***/

Anchor::ContourPointAnchors Anchor::contourPointAnchors;

Anchor::Anchor (Preprocessor & prep) : pos (prep.getCurrentPosition()), x (0), y (0) {
	if (!isExpected (prep, "<"))
		return;
	String buffer = prep.peek();
	if (buffer == ":") {
		contourPoint = true;
		prep.deleteToken();
	} else
		contourPoint = false;

	x = getInt (prep);
	isExpected (prep, ",");
	y = getInt (prep);
	isExpected (prep, ">");
}

ReferenceMemoryBlockPtr Anchor::getTable (GlyphPtr glyph) {
	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);

	if (!contourPoint || glyph->isEmpty()) {
		// Format 1: only coordinates
		pen.writeUShort (1);
		pen.writeShort (x);
		pen.writeShort (y);
	} else {
		// Format 2: coordinates and contour point
		pen.writeUShort (2);
		pen.writeShort (x);
		pen.writeShort (y);
		// Contour point index should be written now but we do not know it yet!
		// So remember to write it later.
		ContourPointAnchor a;
		a.glyph = glyph;
		a.x = x;
		a.y = y;
		a.pointIndexPen = pen;
		pen.writeUShort (0);
		contourPointAnchors.push_back (a);
	}

	return memory;
}

bool Anchor::compareContourPointAnchors (const ContourPointAnchor &a1, const ContourPointAnchor &a2) {
	return a1.glyph->getCompositeDepth() < a2.glyph->getCompositeDepth();
}

void Anchor::writeContourPointAnchors (OpenTypeFont &font) {
	// Make sure that the contour points are ordered in a predictable way
	// (this is important for instructing), so use stable_sort
	stable_sort (contourPointAnchors.begin(), contourPointAnchors.end(), compareContourPointAnchors);
	UShort anchorPointGlyph = 0;

	for (ContourPointAnchors::iterator a = contourPointAnchors.begin(); a != contourPointAnchors.end(); a ++) {
		Contours contours = a->glyph->getContours();
		Contours::iterator contour;
		UShort pointIndex = 0;
		bool found = false;
		for (contour = contours.begin(); contour != contours.end(); contour ++) {
			Contour::iterator point = contour->begin();
			do {
				if (point->x == a->x && point->y == a->y) {
					// Found the point
					a->pointIndexPen.writeUShort (pointIndex);
					found = true;
				} else {
					point ++;
					pointIndex ++;
				}
			} while (point != contour->begin() && !found);
			if (found)
				break;
		}
		if (!found) {
			// No right point was found; a new point has to be inserted.
			const Glyph::BoundingBox & boundingBox = a->glyph->getBoundingBox();
			if (boundingBox.xMin > a->x) {
				// left side bearing would be broken so fix this
				HorMetric hm = a->glyph->getHorMetric();
				hm.lsb -= boundingBox.xMin - a->x;
				a->glyph->setHorMetric (hm);
			}

			if (!a->glyph->isComposite()) {
				// Add contour point
				ContourPoint point;
				point.x = a->x;
				point.y = a->y;
				point.onCurve = true;
				Contour contour;
				contour.push (point);
				contours.push_back (contour);
				util::smart_ptr_cast <SimpleGlyph> (a->glyph)->setContours (contours);
				// Write index
				a->pointIndexPen.writeUShort (contours.getPointNum() - 1);
			} else {
				if (!anchorPointGlyph) {
					// Generate an glyph to be used as an anchor in composite glyphs
					ContourPoint newContourPoint;
					newContourPoint.x = newContourPoint.y = 0;
					newContourPoint.onCurve = true;
					Contour contour;
					contour.push (newContourPoint);
					Contours newContours;
					newContours.push_back (contour);

					HorMetric hm;
					hm.advanceWidth = hm.lsb = 0;
					anchorPointGlyph = font.addGlyph (new SimpleGlyph (font,
						"anchor_generated_by_OTComp", hm, newContours));
				}
				CompositeComponent::Scale scale;
				scale.xx = scale.yy = 1; //0x4000;
				scale.xy = scale.yx = 0;
				CompositeComponent::Translation translation = {a->x, a->y};

				ComponentPtr newComponent = new PositionedCompositeComponent (
					font, anchorPointGlyph, CompositeComponent::cfNone, scale, translation);
				util::smart_ptr_cast <CompositeGlyph> (a->glyph)->addComponent (newComponent);
				// Write index
				a->pointIndexPen.writeUShort (contours.getPointNum());
			}
		}
	}
}

// General structures

typedef vector <GlyphListPtr> GlyphSequence;
struct Substitution {
	GlyphSequence source;
	GlyphSequence dest;
};
typedef vector <Substitution> Substitutions;

typedef vector <PositionPtr> Positions;
struct Positioning {
	GlyphSequence source;
	Positions positions;
};
typedef vector <Positioning> Positionings;

bool compareSubstitutionsBySource (const Substitution &p1, const Substitution &p2) {
	return p1.source < p2.source;
}

bool compareSubstitutionByFirstSource (const Substitution &p1, const Substitution &p2) {
	return p1.source.front()->front() < p2.source.front()->front();
}

GlyphId getSubstitutionGlyphId (Substitutions::const_iterator i) {
	return i->source.front()->front();
}

GlyphId getGlyphIdsGlyphId (GlyphIds::const_iterator i) {
	return *i;
}

// Anchors

struct MarkAnchor {
	GlyphId glyph;
	AnchorPtr anchor;
};

typedef vector <MarkAnchor> MarkAnchors;

bool operator < (const MarkAnchor & a1, const MarkAnchor & a2) {
	return a1.glyph < a2.glyph;
}

struct BaseAnchor {
	GlyphId base, mark;
	Anchors anchors;
};

typedef vector <BaseAnchor> BaseAnchors;

bool operator < (const BaseAnchor & a1, const BaseAnchor & a2) {
	if (a1.base < a2.base)
		return true;
	if (a1.base > a2.base)
		return false;
	return a1.mark < a2.mark;
}

bool sameAnchors (const Anchors & a1, const Anchors & a2) {
	if (a1.size() != a2.size())
		return false;
	Anchors::const_iterator i1 = a1.begin();
	Anchors::const_iterator i2 = a2.begin();
	for (; i1 != a1.end(); i1++, i2 ++) {
		if (**i1 != **i2)
			return false;
	}
	return true;
}

// Single substitution structure

struct SingleSubstGlyph {
	GlyphId source;
	GlyphId dest;
	Short difference;
};
typedef vector <SingleSubstGlyph> SingleSubstGlyphs;

inline bool compareSingleSubstGlyphsByDifference (const SingleSubstGlyph & g1, const SingleSubstGlyph & g2) {
	if (g1.difference < g2.difference)
		return true;
	if (g1.difference > g2.difference)
		return false;
	return g1.source < g2.source;
}

inline bool compareSingleSubstGlyphsBySource (const SingleSubstGlyph & g1, const SingleSubstGlyph & g2) {
	if (g1.source < g2.source)
		return true;
	if (g1.source > g2.source)
		return false;
	return g1.dest < g2.dest;
}

GlyphId getSingleSubstGlyphId (SingleSubstGlyphs::const_iterator i) {
	return i->source;
}

ReferenceMemoryBlocks getSingleSubstitutionMemory (const Substitutions &substitutions, Preprocessor & prep,
												   OpenTypeFont & font)
{
	ReferenceMemoryBlocks subTables;
	SingleSubstGlyphs glyphs;
	glyphs.reserve (substitutions.size());
	for (Substitutions::const_iterator i = substitutions.begin(); i != substitutions.end(); i ++) {
		assert (i->source.size() == 1);
		assert (i->dest.size() == 1);
		assert (i->source.front()->size() == i->source.front()->size());
		GlyphList::const_iterator j = i->source.front()->begin();
		GlyphList::const_iterator k = i->dest.front()->begin();
		for (; j != i->source.front()->end(); j ++, k ++)
		{
			SingleSubstGlyph glyph;
			glyph.source = *j;
			glyph.dest = *k;
			glyph.difference = glyph.dest - glyph.source;
			glyphs.push_back (glyph);
		}
	}

	// Check for duplicates
	sort (glyphs.begin(), glyphs.end(), compareSingleSubstGlyphsBySource);
	GlyphIds duplicates;
	SingleSubstGlyphs::iterator j;
	for (j = glyphs.begin() + 1; j != glyphs.end();) {
		if ((j-1)->source == j->source) {
			duplicates.push_back (j->source);
			j = glyphs.erase (j);
		} else
			j ++;
	}
	if (!duplicates.empty()) {
		ostream & o = prep.startError();
		o << "Duplicate glyphs in input range: ";
		writeToStream (duplicates, o, font);
		o << endl;
	}

	sort (glyphs.begin(), glyphs.end(), compareSingleSubstGlyphsByDifference);

	SingleSubstGlyphs::iterator first = glyphs.begin();
	SingleSubstGlyphs::iterator last = first;
	while (last != glyphs.end()) {
		last ++;
		if (last == glyphs.end() || first->difference != last->difference) {
			// End of equal-difference range
			// See whether this should be encoded as a range of calculated glyph indices
			if (last-first >= 6 ||
				(last == glyphs.end() && first == glyphs.begin() && last-first >= 3))
			{
				// Write a table format 1 (delta value)
				ReferenceMemoryBlockPtr subTable (new ReferenceMemoryBlock);
				ReferenceMemoryPen pen (subTable);
				pen.writeUShort (1);
				// Create a coverage table
				// The glyphs in [first, last) are sorted by source.
				pen.writeReference (getCoverageTable (first, last, getSingleSubstGlyphId));
				pen.writeShort (first->difference);

				subTables.push_back (subTable);

				// Delete the encoded glyphs
				last = first = glyphs.erase (first, last);
			} else
				first = last;
		}
	}

	if (!glyphs.empty()) {
		// The rest of the glyphs should be encoded using format 2, i.e.
		// a list of replacement glyphs per glyph.
		sort (glyphs.begin(), glyphs.end(), compareSingleSubstGlyphsBySource);

		ReferenceMemoryBlockPtr subTable (new ReferenceMemoryBlock);
		ReferenceMemoryPen pen (subTable);
		pen.writeUShort (2);

		// The glyphs are sorted by source.
		pen.writeReference (getCoverageTable (glyphs.begin(), glyphs.end(), getSingleSubstGlyphId));
		pen.writeUShort (glyphs.size());
		for (j = glyphs.begin(); j != glyphs.end(); j ++)
			pen.writeUShort (j->dest);

		subTables.push_back (subTable);
	}
	return subTables;
}

/*** Multiple substitution ***/

ReferenceMemoryBlocks getMultipleSubstitutionMemory (Substitutions &substitutions, Preprocessor & prep,
													 OpenTypeFont & font)
{
	sort (substitutions.begin(), substitutions.end(), compareSubstitutionsBySource);

	// Check for duplicates
	GlyphIds duplicates;
	for (Substitutions::iterator j = substitutions.begin() + 1; j != substitutions.end();) {
		assert (j->source.size() == 1);
		assert (j->source.front()->size() == 1);
		if ((j-1)->source.front()->front() == j->source.front()->front()) {
			duplicates.push_back (j->source.front()->front());
			j = substitutions.erase (j);
		} else
			j ++;
	}

	if (!duplicates.empty()) {
		ostream & o = prep.startError();
		o << "Duplicate glyphs in input range: ";
		writeToStream (duplicates, o, font);
		o << endl;
	}

	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);

	// format
	pen.writeUShort (1);

	pen.writeReference (getCoverageTable (substitutions.begin(), substitutions.end(), getSubstitutionGlyphId));
	pen.writeUShort (substitutions.size());
	for (Substitutions::iterator i = substitutions.begin(); i != substitutions.end(); i ++) {
		ReferenceMemoryBlockPtr sequenceMemory (new ReferenceMemoryBlock);
		ReferenceMemoryPen sequencePen (sequenceMemory);
		sequencePen.writeUShort (i->dest.size());
		for (GlyphSequence::iterator j = i->dest.begin(); j != i->dest.end(); j ++) {
			assert ((*j)->size() == 1);
			sequencePen.writeGlyphId ((*j)->front());
		}
		pen.writeReference (sequenceMemory);
	}

	return ReferenceMemoryBlocks (1, memory);
}

/*** Alternate substitution ***/

ReferenceMemoryBlocks getAlternateSubstitutionMemory (Substitutions &substitutions, Preprocessor & prep,
													  OpenTypeFont & font)
{
	sort (substitutions.begin(), substitutions.end(), compareSubstitutionsBySource);

	// Check for duplicates
	GlyphIds duplicates;
	for (Substitutions::iterator j = substitutions.begin() + 1; j != substitutions.end();) {
		assert (j->source.size() == 1);
		assert (j->source.front()->size() == 1);
		if ((j-1)->source.front()->front() == j->source.front()->front()) {
			duplicates.push_back (j->source.front()->front());
			j = substitutions.erase (j);
		} else
			j ++;
	}

	if (!duplicates.empty()) {
		ostream & o = prep.startError();
		o << "Duplicate glyphs in input range: ";
		writeToStream (duplicates, o, font);
		o << endl;
	}

	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);

	// format
	pen.writeUShort (1);

	pen.writeReference (getCoverageTable (substitutions.begin(), substitutions.end(), getSubstitutionGlyphId));
	pen.writeUShort (substitutions.size());
	for (Substitutions::iterator i = substitutions.begin(); i != substitutions.end(); i ++) {
		ReferenceMemoryBlockPtr sequenceMemory (new ReferenceMemoryBlock);
		ReferenceMemoryPen sequencePen (sequenceMemory);
		assert (i->dest.front()->size() > 0);
		sequencePen.writeUShort (i->dest.front()->size());
		for (GlyphList::iterator j = i->dest.front()->begin(); j != i->dest.front()->end(); j ++) {
			sequencePen.writeGlyphId (*j);
		}
		pen.writeReference (sequenceMemory);
	}

	return ReferenceMemoryBlocks (1, memory);
}

/*** Ligature substitution ***/

ReferenceMemoryBlocks getLigatureSubstitutionMemory (Substitutions &substitutions, Preprocessor & prep,
													 OpenTypeFont & font)
{
	// stable_sort is used to make sure that any substitutions that start with
	// the same glyph are kept in the same order.
	stable_sort (substitutions.begin(), substitutions.end(), compareSubstitutionByFirstSource);

	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);

	// format
	pen.writeUShort (1);
	ReferenceMemoryPen coveragePen = pen;
	pen.writeUShort (0);
	pen.writeUShort (0);

	GlyphIds coveredGlyphs;
	Substitutions::iterator i = substitutions.begin();
	while (i != substitutions.end()) {
		// A ligature set contains all ligatures starting with the same component glyph.
		// E.g., ffi, fi, fl are all in the same set.
		coveredGlyphs.push_back (i->source.front()->front());

		ReferenceMemoryBlockPtr ligatureSet (new ReferenceMemoryBlock);
		ReferenceMemoryPen ligatureSetPen (ligatureSet);
		ReferenceMemoryPen ligatureNumPen = ligatureSetPen;
		ligatureSetPen.writeUShort (0);
		UShort ligatureNum = 0;
		Substitutions::iterator j = i;
		do {
			ReferenceMemoryBlockPtr ligature (new ReferenceMemoryBlock);
			ReferenceMemoryPen ligaturePen (ligature);
			// ligature glyph
			ligaturePen.writeGlyphId (j->dest.front()->front());
			// componentNum
			ligaturePen.writeUShort (j->source.size());
			// components except for the first one
			for (GlyphSequence::iterator glyph = j->source.begin() + 1; glyph != j->source.end(); glyph ++)
				ligaturePen.writeGlyphId ((*glyph)->front());

			ligatureSetPen.writeReference (ligature);
			ligatureNum ++;
			j ++;
		} while (j != substitutions.end() && j->source.front()->front() == i->source.front()->front());
		ligatureNumPen.writeUShort (ligatureNum);
		i = j;

		pen.writeReference (ligatureSet);
	}

	// Coverage table
	coveragePen.writeReference (getCoverageTable (
		coveredGlyphs.begin(), coveredGlyphs.end(), getGlyphIdsGlyphId));
	// ligatureSetNum
	coveragePen.writeUShort (coveredGlyphs.size());

	return ReferenceMemoryBlocks (1, memory);
}


/*** Single positioning ***/

struct SinglePosGlyph {
	GlyphId glyph;
	PositionPtr position;
};
typedef vector <SinglePosGlyph> SinglePosGlyphs;

bool compareSinglePosGlyphByGlyph (const SinglePosGlyph &g1, const SinglePosGlyph &g2) {
	return g1.glyph < g2.glyph;
}

bool compareSinglePosGlyphByPosition (const SinglePosGlyph &g1, const SinglePosGlyph &g2) {
	if (*g1.position < *g2.position)
		return true;
	if (*g1.position > *g2.position)
		return false;
	return g1.glyph < g2.glyph;
}

GlyphId getSinglePosGlyphId (SinglePosGlyphs::const_iterator i) {
	return i->glyph;
}

ReferenceMemoryBlocks getSinglePositioningMemory (const Positionings &positionings, Preprocessor & prep,
												  OpenTypeFont & font)
{
	ReferenceMemoryBlocks subTables;
	SinglePosGlyphs glyphs;
	for (Positionings::const_iterator i = positionings.begin(); i != positionings.end(); i++) {
		assert (i->source.size() == 1);
		assert (i->positions.size() == 1);
		for (GlyphIds::const_iterator g = i->source.front()->begin(); g != i->source.front()->end(); g ++) {
			SinglePosGlyph glyph;
			glyph.glyph = *g;
			glyph.position = i->positions.front();
			glyphs.push_back (glyph);
		}
	}

	sort (glyphs.begin(), glyphs.end(), compareSinglePosGlyphByGlyph);

	GlyphIds duplicates;
	SinglePosGlyphs::iterator j;
	for (j = glyphs.begin() + 1; j != glyphs.end();) {
		if ((j-1)->glyph == j->glyph) {
			duplicates.push_back (j->glyph);
			j = glyphs.erase (j);
		} else
			j ++;
	}
	if (!duplicates.empty()) {
		ostream & o = prep.startError();
		o << "Duplicate glyphs in input range: ";
		writeToStream (duplicates, o, font);
		o << endl;
	}

	sort (glyphs.begin(), glyphs.end(), compareSinglePosGlyphByPosition);

	SinglePosGlyphs::iterator first = glyphs.begin();
	SinglePosGlyphs::iterator last = first;
	while (last != glyphs.end()) {
		last ++;
		if (last == glyphs.end() || first->position != last->position) {
			// End of equal-difference range
			// See whether this should be encoded as a range of calculated glyph indices
			if (last-first >= 6 ||
				(last == glyphs.end() && first == glyphs.begin() && last-first >= 1))
			{
				// Write a table format 1 (delta value)
				ReferenceMemoryBlockPtr subTable (new ReferenceMemoryBlock);
				ReferenceMemoryPen pen (subTable);

				pen.writeUShort (1);
				// The glyphs in [first, last) are sorted by source.
				pen.writeReference (getCoverageTable (first, last, getSinglePosGlyphId));
				pen.writeUShort (first->position->getFormat());
				first->position->write (pen);

				subTables.push_back (subTable);

				// Delete the encoded glyphs
				last = first = glyphs.erase (first, last);
			} else
				first = last;
		}
	}

	if (!glyphs.empty()) {
		// The rest of the glyphs should be encoded using format 2, i.e.
		// a list of replacement glyphs per glyph.
		sort (glyphs.begin(), glyphs.end(), compareSinglePosGlyphByGlyph);

		ReferenceMemoryBlockPtr subTable (new ReferenceMemoryBlock);
		ReferenceMemoryPen pen (subTable);
		pen.writeUShort (2);

		// The glyphs are sorted by source.
		pen.writeReference (getCoverageTable (glyphs.begin(), glyphs.end(), getSinglePosGlyphId));
		pen.writeUShort (glyphs.front().position->getFormat());
		pen.writeUShort (glyphs.size());
		for (j = glyphs.begin(); j != glyphs.end(); j ++)
			j->position->write (pen);

		subTables.push_back (subTable);
	}
	return subTables;
}

/*** Pair positioning ***/

struct PositionPair {
	GlyphId glyph1, glyph2;
	PositionPtr position1, position2;
};

typedef vector <PositionPair> PositionPairs;

bool comparePositionPair (const PositionPair &p1, const PositionPair &p2) {
	if (p1.glyph1 < p2.glyph1)
		return true;
	if (p1.glyph1 > p2.glyph1)
		return false;
	return p1.glyph2 < p2.glyph2;
}

ReferenceMemoryBlocks getPairPositioningMemory (const Positionings &positionings, Preprocessor & prep,
												OpenTypeFont & font)
{
	PositionPairs pairs;
	{
		Positionings::const_iterator i;
		for (i = positionings.begin(); i != positionings.end(); i ++) {
			assert (i->source.size() == 2);
			for (GlyphList::const_iterator firstGlyph = i->source.front()->begin();
			firstGlyph != i->source.front()->end(); firstGlyph ++) {
				for (GlyphList::const_iterator secondGlyph = i->source.back()->begin();
				secondGlyph != i->source.back()->end(); secondGlyph ++) {
					PositionPair pair;
					pair.glyph1 = *firstGlyph;
					pair.glyph2 = *secondGlyph;
					pair.position1 = i->positions.front();
					if (i->positions.size() == 1)
						pair.position2 = new Position;
					else
						pair.position2 = i->positions.back();
					pairs.push_back (pair);
				}
			}
		}
	}

	sort (pairs.begin(), pairs.end(), comparePositionPair);

	GlyphIds duplicates;
	for (PositionPairs::iterator j = pairs.begin() + 1; j != pairs.end();) {
		if ((j-1)->glyph1 == j->glyph1 && (j-1)->glyph2 == j->glyph2) {
			prep.startError() << "Duplicate entry in input range: <" << font.getGlyph (j->glyph1)->getName()
				<< " " << font.getGlyph (j->glyph2)->getName() << ">." << endl;
			j = pairs.erase (j);
		} else
			j ++;
	}

	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);

	// format
	pen.writeUShort (1);
	ReferenceMemoryPen coveragePen = pen;
	pen.writeUShort (0);

	pen.writeUShort (pairs.front().position1->getFormat());
	pen.writeUShort (pairs.front().position2->getFormat());

	ReferenceMemoryPen pairSetNumPen = pen;
	pen.writeUShort (0);

	GlyphIds coveredGlyphs;
	PositionPairs::iterator i = pairs.begin();
	while (i != pairs.end()) {
		// A pair set contains all pairs starting with the same glyph.
		// E.g., A V, A Y and A v all belong in the same set.
		coveredGlyphs.push_back (i->glyph1);

		ReferenceMemoryBlockPtr pairSet (new ReferenceMemoryBlock);
		ReferenceMemoryPen pairSetPen (pairSet);
		ReferenceMemoryPen pairNumPen = pairSetPen;
		pairSetPen.writeUShort (0);
		UShort pairNum = 0;
		PositionPairs::iterator j = i;
		do {
			pairSetPen.writeGlyphId (j->glyph2);
			j->position1->write (pairSetPen);
			j->position2->write (pairSetPen);

			pairNum ++;
			j ++;
		} while (j != pairs.end() && j->glyph1 == i->glyph1);
		pairNumPen.writeUShort (pairNum);
		i = j;

		pen.writeReference (pairSet);
	}

	// Coverage table
	coveragePen.writeReference (getCoverageTable (
		coveredGlyphs.begin(), coveredGlyphs.end(), getGlyphIdsGlyphId));
	// pairSetNum
	pairSetNumPen.writeUShort (coveredGlyphs.size());

	return ReferenceMemoryBlocks (1, memory);
}

/*** Mark to Base attachment ***/

typedef list <MarkAnchors> MarkAnchorGroups;

struct MarkRecord {
	GlyphId glyphId;
	UShort markClass;
	AnchorPtr anchor;
};

typedef vector <MarkRecord> MarkRecords;

bool operator < (const MarkRecord & r1, const MarkRecord & r2) {
	return r1.glyphId < r2.glyphId;
}

GlyphId getMarkRecordGlyphId (MarkRecords::const_iterator i) {
	return i->glyphId;
}

pair <ReferenceMemoryBlockPtr, ReferenceMemoryBlockPtr> getMarkArray (const MarkAnchorGroups &groups,
																	  OpenTypeFont &font) {
	MarkRecords records;
	UShort markClass = 0;
	for (MarkAnchorGroups::const_iterator group = groups.begin(); group != groups.end(); group ++, markClass ++) {
		for (MarkAnchors::const_iterator mark = group->begin(); mark != group->end(); mark ++) {
			MarkRecord r;
			r.glyphId = mark->glyph;
			r.anchor = mark->anchor;
			r.markClass = markClass;
			records.push_back (r);
		}
	}

	sort (records.begin(), records.end());

	ReferenceMemoryBlockPtr markArray (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (markArray);

	pen.writeUShort (records.size());
	for (MarkRecords::iterator r = records.begin(); r != records.end(); r ++) {
		pen.writeUShort (r->markClass);
		pen.writeReference (r->anchor->getTable (font.getGlyph (r->glyphId)));
	}

	return pair <ReferenceMemoryBlockPtr, ReferenceMemoryBlockPtr>
		(getCoverageTable (records.begin(), records.end(), getMarkRecordGlyphId),
		markArray);
}

ReferenceMemoryBlocks getMarkToBaseAttachmentMemory (MarkAnchors &markAnchors, BaseAnchors &baseAnchors,
													 LookupSubTable::LookupType type,
													 Preprocessor &prep, OpenTypeFont &font)
{
	sort (markAnchors.begin(), markAnchors.end());
	sort (baseAnchors.begin(), baseAnchors.end());

	bool error = false;

	MarkAnchors::iterator mark;
	for (mark = markAnchors.begin() + 1; mark != markAnchors.end(); mark ++) {
		if (mark->glyph == (mark-1)->glyph) {
			error = true;
			prep.startError() << "Double attachment point for mark \""
				<< font.getGlyph (mark->glyph)->getName() << "\"." << endl;
		}
	}

	if (error)
		return ReferenceMemoryBlocks();

	BaseAnchors::iterator base = baseAnchors.begin();
	while (base != baseAnchors.end()) {
		mark = markAnchors.begin();
		GlyphId lastBase = base->base;
		UShort lastCompNum = base->anchors.size();
		do {
			if (mark->glyph != base->mark) {
				if (mark->glyph < base->mark) {
					error = true;
					prep.startError() << "Missing attachment point for base \"" <<
						font.getGlyph (base->base)->getName() << "\" to mark \""
						<< font.getGlyph (mark->glyph)->getName() << "\"." << endl;
					mark ++;
				} else {
					error = true;
					if (mark != markAnchors.begin() && base->mark == (base-1)->mark) {
						prep.startError() << "Double attachment point for base \"" <<
							font.getGlyph (base->base)->getName() << "\" to mark \""
							<< font.getGlyph (mark->glyph)->getName() << "\"." << endl;
					} else {
						prep.startError() << "Missing attachment point for mark \"" <<
							font.getGlyph (base->mark)->getName() << "\"." << endl;
					}
					base ++;
				}
			} else {
				if (base->anchors.size() != lastCompNum) {
					error = true;
					prep.startError() << "Number of anchors for base \"" <<
						font.getGlyph (base->base)->getName() << "\" to mark \""
						<< font.getGlyph (mark->glyph)->getName() << "\" invalid: " <<
						base->anchors.size() << ", should be " << lastCompNum << "." << endl;
				}
				mark ++;
				base ++;
			}
		} while (base != baseAnchors.end() && base->base == lastBase);
	}

	if (error)
		return ReferenceMemoryBlocks();

	// The mark to base attachment table works with mark classes.
	// We must now find out which marks have the same anchor for all base glyphs.
	// (This will usually make groups of all marks above, all marks attaching right
	// above, etcetera).

	MarkAnchorGroups markAnchorGroups;
	// Let's put all marks in one group
	markAnchorGroups.push_back (markAnchors);
	MarkAnchorGroups::iterator group;
	GlyphIds baseIds;

	for (base = baseAnchors.begin(); base != baseAnchors.end();) {
		for (group = markAnchorGroups.begin(); group != markAnchorGroups.end(); group ++) {
			MarkAnchorGroups::iterator newGroup = markAnchorGroups.end();
			// Find the first base attachment point for the first mark in this group
			BaseAnchors::iterator groupFirst = base;
			while (groupFirst->mark != group->front().glyph)
				groupFirst ++;
			BaseAnchors::iterator cur = groupFirst + 1;
			for (mark = group->begin() + 1; mark != group->end();) {
				// Find the base attachment point for this mark
				while (cur->mark != mark->glyph)
					cur ++;
				if (!sameAnchors (cur->anchors, groupFirst->anchors)) {
					// This mark should not be in this group
					if (newGroup == markAnchorGroups.end()) {
						// Make a new group
						markAnchorGroups.push_back (MarkAnchors());
						newGroup = markAnchorGroups.end();
						newGroup -- ;
					}
					// Move mark to the new group
					newGroup->push_back (*mark);
					mark = group->erase (mark);
				} else
					mark ++;
			}
		}
		baseIds.push_back (base->base);
		do {
			base ++;
		} while (base != baseAnchors.end() && base->base == (base-1)->base);
	}

/*
	// debug groups
	for (group = markAnchorGroups.begin(); group != markAnchorGroups.end(); group ++) {
		std::cout << "Group consisting of:" << endl;
		for (mark = group->begin(); mark != group->end(); mark ++)
			std::cout << "  " << font.getGlyph (mark->glyph)->getName() << endl;
	}
	std::cout << endl;
*/

	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);

	// format 1
	pen.writeUShort (1);
	pair <ReferenceMemoryBlockPtr, ReferenceMemoryBlockPtr> markTables = 
		getMarkArray (markAnchorGroups, font);
	pen.writeReference (markTables.first);
	pen.writeReference (getCoverageTable (baseIds.begin(), baseIds.end(), getGlyphIdsGlyphId));
	pen.writeUShort (markAnchorGroups.size());
	pen.writeReference (markTables.second);
	
	// Produce the base array

	ReferenceMemoryBlockPtr baseArray (new ReferenceMemoryBlock);
	ReferenceMemoryPen baseArrayPen (baseArray);

	baseArrayPen.writeUShort (baseAnchors.size() / markAnchors.size());

	if (type != LookupSubTable::markToLigatureAttachment) {
		// Write mark-to-base attachment table
		for (base = baseAnchors.begin(); base != baseAnchors.end();) {
			// All base anchors for one base glyph
			for (group = markAnchorGroups.begin(); group != markAnchorGroups.end(); group ++) {
				BaseAnchors::iterator a = base;
				while (a->mark != group->front().glyph)
					a ++;
				baseArrayPen.writeReference (a->anchors.front()->getTable (font.getGlyph (a->base)));
			}

			do {
				base ++;
			} while (base != baseAnchors.end() && base->base == (base-1)->base);
		}
	} else {
		// Write mark-to-ligature attachment table
		for (base = baseAnchors.begin(); base != baseAnchors.end();) {
			// All base anchors for one ligature glyph
			ReferenceMemoryBlockPtr ligatureAttachTable (new ReferenceMemoryBlock);
			ReferenceMemoryPen ligatureAttachPen (ligatureAttachTable);
			ligatureAttachPen.writeUShort (base->anchors.size());
			for (UShort curComponent = 0; curComponent != base->anchors.size(); curComponent ++) {
				// All anchors for one component
				for (group = markAnchorGroups.begin(); group != markAnchorGroups.end(); group ++) {
					BaseAnchors::iterator a = base;
					while (a->mark != group->front().glyph)
						a ++;
					ligatureAttachPen.writeReference (a->anchors [curComponent]->getTable
						(font.getGlyph (a->base)));
				}
			}

			baseArrayPen.writeReference (ligatureAttachTable);
			do {
				base ++;
			} while (base != baseAnchors.end() && base->base == (base-1)->base);
		}
	}

	pen.writeReference (baseArray);

	return ReferenceMemoryBlocks(1, memory); 
}


/*** LookupReference ***/

struct LookupReference {
	PreprocessorPosition pos;
	UShort sequenceIndex;
	String name;
	LookupReference (PreprocessorPosition _pos) : pos (_pos) {}
};
typedef vector <LookupReference> LookupReferences;

struct ReferenceLookup {
	LookupSubTable::LookupPlace place;
	ReferenceMemoryPen numPen;
	ReferenceMemoryPen recordPen;
	LookupReferences references;
};
typedef vector <ReferenceLookup> ReferenceLookups;

ReferenceLookups referenceLookups;

ReferenceMemoryBlocks getContextLookupMemory
(GlyphSequence &input, LookupSubTable::LookupPlace place, ReferenceLookup &lookup) {
	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);
	// write format
	pen.writeUShort (3);

	pen.writeUShort (input.size());
	lookup.numPen = pen;
	pen.writeUShort (0);
	for (GlyphSequence::iterator h = input.begin(); h != input.end(); h ++) {
		sort ((*h)->begin(), (*h)->end());
		pen.writeReference (getCoverageTable ((*h)->begin(), (*h)->end(), getGlyphIdsGlyphId));
	}

	lookup.recordPen = pen;

	return ReferenceMemoryBlocks (1, memory);
}

ReferenceMemoryBlocks getChainingContextLookupMemory
	(GlyphSequence &backtrack, GlyphSequence &input, GlyphSequence & lookAhead,
	LookupSubTable::LookupPlace place, ReferenceLookup &lookup)
{
	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);
	// write format
	pen.writeUShort (3);

	// Backtrack sequence must be written in reverse order
	pen.writeUShort (backtrack.size());
	for (GlyphSequence::reverse_iterator g = backtrack.rbegin(); g != backtrack.rend(); g ++) {
		sort ((*g)->begin(), (*g)->end());
		pen.writeReference (getCoverageTable ((*g)->begin(), (*g)->end(), getGlyphIdsGlyphId));
	}

	// Input sequence
	pen.writeUShort (input.size());
	GlyphSequence::iterator h;
	for (h = input.begin(); h != input.end(); h ++) {
		sort ((*h)->begin(), (*h)->end());
		pen.writeReference (getCoverageTable ((*h)->begin(), (*h)->end(), getGlyphIdsGlyphId));
	}

	// Lookahead sequence
	pen.writeUShort (lookAhead.size());
	for (h = lookAhead.begin(); h != lookAhead.end(); h ++) {
		sort ((*h)->begin(), (*h)->end());
		pen.writeReference (getCoverageTable ((*h)->begin(), (*h)->end(), getGlyphIdsGlyphId));
	}

	lookup.numPen = pen;
	pen.writeUShort (0);
	lookup.recordPen = pen;
	return ReferenceMemoryBlocks (1, memory);
}

/*** getLookup ***/

class UnknownLookupException {
public:
	UnknownLookupException() {}
};

void printSequence (ostream & o, const GlyphSequence & s, OpenTypeFont & font) {
	for (GlyphSequence::const_iterator l = s.begin(); l != s.end(); l ++) {
		if ((*l)->size() == 1)
			o << " " << font.getGlyph ((*l)->front())->getName();
		else
			o << " [" << (*l)->size() << " glyphs]";
	}
}

LookupSubTablePtr getLookup (Preprocessor & prep, OpenTypeFont & font, Groups & groups) {
	LookupSubTablePtr table (new LookupSubTable (prep.getCurrentPosition()));
	table->lookupFlag = 0;
	table->place = LookupSubTable::unknownLookup;
	table->type = LookupSubTable::unknownType;
	
	String buffer = prep.get();
	assert (buffer == "lookup");
	table->name = getIdentifier (prep);
	if (!isExpected (prep, "{"))
		return table;
	buffer = prep.get();
	while (buffer == "ignore") {
		buffer = prep.get();
		if (buffer == "base") {
			if (table->lookupFlag & 0x0002)
				prep.startError() << "\"ignore base\" has already been specified." << endl;
			else
				table->lookupFlag |= 0x0002;
		} else {
			if (buffer == "ligature") {
				if (table->lookupFlag & 0x0004)
					prep.startError() << "\"ignore ligature\" has already been specified." << endl;
				else
					table->lookupFlag |= 0x0004;
			} else {
				if (buffer == "mark") {
					if (table->lookupFlag & 0x0008)
						prep.startError() << "\"ignore mark\" has already been specified." << endl;
					else {
						if (table->lookupFlag & 0xFF00)
							prep.startError() << "\"ignore mark except <group>\" has already been specified." << endl;
						else {
							buffer = prep.peek();
							if (buffer == "except") {
								prep.deleteToken();
								if (isExpected (prep, "@"))
									table->lookupFlag |= (groups.getMarkGroupIndex (prep.get(), prep) << 8);
							} else
								table->lookupFlag |= 0x0008;
						}
					}
				} else {
					prep.startError()
						<< "\"base\", \"ligature\" or \"mark\" expected after \"ignore\" instead of \""
						<< buffer << "\"." << endl;
				}
			}
		}
		
		isExpected (prep, ";");
		
		buffer = prep.get();
	}
	
	PreprocessorPosition pos = prep.getCurrentPosition();
	if (buffer == "sub") {
		table->place = LookupSubTable::GSUBLookup;
		Substitutions substitutions;
		do {
			PreprocessorPosition pos = prep.getCurrentPosition();
			Substitution substitution;
			buffer = prep.peek();
			while (buffer != "->") {
				if (buffer == ";")
					break;
				substitution.source.push_back (new GlyphList (prep, font, groups));
				buffer = prep.peek();
			}
			if (buffer == "->")
				prep.deleteToken();
			
			while (buffer != ";") {
				substitution.dest.push_back (new GlyphList (prep, font, groups));
				buffer = prep.peek();
			}
			prep.deleteToken();
			
			if (substitution.source.empty() || substitution.dest.empty()) {
				prep.startError (pos) << "Glyph names expected before and after \"->\"." << endl;
				break;
			}

			try {
				if (substitution.source.size() == 1) {
					if (substitution.dest.size() == 1) {
						if (substitution.source.front()->size() == substitution.dest.front()->size()) {
							// Single substitution
							checkType (pos, table, LookupSubTable::singleSubstitution);
						} else {
							if (substitution.source.front()->size() == 1 &&
								substitution.dest.front()->size() > 1)
							{	// Alternate substitution
								checkType (pos, table, LookupSubTable::alternateSubstitution);
								GlyphList::iterator i;
								for (i = substitution.dest.front()->begin();
								i != substitution.dest.front()->end(); i ++) {
									if (*i == substitution.source.front()->front()) {
										// Found the source glyph
										substitution.dest.front()->erase (i);
										break;
									}
								}
								if (i == substitution.dest.front()->end()) {
									prep.startError (pos) << "Source glyph not found in alternate list." << endl;
								}
							} else
								throw UnknownLookupException();
						}
					} else {
						if (substitution.source[0]->size() != 1)
							throw UnknownLookupException();
						// Multiple substitution?
						for (GlyphSequence::iterator s = substitution.dest.begin(); s != substitution.dest.end(); s ++) {
							if ((*s)->size() != 1)
								throw UnknownLookupException();
						}
						checkType (pos, table, LookupSubTable::multipleSubstitution);
					}
				} else {
					// Ligature substitution?
					for (GlyphSequence::iterator i = substitution.source.begin();
					i != substitution.source.end(); i++) {
						if ((*i)->size() != 1)
							throw UnknownLookupException();
					}
					if (substitution.dest.size() != 1 || substitution.dest.front()->size() != 1)
						throw UnknownLookupException();
					
					checkType (pos, table, LookupSubTable::ligatureSubstitution);
				}
			
				substitutions.push_back (substitution);
			} catch (UnknownLookupException) {
				ostream &o = prep.startError (pos);
				o << "Unknown substitution format:";
				printSequence (o, substitution.source, font);
				o << " ->";
				printSequence (o, substitution.dest, font);
				o << "." << endl;
			}
			
			buffer = prep.get();
		} while (buffer != "}");
		
		if (!substitutions.empty()) {
			switch (table->type) {
			case LookupSubTable::singleSubstitution:
				table->memory = getSingleSubstitutionMemory (substitutions, prep, font);
				break;
			case LookupSubTable::multipleSubstitution:
				table->memory = getMultipleSubstitutionMemory (substitutions, prep, font);
				break;
			case LookupSubTable::alternateSubstitution:
				table->memory = getAlternateSubstitutionMemory (substitutions, prep, font);
				break;
			case LookupSubTable::ligatureSubstitution:
				table->memory = getLigatureSubstitutionMemory (substitutions, prep, font);
				break;
			default:
				assert (false);
			}
		}
	} else {
		if (buffer == "pos") {
			try {
				Positionings positionings;
				table->place = LookupSubTable::GPOSLookup;
				do {
					PreprocessorPosition pos = prep.getCurrentPosition();
					Positioning positioning;
					while (buffer != "->") {
						if (buffer == ";")
							break;
						positioning.source.push_back (new GlyphList (prep, font, groups));
						buffer = prep.peek();
					}
					if (buffer == "->")
						prep.deleteToken();
					
					while (buffer != ";") {
						positioning.positions.push_back (new Position (prep));
						buffer = prep.peek();
					}
					prep.deleteToken();
					
					if (positioning.source.empty()) {
						prep.startError (pos) << "Glyph names expected before \"->\"." << endl;
						break;
					}
					if (positioning.positions.empty()) {
						prep.startError (pos) << "Positioning expected after \"->\"." << endl;
						break;
					}
					
					if (positioning.source.size() == 1 && positioning.positions.size() == 1) {
						checkType (pos, table, LookupSubTable::singlePositioning);
					} else {
						if (positioning.source.size() == 2 && positioning.positions.size() >= 1) {
							checkType (pos, table, LookupSubTable::pairPositioning);
						} else
							throw pos;
					}
					
					if (!positionings.empty()) {
						Positions::iterator oldPos = positionings.back().positions.begin();
						Positions::iterator newPos = positioning.positions.begin();
						for (; newPos != positioning.positions.end(); oldPos ++, newPos ++) {
							if (oldPos == positionings.back().positions.end() || 
								(*oldPos)->getFormat() != (*newPos)->getFormat())
							{
								prep.startError (pos) <<
									"Positioning format should be the same within one lookup." << endl;
								throw pos;
							}
						}
					}
					
					positionings.push_back (positioning);
					buffer = prep.get();
				} while (buffer != "}");
				if (!positionings.empty()) {
					switch (table->type) {
					case LookupSubTable::singlePositioning:
						table->memory = getSinglePositioningMemory (positionings, prep, font);
						break;
					case LookupSubTable::pairPositioning:
						table->memory = getPairPositioningMemory (positionings, prep, font);
						break;
					default:
						assert (false);
					}
				}
			} catch (PreprocessorPosition & pos) {
				prep.startError (pos) << "Unknown positioning format." << endl;
				do {
					buffer = prep.get();
				} while (buffer != "}" && !prep.eof());
			}
		} else {
			if (buffer == "mark" || buffer == "base") {
				try {
					MarkAnchors markAnchors;
					BaseAnchors baseAnchors;
					do {
						PreprocessorPosition pos = prep.getCurrentPosition();
						if (buffer == "mark") {
							GlyphListPtr marks = new GlyphList (prep, font, groups);
							AnchorPtr anchor = new Anchor (prep);
							for (GlyphList::iterator i = marks->begin(); i != marks->end(); i ++) {
								MarkAnchor a;
								a.glyph = *i;
								a.anchor = anchor;
								markAnchors.push_back (a);
							}
						} else {
							if (buffer == "base") {
								buffer = prep.peek();
								bool markPositioning;
								if (buffer == "mark") {
									markPositioning = true;
									prep.deleteToken();
								} else
									markPositioning = false;
								
								GlyphListPtr baseGlyphs = new GlyphList (prep, font, groups);
								GlyphListPtr marks = new GlyphList (prep, font, groups);
								Anchors anchors;
								do {
									anchors.push_back (new Anchor (prep));
									buffer = prep.peek();
								} while (buffer == "<");

								if (markPositioning)
									checkType (pos, table, LookupSubTable::markToMarkAttachment);
								else {
									if (anchors.size() == 1) {
										checkType (pos, table, LookupSubTable::markToBaseAttachment);
									} else
										checkType (pos, table, LookupSubTable::markToLigatureAttachment);
								}

								for (GlyphList::iterator b = baseGlyphs->begin(); b != baseGlyphs->end(); b ++) {
									for (GlyphList::iterator m = marks->begin(); m != marks->end(); m ++) {
										BaseAnchor a;
										a.base = *b;
										a.mark = *m;
										a.anchors = anchors;
										baseAnchors.push_back (a);
									}
								}
							} else {
								prep.startError (pos) << "\"base\" or \"mark\" expected instead of \""
									<< buffer << "\"." << endl;
								break;
							}
						}
						
						isExpected (prep, ";");
						
						buffer = prep.get();
					} while (buffer != "}");

					if (markAnchors.empty()) {
						prep.startError (pos) << "No mark anchors in mark attachment lookup." << endl;
						throw pos;
					}
					if (baseAnchors.empty()) {
						prep.startError (pos) << "No base glyph anchors in mark attachment lookup." << endl;
						throw pos;
					}

					switch (table->type) {
						// These two tables are exactly the same:
					case LookupSubTable::markToBaseAttachment:
					case LookupSubTable::markToMarkAttachment:
						// And this one is very similar
					case LookupSubTable::markToLigatureAttachment:
						table->memory = getMarkToBaseAttachmentMemory (markAnchors, baseAnchors,
							table->type, prep, font);
						break;
					default:
						assert (false);
					}
				} catch (PreprocessorPosition & pos) {
					prep.startError (pos) << "Unknown attachment format." << endl;
					do {
						buffer = prep.get();
					} while (buffer != "}" && !prep.eof());
				}
			} else {
				if (buffer == "context") {
					PreprocessorPosition pos = prep.getCurrentPosition();
					try {
						GlyphSequence backtrack, input, lookAhead;
						buffer = prep.peek();
						if (buffer == "(") {
							prep.deleteToken();
							do {
								backtrack.push_back (new GlyphList (prep, font, groups));
								buffer = prep.peek();
							} while (buffer != ")");
							prep.deleteToken();
						}
						do {
							input.push_back (new GlyphList (prep, font, groups));
							buffer = prep.peek();
						} while (buffer != ";" && buffer != "(");
						if (buffer == "(") {
							prep.deleteToken();
							do {
								lookAhead.push_back (new GlyphList (prep, font, groups));
								buffer = prep.peek();
							} while (buffer != ")");
							prep.deleteToken();
						}
						isExpected (prep, ";");

						ReferenceLookup refs;

						PreprocessorPosition pos = prep.getCurrentPosition();
						buffer = prep.get();
						do {
							LookupSubTable::LookupPlace place;
							if (buffer == "sub")
								place = LookupSubTable::GSUBLookup;
							else {
								if (buffer == "pos")
									place = LookupSubTable::GPOSLookup;
								else {
									prep.startError (pos) << "\"sub\" or \"pos\" expected instead of \"" <<
										buffer << "\"." << endl;
									throw UnknownLookupException();
								}
							}
							if (table->place == LookupSubTable::unknownLookup)
								table->place = place;
							else {
								if (table->place != place) {
									prep.startError (pos) <<
										"Substitution lookups and positioning lookups may not be mixed in a context lookup." << endl;
									throw UnknownLookupException();
								}
							}
							LookupReference ref (pos);
							ref.sequenceIndex = getInt (prep);
							ref.name = getIdentifier (prep);
							refs.references.push_back (ref);
							isExpected (prep, ";");
							buffer = prep.get();
						} while (buffer != "}");

						if (!input.empty()) {
							if (backtrack.empty() && lookAhead.empty()) {
								// Write simple context substitution table
								if (table->place == LookupSubTable::GSUBLookup)
									table->type = LookupSubTable::contextSubstitution;
								else
									table->type = LookupSubTable::contextPositioning;

								table->memory = getContextLookupMemory (input, table->place, refs);
							} else {
								// Write chaining context substitution table
								if (table->place == LookupSubTable::GSUBLookup)
									table->type = LookupSubTable::chainingContextSubstitution;
								else
									table->type = LookupSubTable::chainingContextPositioning;

								table->memory = getChainingContextLookupMemory (backtrack, input, lookAhead,
									table->place, refs);
							}

							refs.place = table->place;
							referenceLookups.push_back (refs);
						}
					} catch (UnknownLookupException) {
						prep.startError (pos) << "Unknown context lookup." << endl;
						do {
							buffer = prep.get();
						} while (buffer != "}" && !prep.eof());
					}
				} else {
					prep.startError (table->pos) << "\"sub\" or \"pos\" expected instead of \""
						<< buffer << "\"." << endl;
				}
			}
		}
	}
				
	return table;
}


void writeReferenceLookups (const LookupSubTables & lookups, Bools & lookupsUsed,
							LookupSubTable::LookupPlace place)
{
	for (ReferenceLookups::iterator l = referenceLookups.begin(); l != referenceLookups.end(); l ++) {
		if (l->place == place) {
		UShort referenceNum = 0;
			for (LookupReferences::iterator r = l->references.begin(); r != l->references.end(); r ++) {
				bool found = false;
				LookupSubTables::const_iterator t = lookups.begin();
				Bools::iterator b = lookupsUsed.begin();
				for (; t != lookups.end(); t ++, b ++) {
					if ((*t)->name == r->name && (*t)->place == place) {
						found = true;
						*b = true;
						referenceNum ++;
						l->recordPen.writeUShort (r->sequenceIndex);
						l->recordPen.writeUShort (t - lookups.begin());
					}
				}
				if (!found) {
					r->pos.prep.startError (r->pos) << "Lookup \"" << r->name <<
						"\" not found." << endl;
				}
			}

			l->numPen.writeUShort (referenceNum);
		}
	}
}
