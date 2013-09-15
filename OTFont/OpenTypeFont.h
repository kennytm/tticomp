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
	\file OpenTypeFont.h provides access to %OpenType fonts.
	It allows for manipulating glyphs and mapping data.
*/

#ifndef OPENTYPEFONT_H
#define OPENTYPEFONT_H

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <map>

#include "../Util/shared_vector.h"

#include "OpenTypeFile.h"
#include "OTgaspTable.h"

namespace OpenType {

	typedef std::vector <GlyphPtr> Glyphs;

	typedef std::multimap <util::String, UShort> NameIndices;

	/**
		\brief Enables OpenType font manipulation.

		OpenTypeFont reads and writes OpenType fonts with TrueType outlines.
		It has methods for manipulating glyphs, reading and writing various
		global font variables and reading OpenType layout tables.
	*/

	class OpenTypeFont : public OpenTypeFile {
		bool glyphsExtracted;
		Glyphs glyphs;
		bool namesExtracted;
		NameIndices nameIndices;

		/// unicode4Mapping contains the cmap format 4 subtable; unicode12Mapping
		/// contains the cmap format 12 subtable, which must be a superset of
		/// unicode4Mapping, if it is there at all.
		/// unicodeMapping points to one of these tables, but in all cases to
		/// the most extensive table.
		/// When the font is written, unicode4Mapping and unicode12Mapping are
		/// synchronised.
		MappingTablePtr unicodeMapping;
		MappingTablePtr unicode4Mapping;
		MappingTablePtr unicode12Mapping;

		util::smart_ptr <headTable> head;
		util::smart_ptr <hheaTable> hhea;
		util::smart_ptr <hmtxTable> hmtx;
		util::smart_ptr <maxpTable> maxp;
		util::smart_ptr <locaTable> loca;
		util::smart_ptr <postTable> post;
		util::smart_ptr <cmapTable> cmap;
		util::smart_ptr <OS2Table> OS2;
		util::smart_ptr <GSUBTable> GSUB;
		util::smart_ptr <GPOSTable> GPOS;
		util::smart_ptr <GDEFTable> GDEF;

		util::smart_ptr <headTable> getheadTable (bool fail = true);
		util::smart_ptr <hheaTable> gethheaTable (bool fail = true);
		util::smart_ptr <hmtxTable> gethmtxTable (bool fail = true);
		util::smart_ptr <maxpTable> getmaxpTable (bool fail = true);
		util::smart_ptr <locaTable> getlocaTable (bool fail = true);
		util::smart_ptr <postTable> getpostTable (bool fail = true);
		util::smart_ptr <cmapTable> getcmapTable (bool fail = true);
		util::smart_ptr <OS2Table> getOS2Table (bool fail = true);
		util::smart_ptr <GDEFTable> getGDEFTable (bool fail = true);

		void extractGlyphs();
		void extractNames();
		void extractUnicodeMapping();

	protected:
		friend class OpenTypeText;
		util::smart_ptr <GSUBTable> getGSUBTable (bool fail = true);
		util::smart_ptr <GPOSTable> getGPOSTable (bool fail = true);

	public:
		OpenTypeFont ();
		virtual ~OpenTypeFont();

		/// Read an OpenType font from file
		void readFromFile (util::String aFileName);
		/// Write an OpenType font to file, updating glyph information tables
		/// and general information tables accordingly.
		virtual void writeToFile (util::String outFileName);

		/// Return the number of glyphs.
		UShort getGlyphNum();
		/// Return the glyph at a given index.
		GlyphPtr getGlyph (UShort index);
		/// Return the index of the first glyph with the given name.
		/// If fail==true, an exception is thrown if there is no glyph with that name.
		UShort getGlyphIndex (util::String name, bool fail = false);
		/// Return the first glyph with the given name.
		/// If fail==true, an exception is thrown if there is no glyph with that name.
		GlyphPtr getGlyph (util::String name, bool fail = false);

		/// Replace the glyph at index by the new glyph.
		void replaceGlyph (UShort index, GlyphPtr newGlyph);

		/// Add newGlyph to the font and return the new glyph's index
		GlyphId addGlyph (GlyphPtr newGlyph);

		/// Return the glyph index for a Unicode value.
		UShort getGlyphIndexByUnicode (ULong unicode);
		/// Return a glyph for a Unicode value. Equivalent to
		/// getGlyph (getGlyphIndexByUnicode (unicode)).
		GlyphPtr getGlyphByUnicode (ULong unicode);
		/// Add or change a unicode-to-glyph mapping
		void addUnicodeMapping (ULong unicode, UShort glyphId);

		/*** gasp table access ***/
		void setGridFittingBehaviour (const GridFittingBehaviour::Ranges &newRanges);

		/*** Font properties access ***/
		UShort getUnitsPerEm();
		UShort getWinAscent();
		UShort getWinDescent();

		/*** Instruction table access ***/
		MemoryBlockPtr getfpgm (bool fail = true);
		MemoryBlockPtr getprep (bool fail = true);
		MemoryBlockPtr getcvt (bool fail = true);
		void setfpgm (MemoryBlockPtr newfpgm);
		void setprep (MemoryBlockPtr newprep);
		void setcvt (MemoryBlockPtr newcvt);

		UShort getMaxStackElements();
		UShort getMaxStorage();
		UShort getMaxZones();
		UShort getMaxTwilightPoints();
		UShort getMaxFunctionDefs();
		void setMaxStorage (UShort maxStorage);
		void setMaxFunctionDefs (UShort maxFunctionDefs);
		void setMaxInstructionDefs (UShort maxInstructionDefs);
		void setMaxSizeOfInstructions (UShort maxSizeOfInstructions);
		void setMaxStackElements (UShort maxStackElements);
		void setMaxZones (UShort maxZones);
		void setMaxTwilightPoints (UShort maxTwilightPoints);

		/*** OpenType Layout table access ***/
		/// \brief Return sorted script tags
		util::shared_vector <Tag> getScripts();
		/// \brief Return sorted language tags for this script
		util::shared_vector <Tag> getLanguages (Tag script);
		/// \brief Return sorted GPOS feature tags
		util::shared_vector <Tag> getPositioningFeatures (Tag script, Tag language);
		/// \brief Return sorted GSUB feature tags
		util::shared_vector <Tag> getSubstitutionFeatures (Tag script, Tag language);
		/// \brief Return all sorted feature tags
		util::shared_vector <Tag> getFeatures (Tag script, Tag language);

		UShort getGlyphClass (GlyphId glyphId);
		UShort getMarkAttachmentClass (GlyphId glyphId);

		void setGSUB (TablePtr table);
		void setGPOS (TablePtr table);
		void setGDEF (TablePtr table);
	};

}

#endif	// OPENTYPEFONT_H
