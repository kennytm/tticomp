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
	\file OpenType.h %OpenType header definition: types for reading sfnt files
*/

#ifndef OPENTYPE_H
#define OPENTYPE_H

#include "../Util/smart_ptr.h"
#include "../Util/fixed.h"

/// Contains all OpenType font manipulation classes
namespace OpenType {
	typedef unsigned char	Byte;
	typedef signed   char	SignedByte;
	typedef unsigned short	UShort;
	typedef signed   short	Short;
	typedef unsigned int	ULong;
	typedef signed   int	Long;
	typedef unsigned int	Fixed;
	typedef unsigned short	Offset;
	typedef unsigned short	uint16;
	typedef signed   short	int16;
	typedef unsigned short	GlyphId;
	typedef signed   short	FWord;
	typedef unsigned short	UFWord;
	#ifdef _MSC_VER
	typedef signed __int64	LongDateTime;
	typedef signed __int64	LongLong;
	#else
	typedef signed long long	LongDateTime;
	typedef signed long long	LongLong;
	#endif
	typedef signed   int	F26Dot6;
	typedef util::fixed <2, 14> F2Dot14;
	typedef util::fixed <18, 14> F18Dot14;
	typedef util::fixed <26, 6> NewF26Dot6;
	typedef unsigned int	Tag;
	typedef unsigned char	Char;

	// Forward declarations

	class OpenTypeFile;
	class OpenTypeFont;
	class Exception;
	class Glyph;

	class MemoryBlock;
	class MemoryPen;
	class MemoryWritePen;

	class Table;
	class headTable;
	class hheaTable;
	class hmtxTable;
	class maxpTable;
	class locaTable;
	class postTable;
	class cmapTable;
	class OS2Table;
	class GSUBTable;
	class GPOSTable;
	class GDEFTable;
	class MappingTable;

	typedef util::smart_ptr <Exception> ExceptionPtr;
	typedef util::smart_ptr <MemoryBlock> MemoryBlockPtr;
	typedef util::smart_ptr <Glyph> GlyphPtr;
	typedef util::smart_ptr <MappingTable> MappingTablePtr;
	typedef util::smart_ptr <Table> TablePtr;
}

#define OT_PAD_TO(number, padding)  (((number) + ((padding) - 1)) & ~((padding) - 1))

/*#define OT_MULTIPLY_BY_F2DOT14(a, b) ((Long)((((LongLong)(a)) * (b) + 0x2000) >> 14))
#define OT_MULTIPLY_BY_F2DOT14_AND_ADD(a, b, c, d) \
	((Long)((((LongLong)(a)) * (b) + ((LongLong)(c)) * (d) + 0x2000) >> 14))*/

#endif // OPENTYPE_H
