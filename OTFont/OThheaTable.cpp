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
#include "OThheaTable.h"
#include "OTGlyph.h"
#include "OTTags.h"

using util::String;

namespace OpenType {

hheaTable::hheaTable (OpenTypeFile &aFont, MemoryBlockPtr memory) : Table (aFont) {
	MemoryPen pen (memory);
	Fixed version = pen.readFixed();
	if (version != 0x00010000)
		throw Exception ("Unknown table version number " +
			String (version, 16));
	ascender = pen.readFWord();
	descender = pen.readFWord();
	lineGap = pen.readFWord();
	advanceWidthMax = pen.readUFWord();
	minLeftSideBearing = pen.readFWord();
	minRightSideBearing = pen.readFWord();
	xMaxExtent = pen.readFWord();
	caretSlopeRise = pen.readShort();
	caretSlopeRun = pen.readShort();
	caretOffset = pen.readShort();
	
	// Reserved
	pen.readShort();
	pen.readShort();
	pen.readShort();
	pen.readShort();

	metricDataFormat = pen.readShort();
	if (metricDataFormat != 0)
		throw Exception ("Unknown metric data format: " +
			String (metricDataFormat));
	hMetricsNum = pen.readUShort();
}

hheaTable::~hheaTable() {}

Tag hheaTable::getTag() const {
	return hheaTag;
}

MemoryBlockPtr hheaTable::getMemory() const {
	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);

	// version
	pen.writeFixed (0x00010000);

	pen.writeFWord (ascender);
	pen.writeFWord (descender);
	pen.writeFWord (lineGap);
	pen.writeUFWord (advanceWidthMax);
	pen.writeFWord (minLeftSideBearing);
	pen.writeFWord (minRightSideBearing);
	pen.writeFWord (xMaxExtent);
	pen.writeShort (caretSlopeRise);
	pen.writeShort (caretSlopeRun);
	pen.writeShort (caretOffset);

	// Reserved
	pen.writeShort (0);
	pen.writeShort (0);
	pen.writeShort (0);
	pen.writeShort (0);

	pen.writeShort (metricDataFormat);
	pen.writeUShort (hMetricsNum);

	return memory;
}

UShort hheaTable::getHMetricsNum() {
	return hMetricsNum;
}

void hheaTable::setHMetricsNum (UShort aHMetricsNum) {
	hMetricsNum = aHMetricsNum;
}

void hheaTable::reset (const Glyphs &glyphs) {
	advanceWidthMax = 0;
	minLeftSideBearing = 0x7FFF;
	minRightSideBearing = 0x7FFF;
	xMaxExtent = -0x8000;

	bool allEmpty = true;
	Glyphs::const_iterator g;
	for (g = glyphs.begin(); g != glyphs.end(); g ++) {
		if (!(*g)->isEmpty())
			allEmpty = false;
		(*g)->getHorizontalMaxima (this);
	}
	if (allEmpty)
		minLeftSideBearing =
			minRightSideBearing = xMaxExtent = 0;
}

void hheaTable::setHorizontalMaxima (UFWord aAdvanceWidthMax, FWord aMinLeftSideBearing,
									   FWord aMinRightSideBearing, FWord aXMaxExtent)
{
	if (advanceWidthMax < aAdvanceWidthMax)
		advanceWidthMax = aAdvanceWidthMax;
	if (aMinLeftSideBearing < minLeftSideBearing)
		minLeftSideBearing = aMinLeftSideBearing;
	if (aMinRightSideBearing < minRightSideBearing)
		minRightSideBearing = aMinRightSideBearing;
	if (xMaxExtent < aXMaxExtent)
		xMaxExtent = aXMaxExtent;
}

void hheaTable::setAdvanceWidthMax (UFWord aAdvanceWidthMax) {
	if (advanceWidthMax < aAdvanceWidthMax)
		advanceWidthMax = aAdvanceWidthMax;
}

} // end namespace OpenType
