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

#include "OpenTypeText.h"
#include "OpenTypeFont.h"
#include "OTLayoutTable.h"
#include "OTGlyph.h"
#include "OTException.h"

using std::vector;
using util::smart_ptr;

namespace OpenType {

/*** OpenTypeText ***/

OpenTypeText::OpenTypeText (OpenTypeFont &aFont) : font (aFont) {}

void OpenTypeText::setUnicodes (const Unicodes unicodes) {
	Unicodes::const_iterator i;
	for (i = unicodes.begin(); i != unicodes.end(); i ++)
		chars.push_back (newOpenTypeChar (font.getGlyphIndexByUnicode(*i)));
}

void OpenTypeText::setGlyphs (const GlyphIds glyphs) {
	GlyphIds::const_iterator i;
	for (i = glyphs.begin(); i != glyphs.end(); i ++)
		chars.push_back (newOpenTypeChar (*i));
}

OpenTypeText::~OpenTypeText() {}

OpenTypeCharPtr OpenTypeText::newOpenTypeChar (UShort glyphId) {
	return new OpenTypeChar (glyphId, font);
}


void OpenTypeText::applyLookups (Tag script, Tag language, vector <Tag> features) {
	smart_ptr <GSUBTable> GSUB = font.getGSUBTable (false);
	if (GSUB)
		GSUB->apply (*this, script, language, features);

	beforePositioning();
	smart_ptr <GPOSTable> GPOS = font.getGPOSTable (false);
	if (GPOS)
		GPOS->apply (*this, script, language, features);
}

OpenTypeText::iterator OpenTypeText::begin (UShort flags) {
	Chars::iterator i = chars.begin();
	while (i != chars.end() && (*i)->skip (OpenTypeChar::LookupFlag (flags)))
		i++;
	return iterator (i, this, flags);
}

OpenTypeText::iterator OpenTypeText::end (UShort flags) {
	return iterator (chars.end(), this, flags);
}

OpenTypeText::iterator OpenTypeText::replace (iterator it, GlyphId glyphId) {
	(*it)->setGlyphId (glyphId);
	return it;
}

OpenTypeText::iterator OpenTypeText::replace (OpenTypeTextIterator begin, OpenTypeTextIterator end,
											  GlyphId glyphId)
{
	// Note that I use a Chars::iterator here: this one will not skip any glyphs.
	Chars::iterator i = end;
	i --;
	UShort moveBackIndex = 0;
	while (i != begin) {
		if ((*i)->skip (OpenTypeChar::LookupFlag (begin.flags))) {
			if ((*i)->getGlyphClass() == OpenTypeChar::gcMark)
				// This glyph is moved
				(*i)->moveForward (moveBackIndex);
		} else {
			// This glyph is being replaced
			i = chars.erase (i);
			moveBackIndex ++;
		}
		i --;
	}

	assert (i == begin);
	(*begin)->setGlyphId (glyphId);
	return begin;
}

OpenTypeText::iterator OpenTypeText::replace (OpenTypeTextIterator it, GlyphIds glyphIds) {
	UShort flags = it.flags;
	if (glyphIds.empty()) {
		return iterator (chars.erase (it), this, flags);
	} else {
		// First glyph
		Chars::iterator cur = it;
		(*cur)->setGlyphId (glyphIds.front());
		// Other glyphs
		GlyphIds::iterator glyphId = glyphIds.begin();
		glyphId ++;
		UShort appliesTo = 1;
		while (glyphId != glyphIds.end()) {
			while (true) {
				cur ++;
				if (cur != chars.end() && (*cur)->skip (OpenTypeChar::LookupFlag (flags)) &&
					(*cur)->getAppliesTo() >= appliesTo)
				{
					(*cur)->moveBack (appliesTo);
					cur ++;
				} else
					break;
			}

			cur = chars.insert (cur, newOpenTypeChar (*glyphId));

			glyphId ++;
			appliesTo ++;
		}
		return it;
	}
}


/*** OpenTypeChar ***/

OpenTypeChar::OpenTypeChar (UShort aGlyphId, OpenTypeFont &aFont)
: font (aFont) {
	setGlyphId (aGlyphId);
}

OpenTypeChar::~OpenTypeChar() {}

OpenTypeChar::Position OpenTypeChar::getLocalCoordinates (OpenTypeChar::Position global) const {
	// Simply return global position
	return global;
}

OpenTypeChar::Position OpenTypeChar::getLocalCoordinates (const Anchor &anchor) const {
	// Simply return global position
	Position pos = {anchor.getX(), anchor.getY()};
	return pos;
}

void OpenTypeChar::setGlyphId (GlyphId aGlyphId) {
	glyphId = aGlyphId;
	try {
		// Load glyph class
		glyphClass = (GlyphClass) font.getGlyphClass (glyphId);
		if (glyphClass == gcMark)
			markAttachmentClass = font.getMarkAttachmentClass (glyphId);
		else
			markAttachmentClass = 0;
	} catch (Exception & e) {
		// Exceptions here should not be fatal
		font.addWarning (new Exception (e));
		glyphClass = gcUndefined;
		markAttachmentClass = 0;
	}
	appliesTo = 0;
	position.x = 0;
	position.y = 0;
	advance = font.getGlyph (glyphId)->getHorMetric().advanceWidth;
}

OpenTypeChar::GlyphClass OpenTypeChar::getGlyphClass() const {
	return glyphClass;
}

bool OpenTypeChar::skip (LookupFlag flag) const {
	// Lookups should skip over any glyphs specified by glyph class
	switch (glyphClass) {

	case gcUndefined:
	case gcComponent:
		return false;

	case gcBaseGlyph:
		return (flag & lfIgnoreBaseGlyphs) != 0;

	case gcLigature:
		return (flag & lfIgnoreLigatures) != 0;

	case gcMark:
		if (flag & lfIgnoreMarks)
			return true;
		if (flag & lfMarkAttachmentType)
			// Skip over all marks except markAttachmentClass
			return (markAttachmentClass != UShort (flag >> 8));
		else
			return false;
	default:
		assert (false);
		return false;
	}
}

void OpenTypeChar::moveForward (UShort n) {
	if (glyphClass == gcMark)
		appliesTo += n;
}

void OpenTypeChar::moveBack (UShort n) {
	assert (appliesTo >= n);
	appliesTo -= n;
}

void OpenTypeChar::move (Short aX, Short aY,
						 Short aXAdvance, Short aYAdvance) {
	Position deltaPos = { aX, aY };
	position = getLocalCoordinates (deltaPos);
	Position deltaAdvance = { aXAdvance, aYAdvance };
	deltaAdvance = getLocalCoordinates (deltaAdvance);
	if (deltaAdvance.x < 0 && advance < -deltaAdvance.x)
		advance = 0;
	else
		advance += deltaAdvance.x;
	// We don't do anything with aYAdvance (yet)
}

UShort OpenTypeChar::getAdvance() const {
	return advance;
}

OpenTypeChar::Position OpenTypeChar::getPosition() const {
	return position;
}

void OpenTypeChar::attach (const Anchor &thisAnchor,
						   OpenTypeText::Chars::const_iterator base, const Anchor &baseAnchor) {
	Position basePos = (*base)->getLocalCoordinates (baseAnchor) + (*base)->getPosition();
	// Calculate local position from base coordinates
	do {
		basePos.x -= (*base)->getAdvance();
		base ++;
	} while ((*base) != this);
	Position thisPos = getLocalCoordinates (thisAnchor);
	position = basePos - thisPos;
}

OpenTypeChar::Position operator + (const OpenTypeChar::Position &pos1,
								   const OpenTypeChar::Position &pos2) {
	OpenTypeChar::Position pos = {pos1.x + pos2.x, pos1.y + pos2.y};
	return pos;
}

OpenTypeChar::Position operator - (const OpenTypeChar::Position &pos1,
								   const OpenTypeChar::Position &pos2) {
	OpenTypeChar::Position pos = {pos1.x - pos2.x, pos1.y - pos2.y};
	return pos;
}

} // namespace OpenType
