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

#include "OTException.h"
#include "OTOS2Table.h"
#include "OTTags.h"

using util::String;

namespace OpenType {

OS2Table::OS2Table (OpenTypeFile &aFont, MemoryBlockPtr memory)
: Table (aFont) {
	MemoryPen pen (memory);
	UShort i;
	version = pen.readUShort();
	if (version > 3)
		throw Exception ("Unknown table version " + String (version, 16));

	xAvgCharWidth = pen.readShort();
	usWeightClass = pen.readUShort();
	usWidthClass = pen.readUShort();
	fsType = pen.readUShort();
	ySubscriptXSize = pen.readShort();
	ySubscriptYSize = pen.readShort();
	ySubscriptXOffset = pen.readShort();
	ySubscriptYOffset = pen.readShort();
	ySuperscriptXSize = pen.readShort();
	ySuperscriptYSize = pen.readShort();
	ySuperscriptXOffset = pen.readShort();
	ySuperscriptYOffset = pen.readShort();
	yStrikeoutSize = pen.readShort();
	yStrikeoutPosition = pen.readShort();
	sFamilyClass = pen.readShort();
	for (i = 0; i < 10; i ++)
		panose[i] = pen.readByte();
	ulUnicodeRange1 = pen.readULong();
	ulUnicodeRange2 = pen.readULong();
	ulUnicodeRange3 = pen.readULong();
	ulUnicodeRange4 = pen.readULong();
	for (i = 0; i < 4; i ++)
		achVendID[i] = pen.readByte();
	fsSelection = pen.readUShort();
	usFirstCharIndex = pen.readUShort();
	usLastCharIndex = pen.readUShort();
	sTypoAscender = pen.readShort();
	sTypoDescender = pen.readShort();
	sTypoLineGap = pen.readShort();
	usWinAscent = pen.readUShort();
	usWinDescent = pen.readUShort();
	
	if (version >= 1) {
		// Bits 0-31
		ulCodePageRange1 = pen.readULong();
		// Bits 32-63
		ulCodePageRange2 = pen.readULong();
		if (version >= 2) {
			sxHeight = pen.readShort();
			sCapHeight = pen.readShort();
			usDefaultChar = pen.readUShort();
			usBreakChar = pen.readUShort();
			usMaxContext = pen.readUShort();
		}
	}
}

OS2Table::~OS2Table() {}

Tag OS2Table::getTag() const {
	return OS2Tag;
}

MemoryBlockPtr OS2Table::getMemory() const {
	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);
	UShort i;
	
	pen.writeUShort (version);
	pen.writeShort (xAvgCharWidth);
	pen.writeUShort (usWeightClass);
	pen.writeUShort (usWidthClass);
	pen.writeUShort (fsType);
	pen.writeShort (ySubscriptXSize);
	pen.writeShort (ySubscriptYSize);
	pen.writeShort (ySubscriptXOffset);
	pen.writeShort (ySubscriptYOffset);
	pen.writeShort (ySuperscriptXSize);
	pen.writeShort (ySuperscriptYSize);
	pen.writeShort (ySuperscriptXOffset);
	pen.writeShort (ySuperscriptYOffset);
	pen.writeShort (yStrikeoutSize);
	pen.writeShort (yStrikeoutPosition);
	pen.writeShort (sFamilyClass);
	for (i = 0; i < 10; i ++)
		pen.writeByte (panose[i]);
	pen.writeULong (ulUnicodeRange1);
	pen.writeULong (ulUnicodeRange2);
	pen.writeULong (ulUnicodeRange3);
	pen.writeULong (ulUnicodeRange4);
	for (i = 0; i < 4; i ++)
		pen.writeByte (achVendID[i]);
	pen.writeUShort (fsSelection);
	pen.writeUShort (usFirstCharIndex);
	pen.writeUShort (usLastCharIndex);
	pen.writeShort (sTypoAscender);
	pen.writeShort (sTypoDescender);
	pen.writeShort (sTypoLineGap);
	pen.writeUShort (usWinAscent);
	pen.writeUShort (usWinDescent);
	
	if (version >= 1) {
		// Bits 0-31
		pen.writeULong (ulCodePageRange1);
		// Bits 32-63
		pen.writeULong (ulCodePageRange2);
		if (version >= 2) {
			pen.writeShort (sxHeight);
			pen.writeShort (sCapHeight);
			pen.writeUShort (usDefaultChar);
			pen.writeUShort (usBreakChar);
			pen.writeUShort (usMaxContext);
		}
	}

	return memory;
}

UShort OS2Table::getWinAscent() {
	return usWinAscent;
}

UShort OS2Table::getWinDescent() {
	return usWinDescent;
}

} // end namespace OpenType
