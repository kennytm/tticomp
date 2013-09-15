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
#include "OTmaxpTable.h"
#include "OTGlyph.h"
#include "OTTags.h"

using util::String;

namespace OpenType {

maxpTable::maxpTable (OpenTypeFile &aFont, MemoryBlockPtr memory)
: Table (aFont) {
	MemoryPen pen (memory);
	Fixed version = pen.readFixed();
	if (version != 0x00010000)
		throw Exception (
			"Unknown maxp table version number: " + String (version, 16));

	glyphNum = pen.readUShort();
	maxPoints = pen.readUShort();
	maxContours = pen.readUShort();
	maxCompositePoints = pen.readUShort();
	maxCompositeContours = pen.readUShort();
	maxZones = pen.readUShort();
	maxTwilightPoints = pen.readUShort();
	maxStorage = pen.readUShort();
	maxFunctionDefs = pen.readUShort();
	maxInstructionDefs = pen.readUShort();
	maxStackElements = pen.readUShort();
	maxSizeOfInstructions = pen.readUShort();
	maxComponentElements = pen.readUShort();
	maxComponentDepth = pen.readUShort();
}

maxpTable::~maxpTable() {}

Tag maxpTable::getTag() const {
	return maxpTag;
}

MemoryBlockPtr maxpTable::getMemory() const {
	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);

	// version
	pen.writeFixed (0x00010000);
	pen.writeUShort(glyphNum);
	pen.writeUShort(maxPoints);
	pen.writeUShort(maxContours);
	pen.writeUShort(maxCompositePoints);
	pen.writeUShort(maxCompositeContours);
	pen.writeUShort(maxZones);
	pen.writeUShort(maxTwilightPoints);
	pen.writeUShort(maxStorage);
	pen.writeUShort(maxFunctionDefs);
	pen.writeUShort(maxInstructionDefs);
	pen.writeUShort(maxStackElements);
	pen.writeUShort(maxSizeOfInstructions);
	pen.writeUShort(maxComponentElements);
	pen.writeUShort(maxComponentDepth);

	return memory;
}

UShort maxpTable::getGlyphNum() {
	return glyphNum;
}

UShort maxpTable::getMaxStorage() {
	return maxStorage;
}

UShort maxpTable::getMaxTwilightPoints() {
	return maxTwilightPoints;
}

UShort maxpTable::getMaxStackElements() {
	return maxStackElements;
}

UShort maxpTable::getMaxFunctionDefs() {
	return maxFunctionDefs;
}

void maxpTable::setMaxStorage (UShort aMaxStorage) {
	maxStorage = aMaxStorage;
}

void maxpTable::setMaxFunctionDefs (UShort aMaxFunctionDefs) {
	maxFunctionDefs = aMaxFunctionDefs;
}

void maxpTable::setMaxInstructionDefs (UShort aMaxInstructionDefs) {
	maxInstructionDefs = aMaxInstructionDefs;
}

void maxpTable::setMaxSizeOfInstructions (UShort aMaxSizeOfInstructions) {
	maxSizeOfInstructions = aMaxSizeOfInstructions;
}

void maxpTable::setMaxStackElements (UShort aMaxStackElements) {
	maxStackElements = aMaxStackElements;
}

void maxpTable::setMaxZones (UShort aMaxZones) {
	maxZones = aMaxZones;
}

void maxpTable::setMaxTwilightPoints (UShort aMaxTwilightPoints) {
	maxTwilightPoints = aMaxTwilightPoints;
}

void maxpTable::reset (const Glyphs &glyphs) {
	glyphNum = glyphs.size();

	maxPoints = 0;
	maxContours = 0;
	maxCompositePoints = 0;
	maxCompositeContours = 0;
	maxSizeOfInstructions = 0;
	maxComponentElements = 0;
	maxComponentDepth = 0;

	Glyphs::const_iterator g;
	for (g = glyphs.begin(); g != glyphs.end(); g ++)
		(*g)->getMaximumProfile (this);
}

void maxpTable::setMaximumProfile (UShort aMaxPoints, UShort aMaxContours,
	UShort aMaxCompositePoints, UShort aMaxCompositeContours,
	UShort aMaxSizeOfInstructions, UShort aMaxComponentElements,
	UShort aMaxComponentDepth)
{
	if (aMaxPoints > maxPoints)
		maxPoints = aMaxPoints;
	if (aMaxContours > maxContours)
		maxContours = aMaxContours;

	if (aMaxCompositePoints > maxCompositePoints)
		maxCompositePoints = aMaxCompositePoints;
	if (aMaxCompositeContours > maxCompositeContours)
		maxCompositeContours = aMaxCompositeContours;

	if (aMaxSizeOfInstructions > maxSizeOfInstructions)
		maxSizeOfInstructions = aMaxSizeOfInstructions;
	if (aMaxComponentElements > maxComponentElements)
		maxComponentElements = aMaxComponentElements;
	if (aMaxComponentDepth > maxComponentDepth)
		maxComponentDepth = aMaxComponentDepth;
}

} // end namespace OpenType
