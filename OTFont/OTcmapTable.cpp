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
#include "OTException.h"
#include "OTcmapTable.h"
#include "OTTags.h"
#include "OpenTypeFile.h"

using std::vector;
using std::lower_bound;
using util::String;

namespace OpenType {

/*** MappingTable ***/

MappingTable::~MappingTable() {}

/*** UnknownMappingTable ***/

class UnknownMappingTable : public MappingTable {
	UShort formatId;
	MemoryBlockPtr memory;
protected:
	virtual MemoryBlockPtr getMemory() const { return memory; }
public:
	UnknownMappingTable (OpenTypeFile & aFont, UShort aPlatformId, UShort aEncodingId, MemoryPen pen);
	virtual ~UnknownMappingTable();

	virtual bool isKnown() const { return false; }
	virtual UShort getFormatId() const { return formatId; }

	virtual bool mappingExists (ULong code) const { return false; }
	virtual UShort getGlyphId (ULong code) const { return 0; }
	virtual ULong getCode (UShort glyphId) const;

	virtual void clear();
	virtual void addMapping (ULong code, UShort glyphId);
	virtual void mergeFrom (MappingTablePtr table,
		bool warnDifferent = true, bool warnOutOfBounds = false);
};

UnknownMappingTable::UnknownMappingTable (OpenTypeFile & aFont, UShort aPlatformId,
											  UShort aEncodingId, MemoryPen pen)
: MappingTable (aFont, aPlatformId, aEncodingId) {
	MemoryPen startPen (pen);
	formatId = pen.readUShort();
	ULong length = pen.readUShort();
	if (!length)
		// Length is a 32-bit value
		pen.readULong();
	memory = startPen.readBlock (length);
}

UnknownMappingTable::~UnknownMappingTable() {}

ULong UnknownMappingTable::getCode (UShort glyphId) const {
	throw Exception ("Cannot get mapping of unknown 'cmap' subtable");
	return 0;
}

void UnknownMappingTable::clear () {
	throw Exception ("Cannot clear unknown 'cmap' subtable");
}

void UnknownMappingTable::addMapping (ULong code, UShort glyphId) {
	throw Exception ("Cannot add mapping for unknown 'cmap' subtable");
}

void UnknownMappingTable::mergeFrom (MappingTablePtr table,
									 bool warnDifferent, bool warnOutOfBounds)
{
	throw Exception ("Cannot add mapping for unknown 'cmap' subtable");
}

/*** KnownMappingTable ***/

typedef struct {
	ULong code;
	UShort glyphId;
} Mapping;

bool operator < (const Mapping & m1, const Mapping & m2) {
	return m1.code < m2.code;
}

typedef vector <Mapping> Mappings;

class KnownMappingTable : public MappingTable {
	Mappings::const_iterator getMapping (ULong code) const;
	/// Code range this type of table can handle
	ULong maxCode;
protected:
	Mappings mappings;
public:
	KnownMappingTable (OpenTypeFile & aFont, UShort aPlatformId, UShort aEncodingId, ULong aMaxCode)
		: MappingTable (aFont, aPlatformId, aEncodingId), maxCode (aMaxCode) {}
	virtual ~KnownMappingTable();

	virtual bool isKnown() const { return true; }

	virtual bool mappingExists (ULong code) const;
	virtual UShort getGlyphId (ULong code) const;
	virtual ULong getCode (UShort glyphId) const;

	virtual void clear();
	virtual void addMapping (ULong code, UShort glyphId);
	virtual void mergeFrom (MappingTablePtr table,
		bool warnDifferent = true, bool warnOutOfBounds = false);
};

KnownMappingTable::~KnownMappingTable() {}

Mappings::const_iterator KnownMappingTable::getMapping (ULong code) const {
	Mappings::const_iterator lowest = mappings.begin();
	Mappings::const_iterator highest = mappings.end();
	Mappings::const_iterator guess;
	while (lowest != highest) {
		guess = lowest + (highest - lowest) / 2;
		if (code < guess->code) 
			highest = guess;
		else {
			if (guess->code < code)
				lowest = guess + 1;
			else
				// guess->code == code; we've found it!
				return guess;
		}
	}
	return mappings.end();
}

bool KnownMappingTable::mappingExists (ULong code) const {
	Mappings::const_iterator m = getMapping (code);
	return m != mappings.end();
}

UShort KnownMappingTable::getGlyphId (ULong code) const {
	Mappings::const_iterator m = getMapping (code);
	if (m == mappings.end())
		return 0;
	else
		return m->glyphId;
}

ULong KnownMappingTable::getCode (UShort glyphId) const {
	Mappings::const_iterator i;
	for (i = mappings.begin(); i != mappings.end(); i ++) {
		if (i->glyphId == glyphId)
			return i->code;
	}
	return 0;
}

void KnownMappingTable::clear() {
	mappings.clear();
}

void KnownMappingTable::addMapping (ULong code, UShort glyphId) {
	Mapping m = {code, glyphId};
	Mappings::iterator i = lower_bound (mappings.begin(), mappings.end(), m);
	if (i->code == code)
		i->glyphId = glyphId;
	else
		mappings.insert (i, m);
}

void KnownMappingTable::mergeFrom (MappingTablePtr table,
								   bool warnDifferent, bool warnOutOfBounds)
{
	if (!table->isKnown())
		throw Exception ("Attempt to copy data from unknown 'cmap' subtable format");
	const Mappings & m = util::smart_ptr_cast <KnownMappingTable> (table)->mappings;
	Mappings::const_iterator m_i = m.begin();
	Mappings::iterator i = mappings.begin();
	while (m_i != m.end()) {
		if (i != mappings.end() && i->code < m_i->code)
			++ i;
		else {
			if (i == mappings.end()) {
				if (m_i->code <= maxCode) {
					mappings.push_back (*m_i);
					i = mappings.end();
				} else {
					if (warnOutOfBounds)
						font.addWarning (new Exception ("Code " + String (m_i->code) +
							" out of bounds"));
					// No more codes within bounds to come so stop copying now
					return;
				}
			} else {
				if (i->code > m_i->code) {
					i = mappings.insert (i, *m_i);
				} else {
					assert (i->code == m_i->code);
					if (warnDifferent && i->glyphId != m_i->glyphId)
						font.addWarning (new Exception ("Different glyph indices for code " +
							String (i->code)));
					// Do nothing anyway, whether or not the glyphId's are equal
					++ i;
				}
			}
			++ m_i;
		}
	}
}

/*** SegmentMappingToDeltaTable ***/

class SegmentMappingToDeltaTable : public KnownMappingTable {
protected:
	virtual MemoryBlockPtr getMemory() const;
public:
	SegmentMappingToDeltaTable (OpenTypeFile & aFont, UShort aPlatformId,
		UShort aEncodingId, MemoryPen pen);
	SegmentMappingToDeltaTable (OpenTypeFile & aFont, UShort aPlatformId,
		UShort aEncodingId)
		: KnownMappingTable (aFont, aPlatformId, aEncodingId, 0xFFFF) {}
	virtual ~SegmentMappingToDeltaTable();

	virtual UShort getFormatId() const { return 4; }
};

SegmentMappingToDeltaTable::~SegmentMappingToDeltaTable() {}

SegmentMappingToDeltaTable::SegmentMappingToDeltaTable (OpenTypeFile & aFont, UShort aPlatformId,
										  UShort aEncodingId, MemoryPen pen)
: KnownMappingTable (aFont, aPlatformId, aEncodingId, 0xFFFF)
{
	Exception::Context c ("Extracting segment to delta mapping table");
	UShort format = pen.readUShort();
	assert (format == 4);
	// length
	pen.readUShort();

	UShort language = pen.readUShort();
	if (language != 0)
		throw Exception ("Cannot handle subtable languages");
	UShort segmentNum = pen.readUShort() / 2;

	// Skip search ranges and such
	MemoryPen endCodes = pen + 6;
	MemoryPen startCodes = endCodes + 2 + 2 * segmentNum;
	MemoryPen deltas = startCodes + 2 * segmentNum;
	MemoryPen rangeOffsets = deltas + 2 * segmentNum;
	
	UShort i;
	for (i = 0; i < segmentNum; i ++) {
		ULong endCode, startCode, rangeOffset;
		Short delta;
		endCode = endCodes.readUShort();
		startCode = startCodes.readUShort();
		delta = deltas.readShort();
		rangeOffset = rangeOffsets.readUShort();
		if (!rangeOffset) {
			// valid delta value
			ULong code;
			for (code = startCode; code <= endCode; code ++) {
				Mapping m = {code, code + delta};
				mappings.push_back (m);
			}
		} else {
			// This is called an "obscure indexing trick" in the specs
			MemoryPen glyphIds = rangeOffsets + rangeOffset - 2;
			ULong code;
			for (code = startCode; code <= endCode; code ++) {
				Mapping m = {code, glyphIds.readUShort()};
				mappings.push_back (m);
			}
			if (endCode == 0xFFFF)
				font.addWarning(new Exception ("Incorrect number of segments"));
		}
	}

	if (mappings.back().code != 0xFFFF || mappings.back().glyphId != 0)
		throw Exception ("Last segment subtable segment should map 0xFFFF to 0");
	mappings.pop_back();
}

struct _Segment {
	Mappings::const_iterator begin, end;
	bool consecutive;
};



MemoryBlockPtr SegmentMappingToDeltaTable::getMemory() const {
	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);

/*	UShort segmentNum = 0;
	Mappings::const_iterator segStart, nextSegment;
	segStart = mappings.begin();
	while (segStart != mappings.end()) {
		nextSegment = segStart + 1;
		while (nextSegment != mappings.end() &&
			nextSegment->code == (nextSegment-1)->code + 1) {
			nextSegment ++;
		}

		segmentNum ++;

		segStart = nextSegment;
	}*/

	typedef vector <_Segment> _Segments;
	_Segments _segments;

	Mappings::const_iterator _first;
	_first = mappings.begin();
	while (_first != mappings.end()) {
		Mappings::const_iterator last;
		last = _first + 1;
		while (last != mappings.end() &&
			last->code == (last - 1)->code + 1)
		{
			++ last;
		}

		// One segment of consecutive codes found. However, it may be best
		// to separate it into different segments to save space: segments
		// with consecutive glyphIds can be encoded easier.
		// The rules are: an extra segment should be created when > 4
		// consecutive glyphIds are found at the beginning or end, or when
		// > 9 are found anywhere else.

		Mappings::const_iterator firstConsecutive = _first;
		while (_first != last) {
			Mappings::const_iterator lastConsecutive = firstConsecutive + 1;
			while (lastConsecutive != last &&
				lastConsecutive->glyphId == (lastConsecutive - 1)->glyphId + 1)
			{
				++ lastConsecutive;
			}
			// Range of consecutive glyphIds found that stops at lastConsecutive
			if (lastConsecutive == last) {
				// Last codes of segment are consecutive
				if (firstConsecutive == _first) {
					// The whole segment is consecutive
					_Segment s = {_first, last, true};
					_segments.push_back (s);
				} else {
					if (lastConsecutive - firstConsecutive > 4) {
						// Encode consecutive part separately
						_Segment s1 = {_first, firstConsecutive, false};
						_segments.push_back (s1);
						_Segment s2 = {firstConsecutive, last, true};
						_segments.push_back (s2);
					} else {
						// It makes no sense to split up the segment
						_Segment s = {_first, last, false};
						_segments.push_back (s);
					}
				}
				_first = last;
			} else {
				// Not the last part
				if (firstConsecutive == _first) {
					if (lastConsecutive - firstConsecutive > 4) {
						// Encode first, consecutive part, then proceed with the rest
						_Segment s = {firstConsecutive, lastConsecutive, true};
						_segments.push_back (s);
						firstConsecutive = _first = lastConsecutive;
					} else {
						// It makes no sense to encode this separately
						firstConsecutive = lastConsecutive;
					}
				} else {
					// Middle part
					if (lastConsecutive - firstConsecutive > 8) {
						// Encode first and middle part and then proceed with the rest
						_Segment s1 = {_first, firstConsecutive, false};
						_segments.push_back (s1);
						_Segment s2 = {firstConsecutive, lastConsecutive, true};
						_segments.push_back (s2);
						firstConsecutive = _first = lastConsecutive;
					} else {
						// It makes no sense to encode this separately
						firstConsecutive = lastConsecutive;
					}
				}
			}
		}

		_first = last;
	}

	// plus one sentinel segment
	UShort officialSegmentNum = _segments.size() + 1;

	// format
	pen.writeUShort (4);
	// length
	MemoryWritePen lengthPen = pen;
	pen += 2;
	// language
	pen.writeUShort (0);
	pen.writeUShort (officialSegmentNum * 2);
	UShort searchRange = 1;
	UShort entrySelector = 0;
	while (searchRange <= officialSegmentNum) {
		searchRange <<= 1;
		entrySelector ++;
	}
	entrySelector --;

	pen.writeUShort (searchRange);
	pen.writeUShort (entrySelector);
	pen.writeUShort (2 * officialSegmentNum - searchRange);

	MemoryWritePen endCodes = pen;
	MemoryWritePen startCodes = endCodes + 2*officialSegmentNum;
	// reservedPad
	startCodes.writeUShort (0);
	MemoryWritePen deltas = startCodes + 2*officialSegmentNum;
	MemoryWritePen rangeOffsets = deltas + 2*officialSegmentNum;
	MemoryWritePen glyphIdRange = rangeOffsets + 2*officialSegmentNum;

	_Segments::const_iterator s = _segments.begin();
	for (; s != _segments.end(); s ++) {
		endCodes.writeUShort ((s->end - 1)->code);
		startCodes.writeUShort (s->begin->code);
		if (s->consecutive) {
			deltas.writeUShort (s->begin->glyphId - s->begin->code);
			rangeOffsets.writeUShort (0);
		} else {
			deltas.writeUShort (0);
			rangeOffsets.writeUShort (glyphIdRange - rangeOffsets);
			Mappings::const_iterator i;
			for (i = s->begin; i != s->end; i ++)
				glyphIdRange.writeUShort (i->glyphId);
		}
	}

	// Add sentinel entry
	endCodes.writeUShort (0xFFFF);
	startCodes.writeUShort (0xFFFF);
	deltas.writeUShort (1);
	rangeOffsets.writeUShort (0);

	// write length
	lengthPen.writeUShort (memory->getSize());

	return memory;

/*	// sentinel segment
	segmentNum ++;

	// format
	pen.writeUShort (4);
	// length
	MemoryWritePen lengthPen = pen;
	pen += 2;
	// language
	pen.writeUShort (0);
	pen.writeUShort (segmentNum * 2);
	UShort searchRange = 1;
	UShort entrySelector = 0;
	while (searchRange <= segmentNum) {
		searchRange <<= 1;
		entrySelector ++;
	}
	entrySelector --;

	pen.writeUShort (searchRange);
	pen.writeUShort (entrySelector);
	pen.writeUShort (2 * segmentNum - searchRange);

	MemoryWritePen endCodes = pen;
	MemoryWritePen startCodes = endCodes + 2*segmentNum;
	// reservedPad
	startCodes.writeUShort (0);
	MemoryWritePen deltas = startCodes + 2*segmentNum;
	MemoryWritePen rangeOffsets = deltas + 2*segmentNum;
	MemoryWritePen glyphIdRange = rangeOffsets + 2*segmentNum;
	
	bool consecutive;
	segStart = mappings.begin();
	while (segStart != mappings.end()) {
		nextSegment = segStart + 1;
		consecutive = true;
		while (nextSegment != mappings.end() &&
			nextSegment->code == (nextSegment-1)->code + 1) {
			if (nextSegment->glyphId != (nextSegment-1)->glyphId + 1)
				consecutive = false;
			nextSegment ++;
		}

		endCodes.writeUShort ((nextSegment - 1)->code);
		startCodes.writeUShort (segStart->code);
		if (consecutive) {
			deltas.writeUShort (segStart->glyphId - segStart->code);
			rangeOffsets.writeUShort (0);
		} else {
			deltas.writeUShort (0);
			rangeOffsets.writeUShort (glyphIdRange - rangeOffsets);
			Mappings::const_iterator i;
			for (i = segStart; i != nextSegment; i ++)
				glyphIdRange.writeUShort (i->glyphId);
		}

		segStart = nextSegment;
	}

	// Add sentinel entry
	endCodes.writeUShort (0xFFFF);
	startCodes.writeUShort (0xFFFF);
	deltas.writeUShort (1);
	rangeOffsets.writeUShort (0);

	// write length
	lengthPen.writeUShort (memory->getSize());

	return memory;*/
}

/*** SegmentMappingToDeltaTable ***/

class SegmentMappingTable : public KnownMappingTable {
protected:
	virtual MemoryBlockPtr getMemory() const;
public:
	SegmentMappingTable (OpenTypeFile & aFont, UShort aPlatformId,
		UShort aEncodingId, MemoryPen pen);
	SegmentMappingTable (OpenTypeFile & aFont, UShort aPlatformId,
		UShort aEncodingId)
		: KnownMappingTable (aFont, aPlatformId, aEncodingId, 0xFFFFFFFF) {}
	virtual ~SegmentMappingTable();

	virtual UShort getFormatId() const { return 12; }
};

SegmentMappingTable::~SegmentMappingTable() {}

SegmentMappingTable::SegmentMappingTable (OpenTypeFile & aFont, UShort aPlatformId,
										  UShort aEncodingId, MemoryPen pen)
: KnownMappingTable (aFont, aPlatformId, aEncodingId, 0xFFFFFFFF)
{
	Exception::Context c ("Extracting segment coverage mapping table");
	UShort dummy = pen.readUShort();
	assert (dummy == 12);

	// reserved = 0
	dummy = pen.readUShort();
	if (dummy != 0)
		throw Exception ("'cmap' subtable format 12 should have a zero length entry");

	// length
	pen.readULong();

	ULong language = pen.readULong();
	if (language != 0)
		throw Exception ("Cannot handle subtable languages");
	ULong groupNum = pen.readULong();

	ULong startCode, endCode, startGlyphId, lastEndCode;

	ULong i;
	for (i = 0; i < groupNum; i ++) {
		startCode = pen.readULong();
		endCode = pen.readULong();
		startGlyphId = pen.readULong();

		if (i) {
			if (startCode <= lastEndCode)
				throw Exception ("Overlapping cmap subtable format 12 groups");
		}

		ULong code, glyphId;
		glyphId = startGlyphId;
		for (code = startCode; code <= endCode; code ++, glyphId ++) {
			if (glyphId > 0xFFFF)
				throw Exception ("Glyph index " + String (glyphId) + " too high");
			Mapping m = {code, glyphId};
			mappings.push_back (m);
		}
		
		lastEndCode = endCode;
	}
}

MemoryBlockPtr SegmentMappingTable::getMemory() const {
	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);
	// format
	pen.writeUShort (12);
	// reserved
	pen.writeUShort (0);

	MemoryWritePen lengthPen (pen);
	// length
	pen.writeULong (0);
	// Language
	pen.writeULong (0);

	MemoryWritePen groupNumPen (pen);
	// groupsNum dummy
	pen.writeULong (0);

	ULong groupNum = 0;
	Mappings::const_iterator start, current;
	start = mappings.begin();
	while (start != mappings.end()) {
		current = start;
		while (true) {
			current ++;
			if (current == mappings.end())
				break;
			if (current->code != (current - 1)->code + 1)
				break;
			if (current->glyphId != (current - 1)->glyphId + 1)
				break;
		}

		pen.writeULong (start->code);
		pen.writeULong ((current - 1)->code);
		pen.writeULong (start->glyphId);
		groupNum ++;

		start = current;
	}

	groupNumPen.writeULong (groupNum);

	lengthPen.writeULong (memory->getSize());

	return memory;
}

/*** cmapTable ***/

cmapTable::cmapTable (OpenTypeFile &aFont, MemoryBlockPtr memory)
: Table (aFont) {
	MemoryPen startPen (memory);
	MemoryPen pen (memory);

	UShort version = pen.readUShort();
	if (version != 0)
		throw Exception ("Unknown table version: " + String (version, 16));
	UShort mappingNum = pen.readUShort();
	UShort i;
	for (i = 0; i < mappingNum; i ++) {
		UShort platformId = pen.readUShort();
		UShort encodingId = pen.readUShort();
		// read offset
		MemoryPen subTablePen (startPen + pen.readULong());
		UShort formatId = MemoryPen (subTablePen).readUShort();

		switch (formatId) {
		case 4:
			mappingTables.push_back (new SegmentMappingToDeltaTable (font,
				platformId, encodingId, subTablePen));
			break;
		case 12:
			mappingTables.push_back (new SegmentMappingTable (font,
				platformId, encodingId, subTablePen));
			break;
		default:
			mappingTables.push_back (new UnknownMappingTable (font,
				platformId, encodingId, subTablePen));
		}
	}
}

cmapTable::~cmapTable() {}

Tag cmapTable::getTag() const {
	return cmapTag;
}

MemoryBlockPtr cmapTable::getMemory() const {
	MemoryBlockPtr memory = new MemoryBlock;
	MemoryPen memStart (memory);
	MemoryWritePen pen (memory);

	// version
	pen.writeUShort (0);
	pen.writeUShort (mappingTables.size());
	MemoryWritePen headerPen = pen;

	pen += 8 * mappingTables.size();

	MappingTables::const_iterator i;
	for (i = mappingTables.begin(); i != mappingTables.end(); i ++) {
		headerPen.writeUShort ((*i)->getPlatformId());
		headerPen.writeUShort ((*i)->getEncodingId());
		// Offset
		headerPen.writeULong (pen - memStart);

		pen.writeBlock ((*i)->getMemory());
		pen.padWithZeros (4);
	}

	return memory;
}

/*const MappingTables & cmapTable::getMappingTables() const {
	return mappingTables;
}*/

MappingTablePtr cmapTable::getFirstKnownMappingTable (UShort platformId, UShort encodingId) {
	MappingTables::const_iterator i;
	for (i = mappingTables.begin(); i != mappingTables.end(); i ++) {
		if ((*i)->getPlatformId() == platformId && (*i)->getEncodingId() == encodingId && (*i)->isKnown())
			return *i;
	}
	return NULL;
}

MappingTablePtr cmapTable::getFirstKnownMappingTable (UShort platformId, UShort encodingId, UShort formatId) {
	MappingTables::const_iterator i;
	for (i = mappingTables.begin(); i != mappingTables.end(); i ++) {
		if ((*i)->getPlatformId() == platformId && (*i)->getEncodingId() == encodingId &&
			(*i)->getFormatId() == formatId && (*i)->isKnown())
			return *i;
	}
	return NULL;
}

MappingTablePtr cmapTable::newMappingTable (UShort platformId, UShort encodingId, UShort formatId) {
	switch (formatId) {
	case 4:
		return new SegmentMappingToDeltaTable (font, platformId, encodingId);
	case 12:
		return new SegmentMappingTable (font, platformId, encodingId);
	default:
		throw Exception ("Unable to generate 'cmap' subtable format " + 
			String (formatId));
		return NULL;
	}
}

} // end namespace OpenType
