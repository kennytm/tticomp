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


#ifndef OTHMTXTABLE_H
#define OTHMTXTABLE_H

#include <vector>

#include "OpenTypeFile.h"
#include "OTTable.h"

namespace OpenType {

	// From the MS OpenType specs
	typedef struct {
		UShort advanceWidth;
		Short lsb;
	} HorMetric;

	class hheaTable;
	class maxpTable;

	class Glyph;
	typedef util::smart_ptr <Glyph> GlyphPtr;
	typedef std::vector <GlyphPtr> Glyphs;

	typedef std::vector <HorMetric> HorMetrics;

	class hmtxTable : public Table {
		HorMetrics horMetrics;

		HorMetrics::const_iterator getLastFullMetric() const;

	public:
		hmtxTable (OpenTypeFile &aFont, util::smart_ptr <hheaTable> hheaTable,
			const Glyphs &glyphs);
		hmtxTable (OpenTypeFile &aFont, util::smart_ptr <maxpTable> maxpTable,
			util::smart_ptr <hheaTable> hheaTable, MemoryBlockPtr memory);
		virtual ~hmtxTable();

		virtual MemoryBlockPtr getMemory() const;
		virtual Tag getTag() const;

		HorMetric getHorMetric (UShort glyphIndex);
	};
}

#endif	// OTHMTXTABLE_H
