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
	\file Represents an OpenType head Table.
*/

#ifndef OTHEADTABLE_H
#define OTHEADTABLE_H

#include "OpenTypeFile.h"
#include "OTTable.h"
#include "OTGlyph.h"

namespace OpenType {

	class headTable : public Table {
			// Set by font manufacturer.
		Fixed fontRevision;
			// Set to 0x5F0F3CF5. 
		ULong magicNumber;
		UShort flags;
			//  Valid range is from 16 to 16384. This value should be a power of 2 for fonts that have TrueType outlines. 
		UShort unitsPerEm;
			// Number of seconds since 12:00 midnight, January 1, 1904. 64-bit integer 
		LongDateTime created;
			// Number of seconds since 12:00 midnight, January 1, 1904. 64-bit integer 
		LongDateTime modified;
		Short xMin;
		Short yMin;
		Short xMax;
		Short yMax;
		UShort macStyle;
			// Smallest readable size in pixels. 
		UShort lowestRecPPEM;
			// 0: Fully mixed directional glyphs; 1: Only strongly left to right; 2: Like 1 but also contains neutrals; -1: Only strongly right to left; -2: Like -1 but also contains neutrals.
		Short fontDirectionHint;
			// 0 for short offsets, 1 for long. 
		Short indexToLocFormat;
			// 0 for current format.
		Short glyphDataFormat;

	public:
		headTable (OpenTypeFile &aFont, MemoryBlockPtr memory);
		virtual ~headTable();

		virtual MemoryBlockPtr getMemory() const;
		virtual Tag getTag() const;

	protected:
		friend class locaTable;
		Short getIndexToLocFormat();

		friend class OpenTypeFont;
		UShort getUnitsPerEm();
		void reset (const std::vector <util::smart_ptr <Glyph> > &glyphs, const locaTable & loca);

		friend class Glyph;
		void setBoundingBox (const Glyph::BoundingBox & boundingBox);
	};
}

#endif // HEADTABLE_H
