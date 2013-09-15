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

// Some comments in this file may be parsed by doygen to yield nice
// documentation files for the source.

/**
	\file OTOS2Table represents an OpenType OS/2 table. The table version is
	retained.
*/

#ifndef OTOS2TABLE_H
#define OTOS2TABLE_H

#include "OTTable.h"

namespace OpenType {

	class OS2Table : public Table {
		// Table contents
		// 0x0003
		UShort version;
		Short xAvgCharWidth;
		UShort usWeightClass;
		UShort usWidthClass;
		UShort fsType;
		Short ySubscriptXSize;
		Short ySubscriptYSize;
		Short ySubscriptXOffset;
		Short ySubscriptYOffset;
		Short ySuperscriptXSize;
		Short ySuperscriptYSize;
		Short ySuperscriptXOffset;
		Short ySuperscriptYOffset;
		Short yStrikeoutSize;
		Short yStrikeoutPosition;
		Short sFamilyClass;
		Byte panose[10];
		// Bits 0-31
		ULong ulUnicodeRange1;
		// Bits 32-63
		ULong ulUnicodeRange2;
		// Bits 64-95
		ULong ulUnicodeRange3;
		// Bits 96-127
		ULong ulUnicodeRange4;
		Char achVendID[4];
		UShort fsSelection;
		UShort usFirstCharIndex;
		UShort usLastCharIndex;
		Short sTypoAscender;
		Short sTypoDescender;
		Short sTypoLineGap;
		UShort usWinAscent;
		UShort usWinDescent;

		// From here on only in version >= 1
		// Bits 0-31
		ULong ulCodePageRange1;
		// Bits 32-63
		ULong ulCodePageRange2;
		// From here on only in version >= 2
		Short sxHeight;
		Short sCapHeight;
		UShort usDefaultChar;
		UShort usBreakChar;
		UShort usMaxContext;

	public:
		OS2Table (OpenTypeFile &aFont, MemoryBlockPtr memory);
		virtual ~OS2Table();

		virtual MemoryBlockPtr getMemory() const;
		virtual Tag getTag() const;

	protected:
		friend class OpenTypeFont;
		UShort getWinAscent();
		UShort getWinDescent();
	};
}

#endif	//	OTOS2TABLE_H
