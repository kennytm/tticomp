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

#include <fstream>
#include <vector>
#include <algorithm>

using std::ios;
using std::ifstream;
using std::ofstream;

using std::vector;
using std::lower_bound;
using std::upper_bound;

#include "OTException.h"
#include "OpenTypeFile.h"
#include "OTTable.h"
#include "OTTags.h"

using util::String;

namespace OpenType {

typedef struct {
	Tag tag;
	ULong offset;
	ULong length;
} DirectoryEntry;

bool compareDirectoryEntriesByOffsets (const DirectoryEntry &e1, const DirectoryEntry &e2) {
	return e1.offset < e2.offset;
}

bool compareTags (Tag tag1, Tag tag2) {
	const char * t1 = (const char *) &tag1;
	const char * t2 = (const char *) &tag2;
	return strncmp ((const char *) & tag1, (const char *) & tag2, 4) < 0;
}

bool compareTablesByTags (const TablePtr t1, const TablePtr t2) {
	return compareTags (t1->getTag(), t2->getTag());
}

OpenTypeFile::OpenTypeFile () {}

OpenTypeFile::~OpenTypeFile() {}

void OpenTypeFile::readFromFile (String aFileName) {
	fileName = aFileName;
	Exception::FontContext c1 (*this);
	Exception::Context c2 ("reading from file");

	ifstream file (fileName.getCString(), ios::in | ios::binary);
	if (!file.is_open())
		throw Exception ("Could not open file");

	MemoryBlockPtr offsetTable (new MemoryBlock (file, 12));
	MemoryBlockPtr test (offsetTable);

	// Read offset table
	MemoryPen pen (offsetTable);
	Fixed version = pen.readFixed();
	if (version != 0x00010000)
		throw Exception ("Unknown sfnt version: " + String (version, 16));

	UShort tableNum = pen.readUShort();

	// Disregard rest of the offset table

	typedef vector <DirectoryEntry> DirectoryEntryVector;
	DirectoryEntryVector entries;
	entries.reserve (tableNum);

	MemoryBlockPtr tableDirectory (new MemoryBlock (file, 16*tableNum));
	pen.set (tableDirectory);


	UShort i;
	Tag prevTag = 0;
	for (i = 0; i < tableNum; i++) {
		DirectoryEntry entry;
		entry.tag = pen.readTag();
		if (!compareTags (prevTag, entry.tag))
			addWarning (new Exception ("Tag " + tagToString (entry.tag) + " not in alphabetical order"));
		prevTag = entry.tag;
		// Checksum
		pen.readULong();
		entry.offset = pen.readULong();
		entry.length = pen.readULong();
		entries.insert (upper_bound (entries.begin(), entries.end(), entry,
			compareDirectoryEntriesByOffsets), entry);
	}

	ULong curOffset = 12 + 16*tableNum;
	Tag previousTag = 0;
	DirectoryEntryVector::iterator e;
	for (e = entries.begin(); e != entries.end(); e ++) {
		if (e->offset < curOffset)
			throw Exception ("OpenType tables '" + tagToString (previousTag) + "' and '"
				+ tagToString (e->tag) + "' overlap");

		if (OT_PAD_TO (curOffset, 4) < e->offset)
			addWarning (new Exception ("Gap between OpenType tables '" +
				tagToString (previousTag) + "' and '" + tagToString (e->tag) + "'"));

		curOffset = e->offset;
		file.seekg(e->offset);
		MemoryBlockPtr memory (new MemoryBlock (file, e->length));

		addTable (new UnknownTable (*this, e->tag, memory));

		curOffset += e->length;
		previousTag = e->tag;
	}

	file.close();
}

String OpenTypeFile::getFileName() const {
	return fileName;
}

Tables::iterator OpenTypeFile::getTableIterator (ULong tag) {
	Tables::iterator lowest, highest, guess;
	lowest = tables.begin();
	highest = tables.end();
	while (lowest != highest) {
		guess = lowest  + (highest - lowest) / 2;
		ULong foundTag = (*guess)->getTag();
		if (tag == foundTag)
			return guess;
		if (compareTags (tag, foundTag))
			highest = guess;
		else
			lowest = guess + 1;
	}
	
	return tables.end();
}

/*	TablePtr getTable (ULong tag, bool fail);
	bool addTable (TablePtr newTable, bool fail);
	bool deleteTable (ULong tag, bool fail);
	bool replaceTable (TablePtr newTable, bool fail);*/

TablePtr OpenTypeFile::getTable (ULong tag, bool fail) {
	Exception::FontContext c (*this);

	Tables::iterator pos = getTableIterator (tag);
	if (pos == tables.end() || (*pos)->getTag() != tag) {
		if (fail)
			throw Exception ("No '" + tagToString (tag) +
				"' table was found");
		else
			return NULL;
	} else
		return *pos;
}

bool OpenTypeFile::addTable (TablePtr newTable, bool fail) {
	Exception::FontContext c (*this);

	Tables::iterator pos = lower_bound (tables.begin(), tables.end(),
		newTable, compareTablesByTags);

	if (pos != tables.end() && (*pos)->getTag() == newTable->getTag()) {
		// Table already exists
		if (fail)
			throw Exception ("There is already a '" + tagToString(newTable->getTag()) +
				"' table");
		else
			return false;
	} else {
		tables.insert (pos, newTable);
		return true;
	}
}


bool OpenTypeFile::deleteTable (ULong tag, bool fail) {
	Exception::FontContext c (*this);
	Tables::iterator pos = getTableIterator (tag);

	if (pos == tables.end() || (*pos)->getTag() != tag) {
		// Table does not exist
		if (fail)
			throw Exception ("No '" + tagToString (tag) +
				"' table was found");
		else
			return false;
	} else {
		tables.erase (pos);
		return true;
	}
}

bool OpenTypeFile::replaceTable (TablePtr newTable, bool fail) {
	Exception::FontContext c (*this);
	Tables::iterator pos = getTableIterator (newTable->getTag());

	if (pos == tables.end() || (*pos)->getTag() != newTable->getTag()) {
		// Table does not exist yet
		if (fail)
			throw Exception ("No '" + tagToString (newTable->getTag()) +
				"' table was found");
		else {
			// Add table anyway
			tables.insert (lower_bound (tables.begin(), tables.end(), newTable, compareTablesByTags),
				newTable);
			return false;
		}
	} else {
		*pos = newTable;
		return true;
	}
}


/***
	writeToFile()
	From the Microsoft Font Validator Help:
		OpenType fonts with TrueType outlines are most efficiently utilized by
		Windows when the tables are ordered as follows (from first to last):
		head, hhea, maxp, OS/2, hmtx, LTSH, VDMX, hdmx, cmap, fpgm, prep, cvt,
		loca, glyf, kern, name, post, gasp, PCLT, DSIG

		The initial loading of an OpenType font containing CFF data will be
		more efficient if the following table ordering is used within the body
		of the font (from first to last):
		head, hhea, maxp, OS/2, name, cmap, post, CFF, other tables (as convenient)

	CFF fonts are not (yet) supported. We keep to the Windows Ideal Order.
***/

class TableBlock {
public:
	Tag tag;
	MemoryBlockPtr memory;
	MemoryWritePen dirEntryPen;
	ULong checksum;

	TableBlock(Tag aTag, MemoryBlockPtr aMemory, MemoryWritePen aPen)
		: tag (aTag), memory (aMemory), checksum(aMemory->getChecksum()), dirEntryPen (aPen) {}
	TableBlock & operator= (const TableBlock &s) {
		tag = s.tag;
		memory = s.memory;
		checksum = s.checksum;
		dirEntryPen = s.dirEntryPen;
		return *this;
	}
};

int getOrderedTagNumber (Tag tag) {
	switch (tag) {
	case headTag: return 0;
	case hheaTag: return 1;
	case maxpTag: return 2;
	case OS2Tag: return 3;
	case hmtxTag: return 4;
	case LTSHTag: return 5;
	case VDMXTag: return 6;
	case hdmxTag: return 7;
	case cmapTag: return 8;
	case fpgmTag: return 9;
	case prepTag: return 10;
	case cvtTag: return 11;
	case locaTag: return 12;
	case glyfTag: return 13;
	case kernTag: return 14;
	case nameTag: return 15;
	case postTag: return 16;
	case gaspTag: return 17;
	case PCLTTag: return 18;
	case DSIGTag: return 19;
	default: return 100;
	}
}
	
bool compareTableBlocksByIdealPosition (const TableBlock &b1, const TableBlock &b2) {
	int b1t = getOrderedTagNumber (b1.tag);
	int b2t = getOrderedTagNumber (b2.tag);
	if (b1t < b2t)
		return true;
	if (b1t > b2t)
		return false;
	return compareTags (b1.tag, b2.tag);
}

void OpenTypeFile::writeToFile (String outFileName) {
	Exception::FontContext c1 (*this);
	Exception::Context c2 ("writing tables to file \"" + outFileName + '"');
	ofstream file (outFileName.getCString(), ios::out | ios::binary);

	if (!file.is_open())
		throw Exception ("Could not open file");

	MemoryBlockPtr offsetTable (new MemoryBlock (12));
	MemoryWritePen pen (offsetTable);

	// version
	pen.writeFixed (0x00010000);
	pen.writeUShort (tables.size());

	UShort maxPower2 = 1;
	UShort entrySelector = 0;
	while (maxPower2 <= tables.size()) {
		maxPower2 <<= 1;
		entrySelector ++;
	}
	maxPower2 >>= 1;
	entrySelector --;
	// Search range
	pen.writeUShort (maxPower2 * 16);
	pen.writeUShort (entrySelector);
	pen.writeUShort ((tables.size() - maxPower2) * 16);

	// Get table blocks and order them
	ULong totalChecksum = 0;
	MemoryBlockPtr headMemory;

	typedef vector <TableBlock> TableBlockVector;
	TableBlockVector tableBlocks;
	Tables::iterator i;
	for (i = tables.begin(); i != tables.end(); i ++) {
		TableBlock block (
			(*i)->getTag(),
			(*i)->getMemory(),
			pen);

		totalChecksum += block.checksum;
		pen += 16;
		tableBlocks.insert (upper_bound (tableBlocks.begin(), tableBlocks.end(),
			block, compareTableBlocksByIdealPosition), block);
	}

	ULong curOffset = 12 + 16 * tables.size();
	// Write directory entries
	TableBlockVector::iterator b;
	for (b = tableBlocks.begin(); b != tableBlocks.end(); b++ ) {

		(*b).dirEntryPen.writeTag ((*b).tag);
		if ((*b).tag == headTag)
			headMemory = (*b).memory;
		(*b).dirEntryPen.writeULong ((*b).checksum);
		(*b).dirEntryPen.writeULong (curOffset);

		(*b).dirEntryPen.writeULong ((*b).memory->getSize(1));

		curOffset += (*b).memory->getSize(4);
	}

	if (headMemory) {
		// Add font checksum to 'head' table
		totalChecksum += offsetTable->getChecksum();
		pen = MemoryWritePen (headMemory) + 8;
		pen.writeULong (0xB1B0AFBA - totalChecksum);
	}

	offsetTable->write (file, 4);

	// Write the blocks to file

	for (b = tableBlocks.begin(); b != tableBlocks.end(); b++)
		(*b).memory->write (file, 4);

	file.close();
}

/*bool OpenTypeFile::areWarnings() const {
	return !warnings.empty();
}

ExceptionPtr OpenTypeFile::popWarning() {
	ExceptionPtr w = warnings.front();
	warnings.pop_front();
	return w;
}*/

String tagToString (Tag tag) {
	String s;
	const char *t = (const char*) &tag;
	int l;
	if (t[3] != ' ')
		l = 4;
	else {
		if (t[2] != ' ')
			l = 3;
		else {
			if (t[1] != ' ')
				l = 2;
			else {
				l = 1;
			}
		}
	}
	int i;
	for (i = 0; i < l; i ++)
		s += t [i];
	return s;
}

} // end namespace OpenType
