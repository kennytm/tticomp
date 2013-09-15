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

/**
	\file ReferenceTable.h defines ReferenceTable and ReferenceMemoryBlock, which
	are used to produce OpenType tables that contain many linked subtables
	(like the GSUB and GPOS tables).
*/

#ifndef REFERENCETABLE_H
#define REFERENCETABLE_H

#include <vector>
#include <deque>
#include "../OTFont/OTMemoryBlock.h"
#include "../OTFont/OTTable.h"

using util::smart_ptr;

namespace OpenType {

	class ReferenceMemoryBlock;

	typedef smart_ptr <ReferenceMemoryBlock> ReferenceMemoryBlockPtr;

	class ReferenceMemoryBlock : public MemoryBlock {
	protected:
		friend class ReferenceMemoryPen;
		friend class ReferenceTable;
		struct Reference {
			ULong position;
			ReferenceMemoryBlockPtr block;
		};
		typedef std::vector <Reference> References;
		References references;

	public:
		ReferenceMemoryBlock (ULong aSize = 0);
		virtual ~ReferenceMemoryBlock();
	};

	class ReferenceMemoryPen : public MemoryWritePen {
	public:
		ReferenceMemoryPen();
		explicit ReferenceMemoryPen (ReferenceMemoryBlockPtr aBlock);
		ReferenceMemoryPen (const ReferenceMemoryPen &pen);
		virtual ~ReferenceMemoryPen();

		void writeReference (ReferenceMemoryBlockPtr block);
	};

	class ReferenceTable : public Table {
	protected:
		struct SubTable {
			ReferenceMemoryBlockPtr block;
			int level;
		};
		static bool compareSubTablesByLevel (const SubTable & t1, const SubTable & t2);

	private:
		Tag tag;

		// change back into deque after debugging!
		typedef std::vector <SubTable> SubTables;
		mutable SubTables subTables;

		void setLevel (SubTables::iterator table, int level) const;
	public:
		ReferenceTable (OpenTypeFile &aFont, Tag aTag);
		virtual ~ReferenceTable();

		virtual MemoryBlockPtr getMemory() const;
		virtual Tag getTag() const;

		void add (ReferenceMemoryBlockPtr block);
	};

	typedef smart_ptr <ReferenceTable> ReferenceTablePtr;
}

#endif // REFERENCETABLE_H
