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


#ifndef OTLOCATABLE_H
#define OTLOCATABLE_H

#include "OpenTypeFile.h"
#include "OTTable.h"

namespace OpenType {
	class maxpTable;
	class headTable;

	typedef std::vector <ULong> Locations;

	class locaTable : public Table {
		Short format;
		Locations locations;

		Short getFormat() const { return format; }
		friend class headTable;

	public:
		locaTable (OpenTypeFile &aFont);
		locaTable (OpenTypeFile &aFont, util::smart_ptr <maxpTable> maxpTable,
			util::smart_ptr <headTable> headTable, MemoryBlockPtr memory);
		virtual ~locaTable();

		virtual MemoryBlockPtr getMemory() const;
		virtual Tag getTag() const;
		Locations &getLocations();
		void addLocation (ULong location);
	};
}

#endif // OTLOCATABLE_H

