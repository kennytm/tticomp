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
#include "OTheadTable.h"
#include "OTGlyph.h"
#include "OTTags.h"
#include "OTlocaTable.h"

using util::String;

namespace OpenType {

headTable::headTable (OpenTypeFile &aFont, MemoryBlockPtr memory) : Table (aFont) {
	MemoryPen pen (memory);
	Fixed version = pen.readFixed();
	if (version != 0x00010000)
		throw Exception ("Unknown table version " + String (version, 16));

	fontRevision = pen.readFixed();
	// checkSumAdjustment
	pen.readULong();
	magicNumber = pen.readULong();
	if (magicNumber != 0x5F0F3CF5)
		font.addWarning (new Exception ("Invalid magic number: " +
			String (magicNumber, 16)));

	flags = pen.readUShort();
	unitsPerEm = pen.readUShort();
	created = pen.readLongDateTime();
	modified = pen.readLongDateTime();
	xMin = pen.readShort();
	yMin = pen.readShort();
	xMax = pen.readShort();
	yMax = pen.readShort();
	macStyle = pen.readUShort();
	lowestRecPPEM = pen.readUShort();

	fontDirectionHint = pen.readShort();
	if (fontDirectionHint < -2 || fontDirectionHint > 2)
		throw Exception ("Undefined font direction hint value: " +
			String (fontDirectionHint));

	indexToLocFormat = pen.readShort();
	if (indexToLocFormat < 0 || indexToLocFormat > 1)
		throw Exception ("Unknown index to location format: " +
			String (indexToLocFormat));

	glyphDataFormat = pen.readShort();
	if (glyphDataFormat != 0)
		throw Exception ("Unknown glyph data format: " +
			String (glyphDataFormat));
}

headTable::~headTable() {}

Tag headTable::getTag() const {
	return headTag;
}

MemoryBlockPtr headTable::getMemory() const {
	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);

	// version
	pen.writeFixed (0x00010000);
	pen.writeFixed (fontRevision);
	// checkSumAdjustment
	pen.writeULong (0);
	// Magic number
	pen.writeULong (0x5F0F3CF5);
	pen.writeUShort (flags);

	pen.writeUShort (unitsPerEm);
	pen.writeLongDateTime (created);
	pen.writeLongDateTime (modified);
	pen.writeShort (xMin);
	pen.writeShort (yMin);
	pen.writeShort (xMax);
	pen.writeShort (yMax);
	pen.writeUShort (macStyle);
	pen.writeUShort (lowestRecPPEM);
	pen.writeShort (fontDirectionHint);
	pen.writeShort (indexToLocFormat);
	pen.writeShort (glyphDataFormat);

	return memory;
}


Short headTable::getIndexToLocFormat() {
	return indexToLocFormat;
}

UShort headTable::getUnitsPerEm() {
	return unitsPerEm;
}

void headTable::reset (const Glyphs &glyphs, const locaTable & loca) {
	Glyphs::const_iterator g;

	xMin = yMin = 0x7FFF;
	xMax = yMax = -0x8000;

	bool allEmpty = true;
	for (g = glyphs.begin(); g != glyphs.end(); g ++) {
		if (!(*g)->isEmpty()) {
			allEmpty = false;
			(*g)->getBoundingBox (this);
		}
	}
	if (allEmpty)
		xMin = yMin = xMax = yMax = 0;

	
	indexToLocFormat = loca.getFormat();
}

void headTable::setBoundingBox (const Glyph::BoundingBox & boundingBox) {
	if (boundingBox.xMin < xMin)
		xMin = boundingBox.xMin;
	if (boundingBox.yMin < yMin)
		yMin = boundingBox.yMin;

	if (boundingBox.xMax > xMax)
		xMax = boundingBox.xMax;
	if (boundingBox.yMax > yMax)
		yMax = boundingBox.yMax;
}

} // end namespace OpenType
