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
	\file OTgaspTable represents an OpenType 'gasp' table.
	It can be used for writing only.
*/

#ifndef OTGASPTABLE_H
#define OTGASPTABLE_H

#include <vector>

#include "OTTable.h"

namespace OpenType {

	namespace GridFittingBehaviour {
		typedef enum {nothing = 0, gridFit = 0x01, greyscale = 0x02, both = 0x03} Behaviour;

		typedef struct {
			UShort maxPPEM;
			Behaviour behaviour;
		} Range;

		typedef std::vector <Range> Ranges;
	}

	class gaspTable : public Table {
		GridFittingBehaviour::Ranges ranges;
	public:
		gaspTable (OpenTypeFile &aFont, const GridFittingBehaviour::Ranges &aRanges);
		virtual ~gaspTable();

		virtual Tag getTag() const;
		virtual MemoryBlockPtr getMemory() const;
	};
}

#endif	// OTGASPTABLE_H
