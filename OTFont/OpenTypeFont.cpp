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

#include "OpenTypeFont.h"
#include "OTException.h"

#include <algorithm>
using std::sort;
using std::set_union;
using std::lower_bound;
using std::upper_bound;
using std::vector;
using std::pair;
using util::shared_vector;

#include "OTgaspTable.h"
#include "OThmtxTable.h"
#include "OTheadTable.h"
#include "OThheaTable.h"
#include "OTmaxpTable.h"
#include "OTlocaTable.h"
#include "OTpostTable.h"
#include "OTGlyph.h"
#include "OTcmapTable.h"
#include "OTOS2Table.h"
#include "OTLayoutTable.h"
#include "OTTags.h"

using util::String;
using util::smart_ptr;

namespace OpenType {

OpenTypeFont::OpenTypeFont() : glyphsExtracted (false), namesExtracted (false) {}

OpenTypeFont::~OpenTypeFont() {}

void OpenTypeFont::readFromFile (const String aFileName) {
	OpenTypeFile::readFromFile (aFileName);
	// Make sure 'head' table is available so that checksum can be set
	// When writing
	getheadTable();
}

smart_ptr <headTable> OpenTypeFont::getheadTable (bool fail) {
	if (!head) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("extracting 'head' table");
		TablePtr oldhead = getTable (headTag, fail);
		if (!oldhead)
			return NULL;
		head = new headTable (*this, oldhead->getMemory());
		replaceTable (head);
	}
	return head;
}

smart_ptr <hheaTable> OpenTypeFont::gethheaTable (bool fail) {
	if (!hhea) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("extracting 'hhea' table");
		TablePtr oldhhea = getTable (hheaTag, fail);
		if (!oldhhea)
			return NULL;
		hhea = new hheaTable (*this, oldhhea->getMemory());
		replaceTable (hhea);
	}
	return hhea;
}

smart_ptr <hmtxTable> OpenTypeFont::gethmtxTable (bool fail) {
	if (!hmtx) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("extracting 'hmtx' table");
		TablePtr oldhmtx = getTable (hmtxTag, fail);
		if (!oldhmtx)
			return NULL;
		hmtx = new hmtxTable (*this, getmaxpTable(), gethheaTable(), oldhmtx->getMemory());
		replaceTable (hmtx);
	}
	return hmtx;
}

smart_ptr <maxpTable> OpenTypeFont::getmaxpTable (bool fail) {
	if (!maxp) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("extracting 'maxp' table");
		TablePtr oldmaxp = getTable (maxpTag, fail);
		if (!oldmaxp)
			return NULL;
		maxp = new maxpTable (*this, oldmaxp->getMemory());
		replaceTable (maxp);
	}
	return maxp;
}

smart_ptr <locaTable> OpenTypeFont::getlocaTable (bool fail) {
	if (!loca) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("extracting 'loca' table");
		TablePtr oldloca = getTable (locaTag, fail);
		if (!oldloca)
			return NULL;
		loca = new locaTable (*this, getmaxpTable(), getheadTable(), oldloca->getMemory());
		replaceTable (loca);
	}
	return loca;
}

smart_ptr <postTable> OpenTypeFont::getpostTable (bool fail) {
	if (!post) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("extracting 'post' table");
		TablePtr oldpost = getTable (postTag, fail);
		if (!oldpost)
			return NULL;
		post = new postTable (*this, oldpost->getMemory());
		replaceTable (post);
	}
	return post;
}

smart_ptr <cmapTable> OpenTypeFont::getcmapTable (bool fail) {
	if (!cmap) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("extracting 'cmap' table");
		TablePtr oldcmap = getTable (cmapTag, fail);
		if (!oldcmap)
			return NULL;
		cmap = new cmapTable (*this, oldcmap->getMemory());
		replaceTable (cmap);
	}
	return cmap;
}

smart_ptr <OS2Table> OpenTypeFont::getOS2Table (bool fail) {
	if (!OS2) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("extracting 'OS/2' table");
		TablePtr oldOS2 = getTable (OS2Tag, fail);
		if (!oldOS2)
			return NULL;
		OS2 = new OS2Table (*this, oldOS2->getMemory());
		replaceTable (OS2);
	}
	return OS2;
}

smart_ptr <GSUBTable> OpenTypeFont::getGSUBTable (bool fail) {
	if (!GSUB) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("extracting 'GSUB' table");
		TablePtr oldGSUB = getTable (GSUBTag, fail);
		if (!oldGSUB)
			return NULL;
		GSUB = new GSUBTable (*this, oldGSUB->getMemory());
		replaceTable (GSUB);
	}
	return GSUB;
}

smart_ptr <GPOSTable> OpenTypeFont::getGPOSTable (bool fail) {
	if (!GPOS) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("extracting 'GPOS' table");
		TablePtr oldGPOS = getTable (GPOSTag, fail);
		if (!oldGPOS)
			return NULL;
		GPOS = new GPOSTable (*this, oldGPOS->getMemory());
		replaceTable (GPOS);
	}
	return GPOS;
}

smart_ptr <GDEFTable> OpenTypeFont::getGDEFTable (bool fail) {
	if (!GDEF) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("extracting 'GDEF' table");
		TablePtr oldGDEF = getTable (GDEFTag, fail);
		if (!oldGDEF)
			return NULL;
		GDEF = new GDEFTable (*this, oldGDEF->getMemory());
		replaceTable (GDEF);
	}
	return GDEF;
}

void OpenTypeFont::extractGlyphs() {
	Exception::FontContext c1 (*this);
	Exception::Context c2 ("extracting glyph data");

	assert (!glyphsExtracted);
	getlocaTable();
	getpostTable();
	gethmtxTable();

	MemoryBlockPtr glyfMem = getTable (glyfTag, true)->getMemory();
	MemoryPen glyfStart (glyfMem);

	Locations & locations = loca->getLocations();
	Locations::iterator i;
	UShort glyphIndex = 0;
	glyphs.reserve (locations.size());
	for (i = locations.begin(); i != locations.end()-1; i ++, glyphIndex ++) {
		String name = post->getPostName (glyphIndex);
		HorMetric hm = hmtx->getHorMetric (glyphIndex);
		if (*i == *(i+1)) {
			// Empty glyph
			glyphs.push_back (new Glyph (*this, name, hm));
		} else {
			MemoryPen pen = glyfStart + *i;
			Short contourNum = pen.readUShort();
			if (contourNum <= 0) {
				if (contourNum != -1)
					throw Exception ("Invalid number of contours in glyph " +
						String (glyphIndex) + ": " + String(contourNum));
				glyphs.push_back (new CompositeGlyph (*this, name, hm, pen));
			} else
				glyphs.push_back (new SimpleGlyph (*this, name, hm, contourNum, pen));

			// Data validity & sanity check
			if (pen.getPosition() > *(i+1))
				throw Exception ("Overlapping glyph data in glyph " +
					String (glyphIndex));
			if (OT_PAD_TO (pen.getPosition(), 4) < *(i+1))
				addWarning (new Exception("Gap between glyph data positions in glyph " + 
					String (glyphIndex)));
		}
	}
	glyphsExtracted = true;

	// Delete 'loca' table
	deleteTable (locaTag);
	loca = NULL;
	// Delete 'hmtx' table
	deleteTable (hmtxTag);
	hmtx = NULL;
	// Clear 'post' table
	post->clear();
	// Delete 'glyf' table
	deleteTable (glyfTag);
}

void OpenTypeFont::extractNames() {
	Exception::FontContext c1 (*this);
	Exception::Context c2 ("extracting glyph names");

	if (!namesExtracted) {
		if (!glyphsExtracted)
			extractGlyphs();

		Glyphs::const_iterator g;
		for (g = glyphs.begin(); g != glyphs.end(); g ++)
			nameIndices.insert (pair <String, UShort> ((*g)->getName(), g - glyphs.begin()));

		namesExtracted = true;
	}
}

void OpenTypeFont::writeToFile (String outFileName) {
	// Reproduce changed glyph data
	if (glyphsExtracted) {
		Exception::FontContext c1 (*this);
		Exception::Context c2 ("preparing file data for \"" + outFileName + '"');
		// Regenerate these tables, which had been discarded as read
		assert (!loca);
		assert (!hmtx);
		assert (post);
		assert (!getTable (glyfTag, false));

		loca = new locaTable (*this);

		MemoryBlockPtr glyfMemory = new MemoryBlock;
		TablePtr glyf =
			new UnknownTable(*this, glyfTag, glyfMemory);
		// Regenerate glyf table
		MemoryWritePen glyfStart (glyfMemory);
		MemoryWritePen pen = glyfStart;

		Glyphs::iterator glyph;
		for (glyph = glyphs.begin(); glyph != glyphs.end(); glyph ++) {
			// Recalculate bounding box
			(*glyph)->recalculateBoundingBox();
			// Set loca table entry
			loca->addLocation (pen - glyfStart);
			// Write glyph to glyf table
			(*glyph)->write (pen);
			// Padding: it says 4 in the  specs, but that seems strange, as
			// there are no ULong values in there.
			pen.padWithZeros (4);
		}

		// Add sentinel entry
		loca->addLocation (pen - glyfStart);

		addTable (glyf);
		addTable (loca);
		hmtx = new hmtxTable (*this, gethheaTable(), glyphs);
		addTable (hmtx);
		
		post->set (glyphs);
		maxp->reset (glyphs);
		head->reset (glyphs, *loca);
		hhea->reset (glyphs);
	}

	// Produce cmap format 4 subtable if necessary
	if (unicode12Mapping) {
		unicode4Mapping->clear();
		unicode4Mapping->mergeFrom (unicode12Mapping);
	}

	OpenTypeFile::writeToFile (outFileName);
}

UShort OpenTypeFont::getGlyphNum() {
	Exception::FontContext c (*this);
	if (glyphsExtracted)
		return glyphs.size();
	else
		return getmaxpTable()->getGlyphNum();
}

GlyphPtr OpenTypeFont::getGlyph (UShort glyphIndex) {
	Exception::FontContext c (*this);

	if (!glyphsExtracted)
		extractGlyphs();
	if (glyphIndex >= glyphs.size())
		throw Exception ("Glyph index " + String (glyphIndex) +
			" out of bounds ( max " + String (glyphs.size()) + ")");
	return glyphs [glyphIndex];
}

UShort OpenTypeFont::getGlyphIndex (String name, bool fail) {
	Exception::FontContext c (*this);

	extractNames();
	NameIndices::iterator i = nameIndices.lower_bound (name);
	if (i != nameIndices.end() && i->first == name)
		return i->second;
	else {
		if (fail)
			throw Exception ("Glyph '" + name + "' not found");
		else
			return 0;
	}
}

GlyphPtr OpenTypeFont::getGlyph (String name, bool fail) {
	return getGlyph (getGlyphIndex (name, fail));
}

void OpenTypeFont::replaceGlyph (UShort index, GlyphPtr newGlyph) {
	Exception::FontContext c1 (*this);
	Exception::Context c2 ("replacing glyph");
	if (!glyphsExtracted)
		extractGlyphs();
	if (index >= glyphs.size())
		throw Exception ("Glyph index out of bounds");
	if (namesExtracted) {
		// Replace old name with new name
		pair <NameIndices::iterator, NameIndices::iterator> range =
			nameIndices.equal_range (glyphs [index]->getName());
		while (range.first->second != index) {
			range.first ++;
			assert (range.first != range.second);
		}
		nameIndices.erase (range.first);

		nameIndices.insert (pair <String, UShort> (newGlyph->getName(), index));
	}
	
	glyphs [index] = newGlyph;
}

UShort OpenTypeFont::addGlyph (GlyphPtr newGlyph) {
	Exception::FontContext c1 (*this);
	Exception::Context c2 ("adding glyph");
	if (!glyphsExtracted)
		extractGlyphs();
	UShort newIndex = glyphs.size();
	glyphs.push_back (newGlyph);
	if (namesExtracted) {
		// Add new name to name index
		nameIndices.insert (pair <String, UShort> (newGlyph->getName(), newIndex));
	}
	return newIndex;
}

void OpenTypeFont::extractUnicodeMapping() {
	Exception::FontContext c1 (*this);
	Exception::Context c2 ("extracting glyph mappings");

	if (!unicodeMapping) {
		// Simple implementation
/*		unicodeMapping = getcmapTable()->getFirstKnownMappingTable (3, 1);
		if (!unicodeMapping)
			throw Exception ("No Microsoft Unicode mapping table found");*/

		getcmapTable();
		unicode4Mapping = cmap->getFirstKnownMappingTable (3, 1, 4);
		unicode12Mapping = cmap->getFirstKnownMappingTable (3, 10, 12);
		if (unicode4Mapping) {
			if (unicode12Mapping) {
				// Use most extensive table
				unicodeMapping = unicode12Mapping;
				unicode12Mapping->mergeFrom (unicode4Mapping);
			} else {
				unicodeMapping = unicode4Mapping;
			}
		} else {
			if (unicode12Mapping) {
				unicodeMapping = unicode12Mapping;
				// No format 4 table is available, but it should be there
				// according to the specs (for backward compatibility) so
				// add it
				unicode4Mapping = cmap->newMappingTable (3, 1, 4);
			} else {
				throw Exception ("No Microsoft Unicode mapping table found");
			}
		}
	}
}

UShort OpenTypeFont::getGlyphIndexByUnicode (ULong unicode) {
	Exception::FontContext c (*this);
	if (!unicodeMapping)
		extractUnicodeMapping();
	return unicodeMapping->getGlyphId (unicode);
}

GlyphPtr OpenTypeFont::getGlyphByUnicode (ULong unicode) {
	return getGlyph (getGlyphIndexByUnicode (unicode));
}

void OpenTypeFont::addUnicodeMapping (ULong unicode, UShort glyphId){
	Exception::FontContext c (*this);

	if (!glyphsExtracted)
		extractGlyphs();
	if (!unicodeMapping)
		extractUnicodeMapping();

	if (glyphId >= glyphs.size())
		throw Exception ("Cannot add mapping to non-existing glyph " +
			String (glyphId));

	if (unicode > 0xFFFF && !unicode12Mapping) {
		// Once a mapping that cannot be added to the cmap format 4 subtable
		// is to be added, produce a format 12 (UCS-4) table.
		unicode12Mapping = cmap->newMappingTable (3, 10, 12);
		unicode12Mapping->mergeFrom (unicode4Mapping);
		unicodeMapping = unicode12Mapping;
	}

	unicodeMapping->addMapping (unicode, glyphId);
}

void OpenTypeFont::setGridFittingBehaviour (const GridFittingBehaviour::Ranges &newRanges) {
	Exception::FontContext c1 (*this);
	Exception::Context c2 ("creating 'gasp' table");

	replaceTable (new gaspTable (*this, newRanges), false);
}

MemoryBlockPtr OpenTypeFont::getprep(bool fail) {
	Exception::FontContext c (*this);
	TablePtr table = getTable (prepTag, fail);
	if (!table)
		return MemoryBlockPtr();
	else
		return table->getMemory();
}

MemoryBlockPtr OpenTypeFont::getfpgm(bool fail) {
	Exception::FontContext c (*this);
	TablePtr table = getTable (fpgmTag, fail);
	if (!table)
		return MemoryBlockPtr();
	else
		return table->getMemory();
}

MemoryBlockPtr OpenTypeFont::getcvt(bool fail) {
	Exception::FontContext c (*this);
	TablePtr table = getTable (cvtTag, fail);
	if (!table)
		return MemoryBlockPtr();
	else
		return table->getMemory();
}

void OpenTypeFont::setprep (MemoryBlockPtr newprep) {
	Exception::FontContext c (*this);
	replaceTable (new UnknownTable (*this, prepTag, newprep), false);
}

void OpenTypeFont::setfpgm (MemoryBlockPtr newprep) {
	Exception::FontContext c (*this);
	replaceTable (new UnknownTable (*this, fpgmTag, newprep), false);
}

void OpenTypeFont::setcvt (MemoryBlockPtr newprep) {
	Exception::FontContext c (*this);
	replaceTable (new UnknownTable (*this, cvtTag, newprep), false);
}

UShort OpenTypeFont::getUnitsPerEm() {
	return getheadTable()->getUnitsPerEm();
}

UShort OpenTypeFont::getWinAscent() {
	return getOS2Table()->getWinAscent();
}

UShort OpenTypeFont::getWinDescent() {
	return getOS2Table()->getWinDescent();
}

UShort OpenTypeFont::getMaxStorage() {
	return getmaxpTable()->getMaxStorage();
}

UShort OpenTypeFont::getMaxTwilightPoints() {
	return getmaxpTable()->getMaxTwilightPoints();
}

UShort OpenTypeFont::getMaxStackElements() {
	return getmaxpTable()->getMaxStackElements();
}

UShort OpenTypeFont::getMaxFunctionDefs() {
	return getmaxpTable()->getMaxFunctionDefs();
}

void OpenTypeFont::setMaxStorage (UShort maxStorage) {
	getmaxpTable()->setMaxStorage (maxStorage);
}

void OpenTypeFont::setMaxFunctionDefs (UShort maxFunctionDefs) {
	getmaxpTable()->setMaxFunctionDefs (maxFunctionDefs);
}

void OpenTypeFont::setMaxInstructionDefs (UShort maxInstructionDefs) {
	getmaxpTable()->setMaxInstructionDefs (maxInstructionDefs);
}

void OpenTypeFont::setMaxSizeOfInstructions (UShort maxSizeOfInstructions) {
	getmaxpTable()->setMaxSizeOfInstructions (maxSizeOfInstructions);
}

void OpenTypeFont::setMaxStackElements (UShort maxStackElements) {
	getmaxpTable()->setMaxStackElements (maxStackElements);
}

void OpenTypeFont::setMaxZones (UShort maxZones) {
	getmaxpTable()->setMaxZones (maxZones);
}

void OpenTypeFont::setMaxTwilightPoints (UShort maxTwilightPoints) {
	getmaxpTable()->setMaxTwilightPoints (maxTwilightPoints);
}


/*
// set_unsorted_union is the unsorted counterpart of the STL set_union:
// it returns the union of two unsorted sets.
// This has complexity O (n1 * n2), but as it is used for short tag lists
// (n = 20), it is not _too_ slow in practice.
// It takes a set1 range and a set2 range. set2 will be added to set1.

template <class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator set_unsorted_union(InputIterator1 s1begin, InputIterator1 s1end,
								  InputIterator2 s2begin, InputIterator2 s2end,
								  OutputIterator output)
{
	InputIterator1 s1cur = s1begin;
	// Copy set1
	while (s1cur != s1end)
		*output ++ = *s1cur ++;
	// Copy set2
	while (s2begin != s2end) {
		s1cur = s1begin;
		while (s1cur != s1end) {
			if (*s1cur == *s2begin)
				break;
			s1cur++;
		}
		if (s1cur == s1end)
			// The element was not found
			*output ++ = *s2begin;
		s2begin ++;
	}
	return output;
}*/

shared_vector <Tag> OpenTypeFont::getScripts() {
	typedef shared_vector <Tag> Tags;
	Tags GSUBtags, GPOStags;
	if (getGSUBTable (false))
		GSUBtags = GSUB->getScripts();
	sort (GSUBtags.begin(), GSUBtags.end(), compareTags);

	if (getGPOSTable (false))
		GPOStags = GPOS->getScripts();
	sort (GPOStags.begin(), GPOStags.end(), compareTags);

	util::shared_vector <Tag> tags;
	set_union (GSUBtags.begin(), GSUBtags.end(),
		GPOStags.begin(), GPOStags.end(),
		std::insert_iterator <Tags> (tags, tags.end()));

	return tags;
}

shared_vector <Tag> OpenTypeFont::getLanguages (Tag script) {
	typedef shared_vector <Tag> Tags;
	Tags GSUBtags, GPOStags;
	if (getGSUBTable (false))
		GSUBtags = GSUB->getLanguages (script);
	if (getGPOSTable (false))
		GPOStags = GPOS->getLanguages (script);
	Tags tags;
	set_union (GSUBtags.begin(), GSUBtags.end(),
		GPOStags.begin(), GPOStags.end(),
		std::insert_iterator <Tags> (tags, tags.end()));

	return tags;
}
/*vector <Tag> OpenTypeFont::getFeatures (Tag script, Tag language) {
	typedef vector <Tag> Tags;
	Tags GSUBtags, GPOStags;
	if (getGSUBTable (false))
		GSUBtags = GSUB->getFeatures (script, language);
	if (getGPOSTable (false))
		GPOStags = GPOS->getFeatures (script, language);
	Tags tags;
	set_unsorted_union (GSUBtags.begin(), GSUBtags.end(),
		GPOStags.begin(), GPOStags.end(),
		std::insert_iterator <Tags> (tags, tags.end()));

	return tags;
}*/

shared_vector <Tag> OpenTypeFont::getSubstitutionFeatures (Tag script, Tag language) {
	if (getGSUBTable (false))
		return GSUB->getFeatures (script, language);
	else
		return shared_vector <Tag> ();
}

shared_vector <Tag> OpenTypeFont::getPositioningFeatures (Tag script, Tag language) {
	if (getGPOSTable (false))
		return GPOS->getFeatures (script, language);
	else
		return shared_vector <Tag> ();
}

shared_vector <Tag> OpenTypeFont::getFeatures (Tag script, Tag language) {
	typedef shared_vector <Tag> Tags;
	Tags GSUBTags = getSubstitutionFeatures (script, language);
	Tags GPOSTags = getPositioningFeatures (script, language);

	Tags tags;
	set_union (GSUBTags.begin(), GSUBTags.end(),
		GPOSTags.begin(), GPOSTags.end(),
		std::insert_iterator <Tags> (tags, tags.end()), compareTags);
	return tags;
}

UShort OpenTypeFont::getGlyphClass (GlyphId glyphId) {
	if (getGDEFTable (false))
		return GDEF->getGlyphClass (glyphId);
	else
		return 0;
}

UShort OpenTypeFont::getMarkAttachmentClass (GlyphId glyphId) {
	if (getGDEFTable (false))
		return GDEF->getMarkAttachmentClass (glyphId);
	else
		return 0;
}

void OpenTypeFont::setGSUB (TablePtr table) {
	Exception::Context c ("Adding glyph substitution table");
	GSUB = NULL;
	replaceTable (table, false);
}

void OpenTypeFont::setGPOS (TablePtr table) {
	Exception::Context c ("Adding glyph positioning table");
	GPOS = NULL;
	replaceTable (table, false);
}

void OpenTypeFont::setGDEF (TablePtr table) {
	Exception::Context c ("Adding glyph definition table");
	GDEF = NULL;
	replaceTable (table, false);
}

} // end namespace OpenType
