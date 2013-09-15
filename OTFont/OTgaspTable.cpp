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
#include "OTgaspTable.h"
#include "OTTags.h"

using util::String;

namespace OpenType {

gaspTable::gaspTable (OpenTypeFile &aFont, const GridFittingBehaviour::Ranges &aRanges)
: Table (aFont) {
	if (aRanges.empty())
		throw Exception ("Ranges should not be empty");

	if (aRanges.back().maxPPEM != 0xFFFF)
		throw Exception ("Last entry should be 0xFFFF");

	GridFittingBehaviour::Ranges::const_iterator i;
	for (i = aRanges.begin(); i != aRanges.end(); i ++) {
		if (i != aRanges.begin()) {
			if ((i-1)->maxPPEM >= i->maxPPEM)
				throw Exception ("Entries are not in increasing maxPPEM order");
		}
		if (i->behaviour > 3)
			throw Exception ("Invalid behaviour flag " +
				String (i->behaviour, 16));
	}

	ranges = aRanges;
}

gaspTable::~gaspTable() {}

Tag gaspTable::getTag() const {
	return gaspTag;
}

MemoryBlockPtr gaspTable::getMemory() const {
	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);

	// version
	pen.writeUShort (0);
	pen.writeUShort (ranges.size());
	
	GridFittingBehaviour::Ranges::const_iterator r;
	for (r = ranges.begin(); r != ranges.end(); r ++) {
		pen.writeUShort (r->maxPPEM);
		pen.writeUShort (r->behaviour);
	}

	return memory;
}

} // end namespace OpenType
