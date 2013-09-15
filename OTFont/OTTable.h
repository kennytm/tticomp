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
	\file OTTable.h Represents an OpenType table.
*/

#ifndef OTTABLE_H
#define OTTABLE_H

#include "OTMemoryBlock.h"

namespace OpenType {
	class OpenTypeFile;

	/** \brief Table is the abstract base class for all %OpenType tables.

		The table is defined in terms of its input and output.
		No assumptions can be made about what the data means from the Table
		class.
	*/
	class Table {
	protected:
		OpenTypeFile &font;
	public:
		/// \brief Constructs a Table.
		Table(OpenTypeFile &aFont) : font(aFont) {}
		virtual ~Table();

		/** \brief Return a memory block that can be written to file.

			The memory block may be the original, unchanged memory block
			or it may be a newly generated memory block.
		*/
		virtual MemoryBlockPtr getMemory() const = 0;

		/// \brief Return the tag for the %OpenType table directory.
		virtual Tag getTag() const = 0;
	};

	/** \brief Save table information without changing or interpreting it.

		When the table is written back to file, it uses the exact memory
		block it was handed in the first place.
		To turn an UnknownTable into a known table you may ask for the
		memory block by calling getMemory(), use it for the new Table and
		discard the UnknownTable object.
	*/
	class UnknownTable : public Table {
		ULong tag;
		MemoryBlockPtr memory;

	public:
		UnknownTable (OpenTypeFile &aFont, ULong aTag, MemoryBlockPtr aMemory);
		virtual ~UnknownTable();

		/// \brief Return the memory block as it was read.
		virtual MemoryBlockPtr getMemory() const;
		virtual Tag getTag() const;
	};
}

#endif	// OTTABLE_H
