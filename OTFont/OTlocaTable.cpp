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

#include "OTlocaTable.h"
#include "OTmaxpTable.h"
#include "OTheadTable.h"
#include "OTTags.h"

using util::smart_ptr;

namespace OpenType {

locaTable::locaTable (OpenTypeFile &aFont) : Table (aFont), format (0) {}

locaTable::locaTable (OpenTypeFile &aFont, smart_ptr <maxpTable> maxpTable,
					  smart_ptr <headTable> headTable, MemoryBlockPtr memory)
: Table (aFont)
 {
	MemoryPen pen (memory);
	UShort glyphNum = maxpTable->getGlyphNum();
	format = headTable->getIndexToLocFormat();

	locations.reserve (glyphNum);

	UShort i;
	if (format == 0) {
		for (i = 0; i <= glyphNum; i++)
			locations.push_back (pen.readUShort() * 2);
	} else {
		for (i = 0; i <= glyphNum; i++)
			locations.push_back (pen.readULong());
	}
}

locaTable::~locaTable() {}

Tag locaTable::getTag() const {
	return locaTag;
}

MemoryBlockPtr locaTable::getMemory() const {
	assert (!locations.empty());
	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);

	Locations::const_iterator i;

	if (format == 0) {
		for (i = locations.begin(); i != locations.end(); i++)
			pen.writeUShort (*i /2);
	} else {
		for (i = locations.begin(); i != locations.end(); i++)
			pen.writeULong (*i);
	}
	
	return memory;
}

Locations & locaTable::getLocations() {
	return locations;
}

void locaTable::addLocation (ULong location) {
	// Should be an even number
	assert (location % 2 == 0);

	locations.push_back (location);
	if (location > 0xFFFF)
		format = 1;
}

} // end namespace OpenType
