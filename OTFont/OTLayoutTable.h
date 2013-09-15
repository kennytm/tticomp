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
	\file Represents an OpenType Layout table.
*/

#ifndef OTLAYOUTTABLE_H
#define OTLAYOUTTABLE_H

#include "../Util/shared_vector.h"
#include "OTTable.h"

namespace OpenType {

	class ScriptList;
	class FeatureList;
	class LookupList;
	class OpenTypeText;
	class LookupSubTable;
	class ClassDefTable;
	class OpenTypeFont;

	typedef util::shared_vector <Tag> Tags;

	class LayoutTable : public Table {
		MemoryBlockPtr memory;

		util::smart_ptr <ScriptList> scriptList;
		util::smart_ptr <FeatureList> featureList;
		util::smart_ptr <LookupList> lookupList;

	public:
		LayoutTable (OpenTypeFont &aFont, MemoryBlockPtr aMemory,
			util::smart_ptr <LookupSubTable> newSubTable (UShort, UShort, MemoryPen,
			const LookupList &, OpenTypeFont &));
		virtual ~LayoutTable();

		virtual MemoryBlockPtr getMemory() const;

		/// \brief Return scripts (sorted)
		util::shared_vector <Tag> getScripts() const;
		/// \brief Return languages (sorted)
		util::shared_vector <Tag> getLanguages (Tag script) const;
		/// \brief Return features (sorted)
		util::shared_vector <Tag> getFeatures (Tag script, Tag language) const;

		void apply (OpenTypeText &text, Tag script, Tag language,
			Tags features) const;
	};

	class GSUBTable : public LayoutTable {
	public:
		GSUBTable (OpenTypeFont &aFont, MemoryBlockPtr aMemory);
		virtual ~GSUBTable();
		virtual Tag getTag() const;
	};

	class GPOSTable : public LayoutTable {
	public:
		GPOSTable (OpenTypeFont &aFont, MemoryBlockPtr aMemory);
		virtual ~GPOSTable();
		virtual Tag getTag() const;
	};

	class GDEFTable : public Table {
		MemoryBlockPtr memory;

		util::smart_ptr <ClassDefTable> glyphClasses;
		util::smart_ptr <ClassDefTable> markAttachmentClasses;
	public:
		GDEFTable (OpenTypeFile &aFont, MemoryBlockPtr aMemory);
		virtual ~GDEFTable();

		virtual MemoryBlockPtr getMemory() const;
		virtual Tag getTag() const;

		UShort getGlyphClass (GlyphId glyphId) const;
		UShort getMarkAttachmentClass (GlyphId glyphId) const;
	};

	class Anchor {
		Short x, y;
	public:
		Anchor (MemoryPen &pen);
		virtual ~Anchor();

		Short getX() const;
		Short getY() const;

		// Returns 0xFFFF if no contour point is defined.
		virtual UShort getContourPoint() const = 0;
	};

	typedef util::smart_ptr <Anchor> AnchorPtr;
}

#endif	// OTLAYOUTTABLE_H
