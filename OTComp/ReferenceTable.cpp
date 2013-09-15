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

#include <algorithm>
#include <deque>
#include "ReferenceTable.h"

using std::vector;
using std::deque;
using std::sort;

namespace OpenType {

/*** ReferenceMemoryBlock ***/

ReferenceMemoryBlock::ReferenceMemoryBlock (ULong aSize) : MemoryBlock (aSize) {}

ReferenceMemoryBlock::~ReferenceMemoryBlock() {}

/*** ReferenceMemoryPen ***/

ReferenceMemoryPen::ReferenceMemoryPen () : MemoryWritePen() {}

ReferenceMemoryPen::ReferenceMemoryPen (ReferenceMemoryBlockPtr aBlock)
: MemoryWritePen (aBlock) {}

ReferenceMemoryPen::ReferenceMemoryPen (const ReferenceMemoryPen & pen) : MemoryWritePen (pen) {}

ReferenceMemoryPen::~ReferenceMemoryPen() {}

void ReferenceMemoryPen::writeReference (ReferenceMemoryBlockPtr reference) {
	if (reference) {
		ReferenceMemoryBlock::Reference ref;
		ref.position = position;
		ref.block = reference;
		util::smart_ptr_cast <ReferenceMemoryBlock> (block)->references.push_back (ref);
	}
	writeOffset (0);
}

/*** ReferenceTable ***/

ReferenceTable::ReferenceTable (OpenTypeFile & aFont, Tag aTag)
	: Table (aFont), tag (aTag) {}

ReferenceTable::~ReferenceTable() {}

Tag ReferenceTable::getTag() const {
	return tag;
}

void ReferenceTable::setLevel (SubTables::iterator table, int level) const {
	if (table->level < level)
	{
		table->level = level;
		for (ReferenceMemoryBlock::References::iterator i = table->block->references.begin();
		i != table->block->references.end(); i ++)
		{
			SubTables::iterator j = subTables.begin();
			while (true) {
				if (j->block == i->block) {
					setLevel (j, level + 1);
					break;
				}
				j ++;
				assert (j != subTables.end());
			}
		}
	}
}

bool ReferenceTable::compareSubTablesByLevel (const SubTable & t1, const SubTable & t2) {
	return t1.level < t2.level;
}

struct Reference {
	MemoryPen blockStart;
	MemoryWritePen position;
	ReferenceMemoryBlockPtr block;
};

MemoryBlockPtr ReferenceTable::getMemory() const {
	// Set levels
	SubTables::iterator t;
	for (t = subTables.begin(); t != subTables.end(); t ++)
			setLevel (t, 0);

	// Sort tables by levels
	sort (subTables.begin(), subTables.end(), compareSubTablesByLevel);

	// Write table data
	MemoryBlockPtr memory = new MemoryBlock();
	MemoryWritePen pen (memory);

	// change back into deque after debugging!
	typedef vector <Reference> References;
	// The references that are yet to be written are kept here.
	// As the tables normally reference a table on the next level,
	// it should normally not be very large.
	References references;

	for (t = subTables.begin(); t != subTables.end(); t ++) {
		// Write old references
		for (References::iterator r = references.begin(); r != references.end();) {
			if (r->block == t->block) {
				r->position.writeOffset (pen - r->blockStart);
				r = references.erase (r);
			} else
				r ++;
		}
		// Add new references
		for (ReferenceMemoryBlock::References::iterator n = t->block->references.begin(); n != t->block->references.end(); n ++) {
			Reference r;
			r.blockStart = pen;
			r.position = pen + n->position;
			r.block = n->block;
			references.push_back (r);
		}
		// Write table
		pen.writeBlock (t->block);
	}

	assert (references.empty());

	return memory;
}

void ReferenceTable::add (ReferenceMemoryBlockPtr block) {
	assert (block);
	for (SubTables::iterator t = subTables.begin(); t != subTables.end(); t++) {
		// Table is already there
		if (t->block == block)
			return;
	}

	// Add references
	for (ReferenceMemoryBlock::References::iterator b = block->references.begin();
	b != block->references.end(); b++)
	{
		add (b->block);
	}

	SubTable s;
	s.block = block;
	s.level = -1;
	//subTables.push_front (s);
	subTables.insert (subTables.begin(), s);
}

} // end namespace OpenType
