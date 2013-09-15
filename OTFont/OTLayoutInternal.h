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
	\file Internal definitions for OTGSUBTable, OTGPOSTable, OTGDEFTable.
	This include file should not be included in other files than the
	implementation files for those classes.
*/

#ifndef OTLAYOUTINTERNAL_H
#define OTLAYOUTINTERNAL_H

#include <vector>
#include "OTLayoutTable.h"
#include "OpenTypeText.h"

namespace OpenType {
	typedef util::shared_vector <Tag> Tags;
	typedef std::vector <UShort> Indices;
	typedef std::vector <UShort> GlyphIds;
	typedef std::pair <Indices, UShort> FeatureIndices;
	typedef std::vector <UShort> Classes;

	typedef std::vector <bool> BoolVector;


	// ScriptList

	class Script;
	typedef util::smart_ptr <Script> ScriptPtr;
	typedef std::vector <ScriptPtr> Scripts;

	class ScriptList {
		Scripts scripts;
	public:
		ScriptList (MemoryPen pen);
		virtual ~ScriptList();

		Tags getScriptTags() const;
		Tags getLanguageTags (Tag script) const;
		const FeatureIndices & getFeatureIndices (Tag script, Tag language) const;
	};

	// Script

	class LanguageSystem;
	typedef util::smart_ptr <LanguageSystem> LanguagePtr;
	typedef std::vector <LanguagePtr> Languages;

	class Script {
		LanguagePtr defaultLanguage;
		Languages languages;

		Tag tag;
	public:
		Script (Tag aTag, MemoryPen pen);
		virtual ~Script();

		Tag getTag() const;
		Tags getLanguageTags() const;
		const FeatureIndices & getFeatureIndices (Tag language) const;
	};

	// LanguageSystem

	class LanguageSystem {
		Tag tag;
		FeatureIndices features;
	public:
		LanguageSystem (Tag aTag, MemoryPen pen);
		virtual ~LanguageSystem();

		Tag getTag() const;
		const FeatureIndices & getFeatures() const;
	};

	// FeatureList

	class Feature;
	typedef util::smart_ptr <Feature> FeaturePtr;
	typedef std::vector <FeaturePtr> Features;

	class FeatureList {
		Features features;
	public:
		FeatureList (MemoryPen pen);
		virtual ~FeatureList();

		Tag getFeatureTag (UShort index) const;
		void getLookups (BoolVector & lookups, UShort index) const;
	};

	// Feature

	class Feature {
		Tag tag;
		std::vector <UShort> lookups;
	public:
		Feature (Tag aTag, MemoryPen pen);
		virtual ~Feature();

		Tag getTag() const;
		void getLookups (BoolVector &lookups) const;
	};

	// LookupList

	class Lookup;
	class LookupSubTable;
	typedef util::smart_ptr <Lookup> LookupPtr;
	typedef std::vector <LookupPtr> Lookups;

	class LookupList {
		Lookups lookups;
	public:
		LookupList (MemoryPen pen, OpenTypeFont &font,
			util::smart_ptr <LookupSubTable> newSubTable (UShort, UShort, MemoryPen,
			const LookupList &, OpenTypeFont &));
		virtual ~LookupList();

		UShort getNum() const { return lookups.size(); }
		void applyLookups (OpenTypeText &text, const BoolVector &lookupSet) const;
		void applyLookups (OpenTypeText::iterator begin, OpenTypeText::iterator current,
			OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end, UShort index) const;
	};

	// Lookup

	class Lookup {
		OpenTypeFont &font;
		UShort type;
		UShort flags;

		typedef util::smart_ptr <LookupSubTable> SubTablePtr;
		typedef std::vector <SubTablePtr> SubTables;
		SubTables subTables;
	public:
		Lookup (MemoryPen pen, OpenTypeFont &font, const LookupList &lookupList,
			SubTablePtr newSubTable (UShort, UShort, MemoryPen,
			const LookupList &, OpenTypeFont &));
		virtual ~Lookup();

		virtual void apply (OpenTypeText &text) const;
		virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator current,
			OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
	};

	class LookupSubTable {
	public:
		LookupSubTable() {}
		virtual ~LookupSubTable() {}

		virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
			OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const = 0;
	};

	// CoverageTable

	class CoverageTable {
	public:
		virtual ~CoverageTable() {}

		virtual bool isCovered (UShort glyphId, UShort *index) const = 0;
		virtual UShort getGlyphNum() const = 0;
		virtual GlyphId getMaxGlyphId() const = 0;
	};

	typedef util::smart_ptr <CoverageTable> CoveragePtr;
	typedef std::vector <CoveragePtr> CoveragePtrs;
	typedef std::vector <GlyphId> GlyphIds;

	/**
		\brief Return a CoverageTable object read from the position.

		It may throw an Exception object when the glyph indices are out of
		bounds.
	*/
	CoveragePtr getCoverageTable (MemoryPen, OpenTypeFont &);

	// ClassDefTable

	class ClassDefTable {
	public:
		ClassDefTable () {}
		virtual ~ClassDefTable() {}

		virtual UShort getClass (GlyphId glyph) const = 0;
		virtual UShort getClassNum() const = 0;
	};

	typedef util::smart_ptr <ClassDefTable> ClassDefPtr;
	ClassDefPtr getClassDefTable (MemoryPen);

	class ClassDefTable1 : public ClassDefTable {
		GlyphId start;
		Classes classes;
	public:
		ClassDefTable1 (MemoryPen pen);
		virtual ~ClassDefTable1();

		virtual UShort getClass (GlyphId glyph) const;
		virtual UShort getClassNum() const;
	};

	class ClassDefTable2 : public ClassDefTable {
		typedef struct {
			GlyphId start, end;
			UShort glyphClass;
		} ClassRange;
		typedef std::vector <ClassRange> ClassRanges;
		ClassRanges ranges;
	public:
		ClassDefTable2 (MemoryPen pen);
		virtual ~ClassDefTable2();

		virtual UShort getClass (GlyphId glyph) const;
		virtual UShort getClassNum() const;
	};

	/*** Contextual lookups (are the same for GPOS and GSUB) ***/

	// LookupReference

	class LookupReference {
		UShort sequenceIndex;
		UShort lookupListIndex;
		const LookupList &lookupList;
	public:
		LookupReference (MemoryPen &pen, const LookupList & _lookupList);
		void apply (OpenTypeText::iterator begin, OpenTypeText::iterator current,
			OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
	};

	typedef util::smart_ptr <LookupReference> LookupReferencePtr;
	typedef std::vector <LookupReferencePtr> LookupReferences;

	// ContextLookupSubTable

	class ContextLookupSubTable : public LookupSubTable {
	protected:
		void applyReferences (OpenTypeText::iterator begin, OpenTypeText::iterator current,
			OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end,
			const LookupReferences & references) const
		{
			for (LookupReferences::const_iterator r = references.begin(); r != references.end(); r ++) {
				(*r)->apply (begin, current, scopeEnd, end);
			}
		}

		bool checkInput (const CoveragePtrs &coverage, OpenTypeText::iterator &current,
			OpenTypeText::iterator end) const
		{
			CoveragePtrs::const_iterator c;
			for (c = coverage.begin();
			c != coverage.end() && current != end; c ++, current ++) {
				if (!(*c)->isCovered ((*current)->getGlyphId(), NULL))
					return false;
			}
			return c == coverage.end();
		}

		bool checkReverseInput (const CoveragePtrs &coverage, OpenTypeText::iterator &current,
			OpenTypeText::iterator begin) const
		{
			CoveragePtrs::const_iterator c;
			if (current == begin)
				return coverage.empty();
			current --;
			for (c = coverage.begin();
			c != coverage.end() && current != begin; c ++, current --) {
				if (!(*c)->isCovered ((*current)->getGlyphId(), NULL))
					return false;
			}
			if (c == coverage.end())
				return true;
			else {
				// We're stranded at the begin of the sequence
				if (c == coverage.end() - 1 && current == begin) {
					// This may yet be a good input sequence
					return (*c)->isCovered ((*current)->getGlyphId(), NULL);
				}
				return false;
			}
		}

	public:
		ContextLookupSubTable() {}
		virtual ~ContextLookupSubTable() {}
	};

	// ContextLookup

	class ContextLookup3 : public ContextLookupSubTable {
		CoveragePtrs coverage;
		LookupReferences lookupReferences;
	public:
		ContextLookup3 (MemoryPen pen, const LookupList &lookupList, OpenTypeFont &font);
		~ContextLookup3();
		virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
			OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
	};

	// ChainingContextLookup

	class ChainingContextLookup3 : public ContextLookupSubTable {
		CoveragePtrs backtrackCoverage;
		CoveragePtrs inputCoverage;
		CoveragePtrs lookAheadCoverage;
		LookupReferences lookupReferences;
	public:
		ChainingContextLookup3 (MemoryPen pen, const LookupList &lookupList, OpenTypeFont &font);
		~ChainingContextLookup3();
		virtual void apply (OpenTypeText::iterator begin, OpenTypeText::iterator & current,
			OpenTypeText::iterator scopeEnd, OpenTypeText::iterator end) const;
	};
}

#endif // OTLAYOUTINTERNAL_H
