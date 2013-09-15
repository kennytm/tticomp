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
	\file The OTcmapTable class provides access to the cmap table.
*/

#ifndef OTCMAPTABLE_H
#define OTCMAPTABLE_H

#include <vector>

#include "OTTable.h"

namespace OpenType {

	class MappingTable;
	typedef util::smart_ptr <MappingTable> MappingTablePtr;
	typedef std::vector <MappingTablePtr> MappingTables;

	class MappingTable {
		UShort platformId;
		UShort encodingId;

	protected:
		OpenTypeFile &font;

		friend class cmapTable;
		virtual MemoryBlockPtr getMemory() const = 0;
	public:
		MappingTable (OpenTypeFile &aFont, UShort aPlatformId, UShort aEncodingId)
			: font (aFont), platformId (aPlatformId), encodingId (aEncodingId) {}
		virtual ~MappingTable();

		virtual bool isKnown() const = 0;

		UShort getPlatformId() const { return platformId; }
		UShort getEncodingId() const { return encodingId; }
		virtual UShort getFormatId() const = 0;

		virtual bool mappingExists (ULong code) const = 0;
		virtual UShort getGlyphId (ULong code) const = 0;
		virtual ULong getCode (UShort glyphId) const = 0;

		virtual void clear() = 0;
		virtual void addMapping (ULong code, UShort glyphId) = 0;
		virtual void mergeFrom (MappingTablePtr table,
			bool warnDifferent = true, bool warnOutOfBounds = false) = 0;
	};

	class cmapTable : public Table {
		MappingTables mappingTables;
	public:
		cmapTable (OpenTypeFile &aFont, MemoryBlockPtr memory);
		virtual ~cmapTable();

		virtual MemoryBlockPtr getMemory() const;
		virtual Tag getTag() const;

//		const MappingTables & getMappingTables() const;
		MappingTablePtr getFirstKnownMappingTable (UShort platformId, UShort encodingId);
		MappingTablePtr getFirstKnownMappingTable (UShort platformId, UShort encodingId, UShort formatId);

		MappingTablePtr newMappingTable (UShort platformId, UShort encodingId, UShort formatId);
	};
}

#endif // OTCMAPTABLE_H
