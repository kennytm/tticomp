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
#include "OThmtxTable.h"
#include "OThheaTable.h"
#include "OTmaxpTable.h"
#include "OTTags.h"
#include "OTGlyph.h"

using util::smart_ptr;

namespace OpenType {

hmtxTable::hmtxTable (OpenTypeFile &aFont, smart_ptr <hheaTable> hheaTable, const Glyphs &glyphs)
 : Table (aFont) {
	assert (glyphs.size() != 0);
	Glyphs::const_iterator i;
	for (i = glyphs.begin(); i != glyphs.end(); i ++)
		horMetrics.push_back ((*i)->getHorMetric());

	// Set maxp metrics num
	hheaTable->setHMetricsNum (getLastFullMetric() + 1 - horMetrics.begin());
}

hmtxTable::hmtxTable (OpenTypeFile &aFont, smart_ptr <maxpTable> maxpTable,
						  smart_ptr <hheaTable> hheaTable, MemoryBlockPtr memory)
: Table (aFont) {
	MemoryPen pen (memory);
	UShort fullMetricsNum = hheaTable->getHMetricsNum();
	if (fullMetricsNum == 0)
		throw Exception ("Invalid number of full horizontal metrics: 0");

	UShort glyphNum = maxpTable->getGlyphNum();
	if (fullMetricsNum > glyphNum)
		throw Exception ("More horizontal metrics than glyphs");

	horMetrics.reserve (glyphNum);
	UShort lastAdvance;
	UShort i;
	for (i = 0; i < fullMetricsNum; i ++) {
		HorMetric hm;
		lastAdvance = hm.advanceWidth = pen.readUShort();
		hm.lsb = pen.readShort();
		horMetrics.push_back (hm);
	}

	for (; i < glyphNum; i ++) {
		HorMetric hm;
		hm.advanceWidth = lastAdvance;
		hm.lsb = pen.readShort();
		horMetrics.push_back (hm);
	}

	if (!pen.endOfBlock())
		font.addWarning (new Exception ("Table extends beyond data"));
}

hmtxTable::~hmtxTable() {}

HorMetrics::const_iterator hmtxTable::getLastFullMetric() const {
	HorMetrics::const_iterator i = horMetrics.end() - 1;
	while (i != horMetrics.begin() && (i-1)->advanceWidth == i->advanceWidth)
		i --;
	return i;
}

Tag hmtxTable::getTag() const {
	return hmtxTag;
}

MemoryBlockPtr hmtxTable::getMemory() const {
	assert (!horMetrics.empty());
	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);

	HorMetrics::const_iterator i;
	HorMetrics::const_iterator lastFullMetric = getLastFullMetric();

	for (i = horMetrics.begin(); i <= lastFullMetric; i ++) {
		pen.writeUShort (i->advanceWidth);
		pen.writeShort (i->lsb);
	}

	for (; i != horMetrics.end(); i++)
		pen.writeShort (i->lsb);

	return memory;
}

HorMetric hmtxTable::getHorMetric (UShort glyphIndex) {
	if (glyphIndex < horMetrics.size())
		return horMetrics [glyphIndex];
	else
		return horMetrics [horMetrics.size() - 1];
}

} // end namespace OpenType
