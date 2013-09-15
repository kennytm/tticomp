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
	\file OTpostTable represents an OpenType post Table.
*/

#ifndef OTPOSTTABLE_H
#define OTPOSTTABLE_H

#include "OpenTypeFile.h"
#include "OTTable.h"

namespace OpenType {
	class Glyph;

	class postTable : public Table {
		typedef std::vector <util::String> StringVector;

		// Version
		Fixed version;
		// post table information
		Fixed italicAngle;
		FWord underlinePosition;
		FWord underlineThickness;
		ULong isFixedPitch;
		ULong minMemType42;
		ULong maxMemType42;
		ULong minMemType1;
		ULong maxMemType1;

		StringVector postNames;

	public:
		postTable (OpenTypeFile &aFont, MemoryBlockPtr aMemory);
		virtual ~postTable();

		virtual MemoryBlockPtr getMemory() const;
		virtual Tag getTag() const;

		util::String getPostName (UShort index);

		void clear();
		void set (const std::vector <util::smart_ptr <Glyph> > &glyphs);
	};
}

#endif	// OTPOSTTABLE_H
