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

#include "OTException.h"
#include "OTGlyph.h"
#include "OpenTypeFont.h"
#include "OTmaxpTable.h"
#include "OTheadTable.h"
#include "OThheaTable.h"
#include "OTTags.h"
#include "OTOutline.h"
#include "OTGlyphToOutline.h"

using std::vector;
using util::smart_ptr;
using util::String;
using util::return_type_specifier;
using util::fixed_fraction;

namespace OpenType {

/*** Contours ***/

size_t Contours::getPointNum() const {
	size_t sum = 0;
	const_iterator contour;
	for (contour = begin(); contour != end(); contour ++) {
		sum += contour->size();
	}
	return sum;
}

ContourPoint & Contours::getPoint (size_t index) {
	iterator contour = begin();
	do {
		if (index >= contour->size()) {
			index -= contour->size();
			contour ++;
			if (contour == end())
				throw Exception ("Invalid contour point index");
		} else {
			return (*contour) [index];
		}
	} while (true);
}

const ContourPoint & Contours::getPoint (size_t index) const {
	const_iterator contour = begin();
	do {
		if (index >= contour->size()) {
			index -= contour->size();
			contour ++;
			if (contour == end())
				throw Exception ("Invalid contour point index");
		} else {
			return (*contour) [index];
		}
	} while (true);
}

/*** CompositeComponent ***/

CompositeComponent::CompositeComponent (OpenTypeFont &aFont, UShort aGlyphIndex,
										Flags aFlags, Scale aScale)
: font (aFont), glyphIndex (aGlyphIndex), flags (Flags (aFlags & cfSave)),
scale (aScale) {}

CompositeComponent::~CompositeComponent() {}

Contours CompositeComponent::getScaledContours() const {
	smart_ptr <Glyph> glyph = font.getGlyph (glyphIndex);
	Contours contours = glyph->getContours();
	Contours::iterator c;
	for (c = contours.begin(); c != contours.end(); c ++) {
		Contour::iterator p = c->begin();
		do {
			Short x = (p->x * scale.xx + p->y * scale.yx).round(); //OT_MULTIPLY_BY_F2DOT14_AND_ADD (p->x, scale.xx, p->y, scale.yx);
			Short y = (p->x * scale.xy + p->y * scale.yy).round(); //OT_MULTIPLY_BY_F2DOT14_AND_ADD (p->x, scale.xy, p->y, scale.yy);
			p->x = x;
			p->y = y;

			p ++;
		} while (p != c->begin());
	}
	return contours;
}

UShort CompositeComponent::getPointNum() const {
	return font.getGlyph (glyphIndex)->getPointNum();
}

UShort CompositeComponent::getContourNum() const {
	return font.getGlyph (glyphIndex)->getContourNum();
}

UShort CompositeComponent::getDepth() const {
	return font.getGlyph (glyphIndex)->getCompositeDepth() + 1;
}


/*** PositionedCompositeComponent ***/

PositionedCompositeComponent::PositionedCompositeComponent (OpenTypeFont &aFont, UShort aGlyphIndex,
											Flags aFlags, Scale aScale, Translation aTranslation)
: CompositeComponent (aFont, aGlyphIndex, aFlags, aScale),
translation (aTranslation) {}

PositionedCompositeComponent::~PositionedCompositeComponent() {}

std::pair <CompositeComponent::Translation, Contours>
	PositionedCompositeComponent::getContours (const Contours &previous) const
{
	Contours contours = getScaledContours();
	Contours::iterator c;
	for (c = contours.begin(); c != contours.end(); c ++) {
		Contour::iterator p = c->begin();
		do {
			p->x += translation.x;
			p->y += translation.y;

			p ++;
		} while (p != c->begin());
	}
	return std::pair <Translation, Contours> (translation, contours);
}

void PositionedCompositeComponent::write (MemoryWritePen &pen, UShort extraFlags) const {
	// Flags need to be written when everything is known, so let's save the current flag position
	MemoryWritePen flagsPen = pen;
	// Skip flags for now
	pen += 2;
	extraFlags |= cfArgsAreXYValues;

	pen.writeUShort (glyphIndex);

	if (translation.x < -0x80 || translation.x >= 0x80 ||
		translation.y < -0x80 || translation.y >= 0x80)
	{
		extraFlags |= cfArgsAreWords;
		pen.writeShort (translation.x);
		pen.writeShort (translation.y);
	} else {
		pen.writeSignedByte ((SignedByte) translation.x);
		pen.writeSignedByte ((SignedByte) translation.y);
	}
	if (scale.xy == 0 && scale.yx == 0) {
		if (scale.xx == scale.yy) {
			if (scale.xx != 1) {
				extraFlags |= cfSimpleScale;
				pen.writeF2Dot14 (scale.xx);
			}
			// else no scaling so do nothing
		} else {
			extraFlags |= cfXYScale;
			pen.writeF2Dot14 (scale.xx);
			pen.writeF2Dot14 (scale.yy);
		}
	} else {
		extraFlags |= cfTwoByTwo;
		pen.writeF2Dot14 (scale.xx);
		pen.writeF2Dot14 (scale.xy);
		pen.writeF2Dot14 (scale.yx);
		pen.writeF2Dot14 (scale.yy);
	}

	// Write extraFlags
	flagsPen.writeUShort (flags | extraFlags);
}

/*** AttachedCompositeComponent ***/

AttachedCompositeComponent::AttachedCompositeComponent (OpenTypeFont &aFont, UShort aGlyphIndex,
											Flags aFlags, Scale aScale, UShort aP1, UShort aP2)
: CompositeComponent (aFont, aGlyphIndex, aFlags, aScale),
p1 (aP1), p2 (aP2) {}

AttachedCompositeComponent::~AttachedCompositeComponent() {}

std::pair <CompositeComponent::Translation, Contours>
	AttachedCompositeComponent::getContours (const Contours &previousContours) const
{
	Exception::FontContext c1 (font);
	Exception::Context c2 ("calculating points positions");

	Contours contours = getScaledContours();
	if (p1 >= previousContours.getPointNum())
		throw Exception ("Matched glyph attachment point " + String (p1) +
			" too large in composite glyph");

	if (p2 >= contours.getPointNum())
		throw Exception ("New glyph attachment point " + String (p2) +
			" too large in composite glyph");

	const ContourPoint & attachTo = previousContours.getPoint (p1);
	const ContourPoint & attach = contours.getPoint (p2);
	Translation translation;
	translation.x = attachTo.x - attach.x;
	translation.y = attachTo.y - attach.y;

	Contours::iterator c;
	for (c = contours.begin(); c != contours.end(); c ++) {
		Contour::iterator p = c->begin();
		do {
			p->x += translation.x;
			p->y += translation.y;

			p ++;
		} while (p != c->begin());
	}
	return std::pair <Translation, Contours> (translation, contours);
}

CompositeComponent::Translation AttachedCompositeComponent::getTranslation() const {
	Translation t = {0, 0};
	return t;
}


void AttachedCompositeComponent::write (MemoryWritePen &pen, UShort extraFlags) const {
	// extraFlags need to be written when everything is known, so let's save the current flag position
	MemoryWritePen flagsPen = pen;
	// Skip extraFlags for now
	pen += 2;

	pen.writeUShort (glyphIndex);

	if (p1 >= 0x100 || p2 >= 0x100) {
		extraFlags |= cfArgsAreWords;
		pen.writeUShort (p1);
		pen.writeUShort (p2);
	} else {
		pen.writeByte ((Byte) p1);
		pen.writeByte ((Byte) p2);
	}
	if (scale.xy == 0 && scale.yx == 0) {
		if (scale.xx == scale.yy) {
			if (scale.xx != 1) {
				extraFlags |= cfSimpleScale;
				pen.writeF2Dot14 (scale.xx);
			}
			// else no scaling so do nothing
		} else {
			extraFlags |= cfXYScale;
			pen.writeF2Dot14 (scale.xx);
			pen.writeF2Dot14 (scale.yy);
		}
	} else {
		extraFlags |= cfTwoByTwo;
		pen.writeF2Dot14 (scale.xx);
		pen.writeF2Dot14 (scale.xy);
		pen.writeF2Dot14 (scale.yx);
		pen.writeF2Dot14 (scale.yy);
	}

	// Write extraFlags
	flagsPen.writeUShort (flags | extraFlags);
}


/*** Glyph ***/

Glyph::Glyph (OpenTypeFont &aFont, String aName, HorMetric aHorMetric)
: font (aFont), name (aName), horMetric (aHorMetric), dirtyBoundingBox (false) {
	boundingBox.xMin = 0;
	boundingBox.xMax = 0;
	boundingBox.yMin = 0;
	boundingBox.yMax = 0;
}

Glyph::~Glyph() {}

Glyph::Glyph (OpenTypeFont &aFont, String aName, HorMetric aHorMetric, MemoryPen &pen)
: font (aFont), name (aName), horMetric (aHorMetric), dirtyBoundingBox (true) {
	// Number of contours has been read already
	boundingBox.xMin = pen.readShort();
	boundingBox.yMin = pen.readShort();
	boundingBox.xMax = pen.readShort();
	boundingBox.yMax = pen.readShort();
	// The rest is read either by SimpleGlyph() or by CompositeGlyph()
}

void Glyph::write (MemoryWritePen &pen) const {
	// Writing empty glyph == writing nothing
}

bool Glyph::isEmpty() const {
	return true;
}

bool Glyph::isComposite() const {
	return false;
}

String Glyph::getName() const {
	return name;
}

HorMetric Glyph::getHorMetric() const {
	return horMetric;
}

UShort Glyph::getContourNum() const {
	return 0;
}

const Components & Glyph::getComponents() const {
	assert (false);
	return *((Components*)NULL);
}

UShort Glyph::getPointNum() const {
	return 0;
}

UShort Glyph::getCompositeDepth() const {
	return 1;
}

Contours Glyph::getContours() const {
	return Contours();
}

MemoryBlockPtr Glyph::getInstructions() const {
	return instructions;
}

void Glyph::setInstructions (MemoryBlockPtr aInstructions) {
	instructions = aInstructions;
}

void Glyph::getMaximumProfile (maxpTable *maxp) const {
	maxp->setMaximumProfile (0, 0, 0, 0,
		instructions ? instructions->getSize() : 0, 0, 1);
}

void Glyph::getBoundingBox (headTable *head) const {
	recalculateBoundingBoxIfNecessary();
	if (!isEmpty())
		head->setBoundingBox (boundingBox);
}

void Glyph::getHorizontalMaxima (hheaTable *hhea) const {
	recalculateBoundingBoxIfNecessary();
	if (!isEmpty())
		hhea->setHorizontalMaxima (horMetric.advanceWidth, horMetric.lsb,
			horMetric.advanceWidth - horMetric.lsb -
			(boundingBox.xMax - boundingBox.xMin), horMetric.lsb + (boundingBox.xMax - boundingBox.xMin));
	else
		hhea->setAdvanceWidthMax (horMetric.advanceWidth);
}

void Glyph::recalculateBoundingBox() const {
	Contours contours = getContours();
	if (contours.empty())
		boundingBox.xMin = boundingBox.yMin = boundingBox.xMax = boundingBox.yMax = 0;
	else {
		boundingBox.xMin = boundingBox.yMin = 0x7FFF;
		boundingBox.xMax = boundingBox.yMax = -0x8000;
		Contours::iterator c = contours.begin();
		for (; c != contours.end(); c ++) {
			ContourOutlineIterator begin (c->begin());
			ContourOutlineIterator o = begin;
			do {
				Short cXMin, cYMin, cXMax, cYMax;
				o.get (return_type_specifier <ContourPoint>())->getBoundingBox (cXMin, cYMin, cXMax, cYMax);
				if (cXMin < boundingBox.xMin)
					boundingBox.xMin = cXMin;
				if (cYMin < boundingBox.yMin)
					boundingBox.yMin = cYMin;
				if (cXMax > boundingBox.xMax)
					boundingBox.xMax = cXMax;
				if (cYMax > boundingBox.yMax)
					boundingBox.yMax = cYMax;

				++ o;
			} while (o != begin);
		}
	}
	dirtyBoundingBox = false;
}

Short Glyph::getDisplacement() const {
	recalculateBoundingBoxIfNecessary();
	return boundingBox.xMin - horMetric.lsb;
}


const Glyph::BoundingBox & Glyph::getBoundingBox() {
	recalculateBoundingBoxIfNecessary();
	return boundingBox;
}

void Glyph::setHorMetric (HorMetric aHm) {
	horMetric = aHm;
}


/*** SimpleGlyph ***/

// Point flags
#define pfOnCurve	0x01
#define pfXShort	0x02
#define pfYShort	0x04
#define pfRepeat	0x08
#define pfXSame		0x10
#define pfYSame		0x20


SimpleGlyph::SimpleGlyph (OpenTypeFont &aFont, String aName, HorMetric aHorMetric,
							  UShort contourNum, MemoryPen &pen)
: Glyph (aFont, aName, aHorMetric, pen)
{
	assert (contourNum > 0);
	// Read last contour points
	typedef vector <UShort> ContourEnds;
	ContourEnds contourEnds;
	contourEnds.reserve (contourNum);
	UShort i;
	UShort lastContourEnd;
	for (i = 0; i < contourNum; i++) {
		UShort newContourEnd = pen.readUShort();
		if (i && newContourEnd <= lastContourEnd)
			throw Exception ("Error in contour endpoints: " + String (newContourEnd) +
				" follows " + String (lastContourEnd));

		lastContourEnd = newContourEnd;
		contourEnds.push_back (newContourEnd);
	}

	UShort pointNum = contourEnds [contourNum - 1] + 1;

	// Read instructions
	UShort instructionLength = pen.readUShort();
	instructions = pen.readBlock (instructionLength);

	// Read flags
	typedef vector <Byte> Flags;
	Flags flags;
	flags.reserve (i);
	for (i = 0; i < pointNum; i++) {
		Byte flag = pen.readByte();
		flags.push_back (flag);

		// Repeat flag a number of times
		if (flag & pfRepeat) {
			UShort repeatEnd = i + pen.readByte();
			for (; i < repeatEnd; i++)
				flags.push_back (flag);
		}
	}
	if (flags.size() != pointNum)
		throw Exception ("Number of point flags is invalid");

	// Read point coordinates
	Short curX = 0;
	Short curY = 0;
	Flags::iterator flag = flags.begin();
	ContourEnds::iterator contourEnd = contourEnds.begin();
	contours = Contours (contourNum);
	Contours::iterator contour = contours.begin();
	UShort pointIndex = 0;
	
	do {
		ContourPoint p;
		p.onCurve = (*flag & pfOnCurve) != 0;

		if (*flag & pfXShort) {
			// X same flag describes sign
			if (*flag & pfXSame)
				curX += pen.readByte();
			else
				curX -= pen.readByte();
		} else {
			// X same flag describes whether x is the same as the previous
			if (!(*flag & pfXSame))
				curX += pen.readShort();
		}

		p.x = curX;
		// p.y is not known yet
		contour->push (p);

		if (pointIndex == *contourEnd) {
			contour ++;
			contourEnd ++;
		}
		pointIndex ++;
		flag ++;
	} while (pointIndex < pointNum);

	assert (contourEnd == contourEnds.end());

	contour = contours.begin();
	Contour::iterator p = contour->begin();
	for (flag = flags.begin(); flag != flags.end(); flag++) {
		if (*flag & pfYShort) {
			// Y same flag describes sign
			if (*flag & pfYSame)
				curY += pen.readByte();
			else
				curY -= pen.readByte();
		} else {
			// Y same flag describes whether y is the same as the previous
			if (!(*flag & pfYSame))
				curY += pen.readShort();
		}

		p->y = curY;

		p ++;
		if (p == contour->begin()) {
			contour ++;
			if (contour != contours.end())
				p = contour->begin();
		}
	}
	assert (contour == contours.end());

	dirtyBoundingBox = true;
}

SimpleGlyph::SimpleGlyph (OpenTypeFont &aFont, String aName, HorMetric aHorMetric,
						  const Contours & aContours)
						  : Glyph (aFont, aName, aHorMetric), contours (aContours)
{
	assert (!contours.empty());
	dirtyBoundingBox = true;
}

SimpleGlyph::SimpleGlyph (const Glyph & glyph)
: Glyph (glyph.font, glyph.name, glyph.horMetric) {
	contours = glyph.getContours();
	dirtyBoundingBox = true;
}

SimpleGlyph::~SimpleGlyph() {}

void SimpleGlyph::write (MemoryWritePen &pen) const {
	pen.writeUShort (getContourNum());
	pen.writeShort (boundingBox.xMin);
	pen.writeShort (boundingBox.yMin);
	pen.writeShort (boundingBox.xMax);
	pen.writeShort (boundingBox.yMax);

	Contours::const_iterator contour;
	Contour::const_iterator point;
	// Write last contour points
	UShort totalSize = UShort (-1);
	for (contour = contours.begin(); contour != contours.end(); contour ++) {
		totalSize += contour->size();
		pen.writeUShort (totalSize);
	}

	// Write instructions
	if (instructions) {
		pen.writeUShort (instructions->getSize ());
		pen.writeBlock (instructions);
	} else
		pen.writeUShort (0);

	// Gather flags
	typedef vector <Byte> Flags;
	Flags flags;
	Short curX = 0;
	Short curY = 0;
	//for (point = points.begin(); point != points.end(); point ++) {
	for (contour = contours.begin(); contour != contours.end(); contour ++) {
		point = contour->begin();
		do {
			Byte flag = point->onCurve ? pfOnCurve : 0;

			if (point->x == curX)
				flag |= pfXSame;
			else {
				if (abs (point->x - curX) < 0x100) {
					// Use byte
					flag |= pfXShort;
					if (point->x > curX)
						// Positive x
						flag |= pfXSame;
				}
			}

			if (point->y == curY)
				flag |= pfYSame;
			else {
				if (abs (point->y - curY) < 0x100) {
					// Use byte
					flag |= pfYShort;
					if (point->y > curY)
						// Positive y
						flag |= pfYSame;
				}
			}

			curX = point->x;
			curY = point->y;

			flags.push_back (flag);

			++ point;
		} while (point != contour->begin());
	}

	// Write flags
	Flags::iterator flag;
	for (flag = flags.begin(); flag != flags.end(); flag ++) {
		if (flag != flags.end() - 1) {
			if (*flag == *(flag + 1)) {
				pen.writeByte (*flag | pfRepeat);
				Byte repeat = 0xFF; //-1
				do {
					repeat ++;
					flag ++;
				} while (repeat < 0xFF &&  flag != flags.end() && *flag == *(flag - 1));
				flag --;
				pen.writeByte (repeat);
			} else
				pen.writeByte (*flag);
		} else
			pen.writeByte (*flag);
	}

	// Write Xs
	curX = 0;
	flag = flags.begin();
	for (contour = contours.begin(); contour != contours.end(); contour ++) {
		point = contour->begin();
		do {
			if (*flag & pfXShort)
				// X same flag describes sign, so no need to write it here
				pen.writeByte (abs(point->x - curX));
			else {
				// X same flag describes whether x is the same as the previous
				if (!(*flag & pfXSame))
					pen.writeShort(point->x - curX);
			}
			curX = point->x;

			point ++;
			flag ++;
		} while (point != contour->begin());
	}

	// Write Ys
	curY = 0;
	flag = flags.begin();
	for (contour = contours.begin(); contour != contours.end(); contour ++) {
		point = contour->begin();
		do {
			if (*flag & pfYShort)
				// Y same flag describes sign, so no need to write it here
				pen.writeByte (abs(point->y - curY));
			else {
				// Y same flag describes whether y is the same as the previous
				if (!(*flag & pfYSame))
					pen.writeShort(point->y - curY);
			}
			curY = point->y;
			point ++;
			flag ++;
		} while (point != contour->begin());
	}

	// That's all.
}

bool SimpleGlyph::isEmpty() const {
	return false;
}

UShort SimpleGlyph::getPointNum() const {
	return contours.getPointNum();
}

UShort SimpleGlyph::getContourNum() const {
	return contours.size();
}

void SimpleGlyph::setContours (const Contours & aContours) {
	contours = aContours;
	dirtyBoundingBox = true;
}

Contours SimpleGlyph::getContours() const {
	return contours;
}

void SimpleGlyph::getMaximumProfile (maxpTable *maxp) const {
	maxp->setMaximumProfile (getPointNum(), getContourNum(), 0, 0,
		instructions ? instructions->getSize() : 0, 0, 0);
}

/*** CompositeGlyph ***/

CompositeGlyph::CompositeGlyph (OpenTypeFont &aFont, String aName, HorMetric aHorMetric,
								const Components &aComponents)
								: Glyph (aFont, aName, aHorMetric), components (aComponents)
{
	assert (!components.empty());
	dirtyBoundingBox = true;
}

// These flags should be saved because they are not used or changed
#define cfSave	(~(cfArgsAreWords | cfArgsAreXYValues | \
	cfSimpleScale | cfMoreComponents | cfXYScale | cfTwoByTwo | cfInstructions))

CompositeGlyph::CompositeGlyph (OpenTypeFont &aFont, String aName, HorMetric aHorMetric,
									MemoryPen &pen)
: Glyph (aFont, aName, aHorMetric, pen) {
	CompositeComponent::Flags flags;

	// Adapted from the MS OpenType specs
	do {
		flags = (CompositeComponent::Flags) pen.readUShort();
		UShort glyphIndex = pen.readUShort();

		// Read composite position
		UShort arg1, arg2;
		if (flags & CompositeComponent::cfArgsAreWords) {
			arg1 = pen.readUShort();;
			arg2 = pen.readUShort();
		} else {
			if (flags & CompositeComponent::cfArgsAreXYValues) {
				// Signed values
				*((Short *) &arg1) = pen.readSignedByte();
				*((Short *) &arg2) = pen.readSignedByte();
			} else {
				arg1 = pen.readByte();
				arg2 = pen.readByte();
			}
		}

		// Read scaling
		CompositeComponent::Scale scale;
		if (flags & CompositeComponent::cfSimpleScale) {
			scale.xx = scale.yy = pen.readF2Dot14();
			scale.xy = scale.yx = 0;
		} else {
			if (flags & CompositeComponent::cfXYScale) {
				scale.xx = pen.readF2Dot14();
				scale.yy = pen.readF2Dot14();
				scale.xy = scale.yx = 0;
			} else {
				if (flags & CompositeComponent::cfTwoByTwo) {
					scale.xx = pen.readF2Dot14();
					scale.xy = pen.readF2Dot14();
					scale.yx = pen.readF2Dot14();
					scale.yy = pen.readF2Dot14();
				} else {
					scale.xx = scale.yy = 1;
					scale.xy = scale.yx = 0;
				}
			}
		}

		if (flags & CompositeComponent::cfArgsAreXYValues) {
			CompositeComponent::Translation t = {arg1, arg2};
			components.push_back (new PositionedCompositeComponent (
				font, glyphIndex, flags, scale, t));
		} else {
			components.push_back (new AttachedCompositeComponent (
				font, glyphIndex, flags, scale,
					arg1, arg2));
		}
	} while (flags & CompositeComponent::cfMoreComponents);
	if (flags & CompositeComponent::cfInstructions) {
		UShort instructionNum = pen.readUShort();
		instructions = pen.readBlock (instructionNum);
	}
	dirtyBoundingBox = true;
}

CompositeGlyph::~CompositeGlyph() {}

void CompositeGlyph::write (MemoryWritePen &pen) const {
	// -1 for contourNum for a composite glyph
	pen.writeShort (-1);
	pen.writeShort (boundingBox.xMin);
	pen.writeShort (boundingBox.yMin);
	pen.writeShort (boundingBox.xMax);
	pen.writeShort (boundingBox.yMax);

	Components::const_iterator c;
	for (c = components.begin(); c != components.end(); c ++) {
		CompositeComponent::Flags extraFlags;
		if (c != components.end() - 1)
			extraFlags = CompositeComponent::cfMoreComponents;
		else {
			if (instructions)
				extraFlags = CompositeComponent::cfInstructions;
			else
				extraFlags = CompositeComponent::cfNone;
		}
		(*c)->write (pen, extraFlags);
	}
	if (instructions) {
		pen.writeUShort (instructions->getSize());
		pen.writeBlock (instructions);
	}
}

bool CompositeGlyph::isEmpty() const {
	return false;
}


bool CompositeGlyph::isComposite() const {
	return true;
}

UShort CompositeGlyph::getPointNum() const {
	Components::const_iterator c;
	UShort pointNum = 0;
	for (c = components.begin(); c != components.end(); c ++)
		pointNum += (*c)->getPointNum();
	return pointNum;
}

const Components & CompositeGlyph::getComponents() const {
	return components;
}

UShort CompositeGlyph::getContourNum() const {
	Components::const_iterator c;
	UShort contourNum = 0;
	for (c = components.begin(); c != components.end(); c ++)
		contourNum += (*c)->getContourNum();
	return contourNum;
}

UShort CompositeGlyph::getCompositeDepth() const {
	Components::const_iterator c;
	UShort depth = 0;
	for (c = components.begin(); c != components.end(); c ++) {
		UShort thisDepth = (*c)->getDepth();
		if (thisDepth > depth)
			depth = thisDepth;
	}
	return depth;
}

Contours CompositeGlyph::getContours() const {
	Contours points;
	Components::const_iterator i;
	for (i = components.begin(); i != components.end(); i++) {
		Contours componentPoints = (*i)->getContours (points).second;
		points.insert (points.end(), componentPoints.begin(), componentPoints.end());
	}
	return points;
}

void CompositeGlyph::addComponent (ComponentPtr newComponent) {
	components.push_back (newComponent);
}

void CompositeGlyph::setComponents (const Components & _components) {
	assert (!_components.empty());
	components = _components;
	dirtyBoundingBox = true;
}

CompositeGlyph::Translations CompositeGlyph::getTranslations() const {
	Translations translations;
	Contours points;
	Components::const_iterator i;
	for (i = components.begin(); i != components.end(); i ++) {
		std::pair <CompositeComponent::Translation, Contours> componentPoints = (*i)->getContours (points);
		points.insert (points.end(), componentPoints.second.begin(), componentPoints.second.end());
		translations.push_back (componentPoints.first);
	}
	return translations;
}

void CompositeGlyph::getMaximumProfile (maxpTable *maxp) const {
	recalculateBoundingBoxIfNecessary();
	maxp->setMaximumProfile (0, 0, getPointNum(), getContourNum(),
		instructions ? instructions->getSize() : 0, components.size(), getCompositeDepth());
}

} // end namespace OpenType
