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
	\file OpenTypeText.h Defines a line of text on which %OpenType substitution
	and positioning may take place.
*/

#ifndef OPENTYPETEXT_H
#define OPENTYPETEXT_H

#include <vector>
#include <list>
#include <algorithm>

#include "OpenType.h"

namespace OpenType {

	class OpenTypeChar;
	class OpenTypeFont;
	class Anchor;
	class OpenTypeTextIterator;

	typedef util::smart_ptr <OpenTypeChar> OpenTypeCharPtr;

	/** \brief Represents a run of text that is manipulated by OpenType
		layout tables.
	*/
	class OpenTypeText {
	public:
		typedef std::vector <ULong> Unicodes;
		typedef std::vector <GlyphId> GlyphIds;
		typedef std::list <OpenTypeCharPtr> Chars;
		typedef OpenTypeTextIterator iterator;

	protected:
		friend class OpenTypeTextIterator;
		Chars chars;
		OpenTypeFont &font;

		virtual OpenTypeCharPtr newOpenTypeChar (UShort glyphId);
		virtual void beforePositioning() {}
	public:
		OpenTypeText (OpenTypeFont &aFont);
		virtual ~OpenTypeText();

		void setGlyphs (const GlyphIds glyphIds);
		void setUnicodes (const Unicodes unicodes);

		void applyLookups (Tag script, Tag language, std::vector <Tag> features);

		/// \brief Return the first glyph of the text.
		///
		/// All glyphs that do not conform to flags are skipped.
		iterator begin (UShort flags);
		iterator end (UShort flags);

		/// \brief Replace one character by a new glyph.
		///
		/// Returns an iterator pointing to the new glyph.
		/// \param i1 Pointer to the original glyph.
		/// \param glyphId New glyph index.
		iterator replace (iterator it, GlyphId glyphId);
		/// \brief Replace multiple characters by a new glyph.
		///
		/// Returns an iterator pointing to the new glyph.
		/// All glyphs that are skipped by the iterator are moved out
		/// of the way.
		/// \param begin,end Range [begin, end> to be deleted.
		/// \param glyphId New glyph index.
		iterator replace (iterator begin, iterator end, GlyphId glyphId);
		/// \brief Replace one character by a number of new glyphs.
		///
		/// Returns an iterator pointing to the first new glyph.
		/// \param i1 Pointer to the original glyph.
		/// \param glyphIds New glyph indices.
		iterator replace (iterator i1, GlyphIds glyphId);
	};

	class OpenTypeChar {
	public:
		typedef enum {
			gcUndefined = 0,
			gcBaseGlyph = 1,
			gcLigature = 2,
			gcMark = 3,
			gcComponent = 4
		} GlyphClass;

		typedef enum {
			lfRightToLeft = 0x0001,
			lfIgnoreBaseGlyphs = 0x0002,
			lfIgnoreLigatures = 0x0004,
			lfIgnoreMarks = 0x0008,
			//lfReserved = 0x00F0;
			lfMarkAttachmentType = 0xFF00
		} LookupFlag;

		typedef struct {
			Short x, y;
		} Position;

	protected:
		/// Glyph ID
		UShort glyphId;
		/// Glyph class: unknown/base glyph/ligature/mark/component
		GlyphClass glyphClass;
		/// Mark attachment class
		UShort markAttachmentClass;
		/// Number of characters back a mark applies to
		UShort appliesTo;

		OpenTypeFont &font;

		/// Advance width, x and y may be in local coordinates (for example,
		/// screen coordinates).
		UShort advance;
		Position position;
	protected:
		/// Get local coordinates from global coordinates. Default just returns
		/// the global coordinate.
		virtual Position getLocalCoordinates (Position global) const;
		virtual Position getLocalCoordinates (const Anchor &anchor) const;

		friend class OpenTypeText;
		/// \brief Indicate that the glyph is being moved forward.
		///
		/// The appliesTo index will be set accordingly.
		///
		/// This happens for example when a sequence f - mark - i is turned into
		/// a ligature: this character needs to know what component of the
		/// newly formed ligature to apply to (in the example, to the 0th component).
		void moveForward (UShort n);
		void moveBack (UShort n);

	public:
		OpenTypeChar (UShort aGlyphId, OpenTypeFont &aFont);
		virtual ~OpenTypeChar();

		GlyphId getGlyphId() const {return glyphId;}
		UShort getAppliesTo() { return appliesTo; }
		virtual void setGlyphId (GlyphId aGlyphId);
		GlyphClass getGlyphClass() const;

		virtual void move (Short aX, Short aY,
			Short aXAdvance, Short aYAdvance);
		virtual void attach (const Anchor &thisAnchor,
			OpenTypeText::Chars::const_iterator base, const Anchor &baseAnchor);

		/// Default getAdvance returns the advance in global coordinates, but
		/// other implementations may return local coordinates and/or round.
		virtual UShort getAdvance() const;
		virtual Position getPosition() const;

		bool skip (LookupFlag flags) const;
	};

	OpenTypeChar::Position operator - (const OpenTypeChar::Position &pos1, const OpenTypeChar::Position &pos2);
	OpenTypeChar::Position operator + (const OpenTypeChar::Position &pos1, const OpenTypeChar::Position &pos2);

	class OpenTypeTextIterator : public OpenTypeText::Chars::iterator {
		OpenTypeText *text;
		//typedef OpenTypeText::Chars::iterator iterator;
	protected:
		friend class OpenTypeText;
		UShort flags;
	public:
		OpenTypeTextIterator (const OpenTypeText::Chars::iterator &i, OpenTypeText *_text, UShort _flags)
			: OpenTypeText::Chars::iterator (i), text (_text), flags (_flags) {}
		OpenTypeTextIterator (const OpenTypeTextIterator &i)
			: OpenTypeText::Chars::iterator (i), text (i.text), flags (i.flags) {}

		OpenTypeTextIterator & operator = (const OpenTypeTextIterator &i) {
			OpenTypeText::Chars::iterator::operator = (i);
			text = i.text;
			flags = i.flags;
			return *this;
		}

		OpenTypeText & getText () const { return *text; }

		OpenTypeTextIterator & operator ++() {
			assert (*this != text->end (flags));
			do {
				OpenTypeText::Chars::iterator::operator ++();
			} while (*this != text->end (flags) && (**this)->skip ((OpenTypeChar::LookupFlag) flags));
			return *this;
		}

		OpenTypeTextIterator operator ++ (int n) {
			OpenTypeTextIterator i = *this;
			++ (*this);
			return i;
		}

		OpenTypeTextIterator & operator --() {
			assert (*this != text->begin (flags));
			do {
				OpenTypeText::Chars::iterator::operator --();
			} while ((**this)->skip ((OpenTypeChar::LookupFlag) flags));
			return *this;
		}

		OpenTypeTextIterator operator -- (int n) {
			OpenTypeTextIterator i = *this;
			-- (*this);
			return i;
		}
	};
}

#endif // OPENTYPETEXT_H
