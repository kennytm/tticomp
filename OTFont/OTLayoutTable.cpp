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
	\file General OpenType Layout tables
*/


#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <vector>
#include <algorithm>
#include "../Util/erase_duplicates.h"

#include "OTException.h"
#include "OpenTypeFont.h"

#include "OTLayoutTable.h"
#include "OTLayoutInternal.h"
#include "OpenTypeText.h"
#include "OTTags.h"

using std::sort;
using util::String;
using util::smart_ptr;
using util::erase_duplicates;

namespace OpenType {

/*** ScriptList ***/

FeatureIndices noFeatures = FeatureIndices (Indices(), 0xFFFF);

ScriptList::ScriptList (MemoryPen pen) {
	const MemoryPen start = pen;
	UShort scriptNum = pen.readUShort();
	for (UShort i = 0; i < scriptNum; i++) {
		Tag tag = pen.readTag();
		scripts.push_back (new Script (tag, start + pen.readOffset()));
	}
}

ScriptList::~ScriptList() {}

Tags ScriptList::getScriptTags() const {
	Tags tags;
	Scripts::const_iterator s;
	for (s = scripts.begin(); s != scripts.end(); s ++)
		tags.push_back ((*s)->getTag());
	return tags;
}

Tags ScriptList::getLanguageTags (Tag script) const {
	Scripts::const_iterator s;
	for (s = scripts.begin(); s != scripts.end(); s ++) {
		if ((*s)->getTag() == script)
			return (*s)->getLanguageTags();
	}
	return Tags();
}

const FeatureIndices & ScriptList::getFeatureIndices (Tag script, Tag language) const
{
	Scripts::const_iterator s;
	for (s = scripts.begin(); s != scripts.end(); s ++) {
		if ((*s)->getTag() == script) {
			return (*s)->getFeatureIndices (language);
		}
	}
	return noFeatures;
}

/*** Script ***/

Script::Script (Tag aTag, MemoryPen pen) : tag (aTag) {
	const MemoryPen start = pen;

	Offset defLangOffset = pen.readOffset();
	if (defLangOffset) {
		defaultLanguage = new LanguageSystem (DFLTTag, start + defLangOffset);
		languages.push_back (defaultLanguage);
	}

	UShort langNum = pen.readUShort();
	for (UShort i = 0; i < langNum; i ++) {
		Tag tag = pen.readTag();
		languages.push_back (new LanguageSystem (tag, start + pen.readOffset()));
	}
}

Script::~Script() {}

Tag Script::getTag() const {
	return tag;
}

Tags Script::getLanguageTags() const {
	Tags tags;
	Languages::const_iterator l;
	for (l = languages.begin(); l != languages.end(); l ++)
		tags.push_back ((*l)->getTag());
	return tags;
}

const FeatureIndices & Script::getFeatureIndices (Tag language) const {
	Languages::const_iterator l;
	if (language == DFLTTag)
		return defaultLanguage->getFeatures();

	for (l = languages.begin(); l != languages.end(); l ++) {
		if ((*l)->getTag() == language)
			return (*l)->getFeatures();
	}

	// The language was not found, so get default language tags
	if (defaultLanguage)
		return defaultLanguage->getFeatures();
	else
		return noFeatures;
}

/*** LanguageSystem ***/

LanguageSystem::LanguageSystem (Tag aTag, MemoryPen pen)
: tag (aTag) {
	// lookupOrder
	pen.readOffset();
	features.second = pen.readUShort();
	UShort featureNum = pen.readUShort();
	for (UShort i = 0; i < featureNum; i ++)
		features.first.push_back (pen.readUShort());
}

LanguageSystem::~LanguageSystem() {}

Tag LanguageSystem::getTag() const {
	return tag;
}

const FeatureIndices & LanguageSystem::getFeatures() const {
	return features;
}

/*** FeatureList ***/

FeatureList::FeatureList (MemoryPen pen) {
	const MemoryPen start = pen;
	UShort featureNum = pen.readUShort();
	for (UShort i = 0; i < featureNum; i++) {
		Tag tag = pen.readTag();
		features.push_back (new Feature (tag, start + pen.readOffset()));
	}
}

FeatureList::~FeatureList() {}

Tag FeatureList::getFeatureTag (UShort index) const {
	if (index >= features.size())
		throw Exception ("Feature index " + String (index) +
			" out of bounds");
	return features [index]->getTag();
}

void FeatureList::getLookups (BoolVector &lookups, UShort index) const {
	if (index >= features.size())
		throw Exception ("Feature index " + String (index) +
			" out of bounds");
	features [index]->getLookups (lookups);
}

/*** Feature ***/

Feature::Feature (Tag aTag, MemoryPen pen) : tag (aTag) {
	// featureParams
	pen.readOffset();
	UShort lookupNum = pen.readUShort();
	for (UShort i = 0; i < lookupNum; i ++)
		lookups.push_back (pen.readUShort());
}

Feature::~Feature() {}

Tag Feature::getTag() const {
	return tag;
}

void Feature::getLookups (BoolVector &lookupsUsed) const {
	Indices::const_iterator i;
	// Set every lookup this feature uses to true
	for (i = lookups.begin(); i != lookups.end(); i ++)
		lookupsUsed [*i] = true;
}

/*** LookupList ***/

LookupList::LookupList (MemoryPen pen, OpenTypeFont &aFont,
						smart_ptr <LookupSubTable> newSubTable (UShort, UShort, MemoryPen,
						const LookupList &, OpenTypeFont &)) {
	const MemoryPen start = pen;
	UShort lookupNum = pen.readUShort();
	for (UShort i = 0; i < lookupNum; i ++)
		lookups.push_back (new Lookup (start + pen.readOffset(), aFont,
			*this, newSubTable));
}

LookupList::~LookupList() {}

void LookupList::applyLookups (OpenTypeText &text, const BoolVector &lookupSet) const {
	assert (lookupSet.size() == lookups.size());
	BoolVector::const_iterator b = lookupSet.begin();
	Lookups::const_iterator l = lookups.begin();

	// Apply all lookups that are specified in lookupSet
	while (l != lookups.end()) {
		if (*b)
			(*l)->apply (text);
		b++;
		l++;
	}
}

void LookupList::applyLookups (OpenTypeText::iterator begin, OpenTypeText::iterator current,
							   OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end,
							   UShort index) const
{
	if (index >= lookups.size())
		throw Exception ("Lookup index " + String (index) + " out of bounds.");
	lookups [index]->apply (begin, current, scopeEnd, end);
}

/*** Lookup ***/

Lookup::Lookup (MemoryPen pen, OpenTypeFont &aFont, const LookupList &lookupList,
				SubTablePtr newSubTable (UShort, UShort, MemoryPen,
				const LookupList &, OpenTypeFont &font))
: font (aFont) {
	MemoryPen start = pen;
	// type
	type = pen.readUShort();
	flags = pen.readUShort();
	UShort subTableNum = pen.readUShort();
	for (UShort i = 0; i < subTableNum; i ++) {
		MemoryPen subTablePen = start + pen.readOffset();
		UShort subTableFormat = MemoryPen (subTablePen).readUShort();
		try {
			SubTablePtr subTable = 	newSubTable (type, subTableFormat, subTablePen,
				lookupList, font);
			if (!subTable)
				throw Exception (String ("Unknown lookup type ") +
					String (type) +	" subtable format " + String (subTableFormat));
			else
				subTables.push_back (subTable);
		} catch (Exception &e) {
			e.addDescription ("This lookup " + String (type) + " subtable format " 
				+ String (subTableFormat) + " will be ignored");
			font.addWarning (new Exception (e));
		}
	}
}

Lookup::~Lookup() {}

void Lookup::apply (OpenTypeText &text) const {
	SubTables::const_iterator table;
	OpenTypeText::iterator begin = text.begin (flags);
	OpenTypeText::iterator end  = text.end (flags);
	for (table = subTables.begin(); table != subTables.end(); table ++) {
		assert (*table);
		for (OpenTypeText::iterator current = begin; current != end;)
			// Use the full scope: endScope = end
			(*table)->apply (begin, current, end, end);
	}
}

void Lookup::apply (OpenTypeText::iterator begin, OpenTypeText::iterator current,
					OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	SubTables::const_iterator table;
	for (table = subTables.begin(); table != subTables.end(); table ++) {
		assert (*table);
		OpenTypeText::iterator finalPosition = current;
		(*table)->apply (begin, finalPosition, scopeEnd, end);
	}
}

/*** LookupReference ***/

LookupReference::LookupReference (MemoryPen & pen, const LookupList &_lookupList)
: lookupList (_lookupList) {
	sequenceIndex = pen.readUShort();
	lookupListIndex = pen.readUShort();
}

void LookupReference::apply (OpenTypeText::iterator begin, OpenTypeText::iterator current,
							 OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	UShort forward = sequenceIndex;
	while (forward && current != scopeEnd) {
		current ++;
		forward --;
	}
	if (current != scopeEnd)
		// Apply lookup
		lookupList.applyLookups (begin, current, scopeEnd, end, lookupListIndex);
	else
		throw Exception ("Cannot apply lookup at position " + String (sequenceIndex) +
			" because the glyph string is not long enough");
}

/*** ContextLookup3 ***/

ContextLookup3::ContextLookup3 (MemoryPen pen, const LookupList &lookupList, OpenTypeFont &font) {
	MemoryPen start = pen;
	// format id = 3
	pen.readUShort();
	UShort glyphNum = pen.readUShort();
	UShort substNum = pen.readUShort();
	UShort i;
	for (i = 0; i < glyphNum; i ++)
		coverage.push_back (getCoverageTable (start + pen.readOffset(), font));
	for (i = 0; i < substNum; i ++)
		lookupReferences.push_back (new LookupReference (pen, lookupList));
}

ContextLookup3::~ContextLookup3() {}

void ContextLookup3::apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
							OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	OpenTypeText::iterator endOfInput = current;
	if (checkInput (coverage, endOfInput, scopeEnd)) {
		// Found the input sequence
		applyReferences (begin, current, endOfInput, end, lookupReferences);
		current = endOfInput;
	} else
		current ++;
}

/*** ChainingContextLookup3 ***/

ChainingContextLookup3::ChainingContextLookup3 (MemoryPen pen, const LookupList &lookupList,
												OpenTypeFont &font) {
	MemoryPen start = pen;
	// format = 3
	pen.readUShort();
	UShort num;
	// Read backtrack sequence
	for (num = pen.readUShort(); num; num --)
		backtrackCoverage.push_back (getCoverageTable (start + pen.readOffset(), font));
	// Read input sequence
	for (num = pen.readUShort(); num; num --)
		inputCoverage.push_back (getCoverageTable (start + pen.readOffset(), font));
	// Read lookahead sequence
	for (num = pen.readUShort(); num; num --)
		lookAheadCoverage.push_back (getCoverageTable (start + pen.readOffset(), font));
	// Read lookupRecords
	for (num = pen.readUShort(); num; num --)
		lookupReferences.push_back (new LookupReference (pen, lookupList));
}

ChainingContextLookup3::~ChainingContextLookup3() {}

void ChainingContextLookup3::apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
									OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const
{
	OpenTypeText::iterator endOfInput = current;
	// Check input sequence
	if (checkInput (inputCoverage, endOfInput, scopeEnd))
	{
		OpenTypeText::iterator checkPosition = current;
		// Check backtrack sequence
		if (checkReverseInput (backtrackCoverage, checkPosition, begin)) {
			checkPosition = endOfInput;
			// Check lookahead sequence
			if (checkInput (lookAheadCoverage, checkPosition, end)) {
				// The context is right!
				applyReferences (begin, current, endOfInput, end, lookupReferences);
				current = endOfInput;
				return;
			}
		}
	}
	// If the context was not right
	current ++;
}

/*** CoverageTable ***/

class CoverageSingleTable : public CoverageTable {
	GlyphIds glyphIds;
public:
	CoverageSingleTable (MemoryPen pen, OpenTypeFont &font);
	virtual ~CoverageSingleTable();

	virtual bool isCovered (UShort glyphId, UShort *index) const;
	virtual UShort getGlyphNum() const;
	virtual GlyphId getMaxGlyphId() const;
};

class CoverageRangeTable : public CoverageTable {
	typedef struct {
		UShort start;
		UShort end;
		UShort startIndex;
	} Range;

	typedef std::vector <Range> Ranges;
	Ranges ranges;
public:
	CoverageRangeTable (MemoryPen pen, OpenTypeFont &font);
	virtual ~CoverageRangeTable();

	virtual bool isCovered (UShort glyphId, UShort *index) const;
	virtual UShort getGlyphNum() const;
	virtual GlyphId getMaxGlyphId() const;
};

CoveragePtr getCoverageTable (MemoryPen pen, OpenTypeFont &font) {
	MemoryPen start = pen;
	UShort coverageVersion = pen.readUShort();
	switch (coverageVersion) {
	case 1:
		// individual glyphs
		return new CoverageSingleTable (start, font);
	case 2:
		return new CoverageRangeTable (start, font);
	default:
		throw Exception ("Unknown coverage table version " +
			String (coverageVersion));
	}
}

CoverageSingleTable::CoverageSingleTable (MemoryPen pen, OpenTypeFont &font) {
	// format id = 1
	pen.readUShort();
	UShort glyphCount = pen.readUShort();
	glyphIds.reserve (glyphCount);
	UShort lastOne;
	for (UShort i = 0; i < glyphCount; i ++) {
		UShort newOne = pen.readUShort();
		if (i != 0 && newOne <= lastOne) {
			// Palatino seems to fail this test?????
			throw Exception ("Coverage table glyphs are not in increasing order: " +
				String (lastOne) + " comes before " + String (newOne));
		}
		glyphIds.push_back (newOne);
		lastOne = newOne;
	}

	if (lastOne > font.getGlyphNum())
		throw Exception ("Coverage table glyphs are out of range: glyph " +
			String (lastOne) + " does not exist");
}

CoverageSingleTable::~CoverageSingleTable() {}

UShort CoverageSingleTable::getGlyphNum() const {
	return glyphIds.size();
}

bool CoverageSingleTable::isCovered (UShort glyphId, UShort *index) const {
	GlyphIds::const_iterator lowest, highest, guess;
	lowest = glyphIds.begin();
	highest = glyphIds.end();
	while (lowest != highest) {
		guess = lowest + (highest - lowest) / 2;
		if (glyphId < *guess)
			highest = guess;
		else {
			if (*guess < glyphId)
				lowest = guess + 1;
			else {
				// Glyph id is at guess
				if (index)
					*index = guess - glyphIds.begin();
				return true;
			}
		}
	}
	return false;
}

GlyphId CoverageSingleTable::getMaxGlyphId() const {
	return glyphIds.back();
}

CoverageRangeTable::CoverageRangeTable (MemoryPen pen, OpenTypeFont &font) {
	// format id = 2
	pen.readUShort();
	UShort rangeNum = pen.readUShort();
	ranges.reserve (rangeNum);
	UShort lastOne;
	for (UShort i = 0; i < rangeNum; i ++) {
		Range r;
		r.start = pen.readUShort();
		if (i > 0 && r.start <= lastOne)
			throw Exception ("Coverage table ranges should be in increasing order");
		r.end = pen.readUShort();
		r.startIndex = pen.readUShort();
		ranges.push_back (r);
		lastOne = r.end;
	}

	if (lastOne > font.getGlyphNum())
		throw Exception ("Coverage table glyphs are out of range: glyph " +
			String (lastOne) + " does not exist");
}

CoverageRangeTable::~CoverageRangeTable() {}

UShort CoverageRangeTable::getGlyphNum() const {
	UShort glyphNum = 0;
	Ranges::const_iterator r;
	for (r = ranges.begin(); r != ranges.end(); r ++)
		glyphNum += r->end - r->start + 1;
	return glyphNum;
}

bool CoverageRangeTable::isCovered (UShort glyphId, UShort *index) const {
	Ranges::const_iterator lower, upper, guess;
	lower = ranges.begin();
	upper = ranges.end();
	while (lower != upper) {
		guess = lower + (upper - lower)/2;
		if (glyphId < guess->start)
			upper = guess;
		else {
			if (guess->end < glyphId)
				lower = guess + 1;
			else {
				// Glyph is in range
				if (index)
					*index = guess->startIndex + glyphId - guess->start;
				return true;
			}
		}
	}
	return false;
}

GlyphId CoverageRangeTable::getMaxGlyphId() const {
	return ranges.back().end;
}

/*** ClassDefTable ***/

ClassDefPtr getClassDefTable (MemoryPen pen) {
	UShort format = MemoryPen (pen).readUShort();
	switch (format) {
	case 1:
		// Format 1: class array
		return new ClassDefTable1 (pen);
	case 2:
		// Format 2: class ranges
		return new ClassDefTable2 (pen);
	default:
		throw Exception ("Unknown class definition table format " +
			String (format));
	}
}

ClassDefTable1::ClassDefTable1 (MemoryPen pen) {
	// format = 1
	pen.readUShort();
	start = pen.readGlyphId();
	UShort glyphNum = pen.readUShort();
	classes.reserve (glyphNum);
	for (UShort i = 0; i < glyphNum; i ++)
		classes.push_back (pen.readUShort());
}

ClassDefTable1::~ClassDefTable1() {}

UShort ClassDefTable1::getClass (GlyphId glyphId) const {
	if (glyphId < start || glyphId >= start + classes.size())
		return 0;
	return classes [glyphId - start];
}

UShort ClassDefTable1::getClassNum() const {
	UShort maxClass = 0;
	Classes::const_iterator c;
	for (c = classes.begin(); c != classes.end(); c ++) {
		if (maxClass < *c)
			maxClass = *c;
	}
	return maxClass + 1;
}

ClassDefTable2::ClassDefTable2 (MemoryPen pen) {
	// format = 2
	pen.readUShort();
	UShort rangeNum = pen.readUShort();
	ranges.reserve (rangeNum);
	GlyphId lastEnd;
	for (UShort i = 0; i < rangeNum; i ++) {
		ClassRange r;
		r.start = pen.readGlyphId();
		if (i != 0 && lastEnd >= r.start)
			throw Exception ("Class definition ranges overlap");
		r.end = pen.readGlyphId();
		if (r.start > r.end)
			throw Exception ("Class definition range structure error");
		r.glyphClass = pen.readUShort();
		lastEnd = r.end;
		ranges.push_back (r);
	}
}

ClassDefTable2::~ClassDefTable2() {}

UShort ClassDefTable2::getClass (GlyphId glyph) const {
	ClassRanges::const_iterator lower, upper, guess;
	lower = ranges.begin();
	upper = ranges.end();
	while (lower != upper) {
		guess = lower + (upper - lower)/2;
		if (glyph < guess->start)
			upper = guess;
		else {
			if (guess->end < glyph)
				lower = guess + 1;
			else
				return guess->glyphClass;
		}
	}
	return 0;
}

UShort ClassDefTable2::getClassNum() const {
	UShort maxClass = 0;
	ClassRanges::const_iterator r;
	for (r = ranges.begin(); r != ranges.end(); r ++) {
		if (maxClass < r->glyphClass)
			maxClass = r->glyphClass;
	}
	return maxClass + 1;
}


/*** LayoutTable ***/

smart_ptr <LookupSubTable> newDefaultLookupSubTable (UShort type, MemoryPen pen) {
	return smart_ptr <LookupSubTable> ();
}

LayoutTable::LayoutTable (OpenTypeFont &aFont, MemoryBlockPtr aMemory,
							  smart_ptr <LookupSubTable> newSubTable (UShort, UShort, MemoryPen,
							  const LookupList&, OpenTypeFont &font))
: Table (aFont), memory (aMemory) {
	const MemoryPen start (memory);
	MemoryPen pen = start;
	Fixed version = pen.readFixed();
	if (version != 0x00010000)
		throw Exception ("Unknown layout table version " +
			String (version, 16));
	scriptList = new ScriptList (start + pen.readOffset());
	featureList = new FeatureList (start + pen.readOffset());
	lookupList = new LookupList (start + pen.readOffset(), (OpenTypeFont &)font, newSubTable);
}

LayoutTable::~LayoutTable() {}

MemoryBlockPtr LayoutTable::getMemory() const {
	return memory;
}

Tags LayoutTable::getScripts() const {
	Tags tags = scriptList->getScriptTags();
	sort (tags.begin(), tags.end(), compareTags);
	erase_duplicates (tags);
	return tags;
}

Tags LayoutTable::getLanguages (Tag script) const {
	Tags tags = scriptList->getLanguageTags (script);
	sort (tags.begin(), tags.end(), compareTags);
	erase_duplicates (tags);
	return tags;
}

Tags LayoutTable::getFeatures (Tag script, Tag language) const {
	Exception::FontContext c1 (font);
	Exception::Context c2 ("enumerating OT features");
	Indices featureIndices = scriptList->getFeatureIndices (script, language).first;
	Indices::iterator i;
	Tags tags;
	for (i = featureIndices.begin(); i != featureIndices.end(); i ++)
		tags.push_back (featureList->getFeatureTag (*i));

	sort (tags.begin(), tags.end(), compareTags);
	erase_duplicates (tags);
	return tags;
}

void LayoutTable::apply (OpenTypeText &text, Tag script, Tag language,
						   Tags features) const
{
	Exception::FontContext c1 (font);
	Exception::Context c2 ("applying OT lookups");

	BoolVector lookupsUsed (lookupList->getNum(), false);

	FeatureIndices featureIndices = scriptList->getFeatureIndices (script, language);
	Indices::iterator i;
	Tags tags;
	if (featureIndices.second != 0xFFFF)
		// Required feature
		featureList->getLookups (lookupsUsed, featureIndices.second);

	for (i = featureIndices.first.begin(); i != featureIndices.first.end(); i ++) {
		Tag featureTag = featureList->getFeatureTag (*i);
		// See if this feature should be applied
		Tags::iterator f = std::find (features.begin(), features.end(), featureTag);
		if (f != features.end()) {
			// Yes, the feature should be applied
			featureList->getLookups (lookupsUsed, *i);
		}
	}
	
	// Apply lookups
	lookupList->applyLookups (text, lookupsUsed);
}

} // end namespace OpenType
