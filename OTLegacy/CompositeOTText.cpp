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

#include "CompositeOTText.h"
#include "../OTFont/OpenTypeFont.h"
#include "../OTFont/OTLayoutTable.h"
#include "../OTFont/OTException.h"

using util::String;
using namespace OpenType;

/*** CompositeChar ***/

CompositeChar::CompositeChar (UShort aGlyphId, OpenTypeFont &aFont)
: OpenTypeChar (aGlyphId, aFont), thisAttachPoint (0xFFFF), baseAttachPoint (0xFFFF) {}

CompositeChar::~CompositeChar() {}

void CompositeChar::setFirstPoint (UShort &aFirstPoint) {
	firstPoint = aFirstPoint;
	aFirstPoint += font.getGlyph (glyphId)->getPointNum();
}

void CompositeChar::move (Short aX, Short aY,
							Short aXAdvance, Short aYAdvance) {
	thisAttachPoint = baseAttachPoint = 0xFFFF;
	OpenTypeChar::move (aX, aY, aXAdvance, aYAdvance);
}

void CompositeChar::attach (const Anchor &thisAnchor,
							  OpenTypeText::Chars::const_iterator base, const Anchor &baseAnchor) {
	OpenTypeChar::attach (thisAnchor, base, baseAnchor);
	thisAttachPoint = thisAnchor.getContourPoint();
	baseAttachPoint = baseAnchor.getContourPoint();
	if (thisAttachPoint == 0xFFFF || baseAttachPoint == 0xFFFF)
		thisAttachPoint = baseAttachPoint = 0xFFFF;
	else
		baseAttachPoint += ((const CompositeChar &)(**base)).firstPoint;
}

void CompositeChar::addToComponents (Components &components, HorMetric &hm, UShort totalAdvance) {
	// Current "pen" position is hm.advanceWidth
	ComponentPtr c;
	GlyphPtr glyph = font.getGlyph (glyphId);
	if (!glyph->isEmpty()) {
		CompositeComponent::Flags flags;
		if (totalAdvance && totalAdvance == advance && position.x == 0) {
			flags = CompositeComponent::Flags (
				CompositeComponent::cfRoundXYToGrid | CompositeComponent::cfUseMyMetrics);
		} else
			flags = CompositeComponent::cfRoundXYToGrid;

		CompositeComponent::Scale scale;// = {0x4000, 0, 0, 0x4000};
		scale.xx = scale.yy = 1;
		scale.xy = scale.yx = 0;
		if (thisAttachPoint == 0xFFFF) {
			CompositeComponent::Translation translation = {hm.advanceWidth -
				glyph->getDisplacement() + position.x, position.y};
			c = new PositionedCompositeComponent (font, glyphId, flags,
				scale, translation);
		} else {
			c = new AttachedCompositeComponent (font, glyphId, flags,
				scale, baseAttachPoint, thisAttachPoint);
		}
		components.push_back (c);
	}
	hm.advanceWidth += advance;
}

/*** CompositeText ***/

CompositeText::CompositeText (OpenTypeFont &aFont) : OpenTypeText (aFont) {}

CompositeText::~CompositeText() {}

OpenTypeCharPtr CompositeText::newOpenTypeChar (UShort glyphId) {
	return new CompositeChar (glyphId, font);
}

void CompositeText::beforePositioning() {
	// Set points
	UShort pointNum = 0;
	Chars::iterator c;
	for (c = chars.begin(); c != chars.end(); c ++) {
		((CompositeChar &)(**c)).setFirstPoint (pointNum);
	}
}

GlyphId CompositeText::addToFont(String name) {
	Exception::FontContext c1 (font);

	HorMetric hm;
	hm.advanceWidth = 0;
	hm.lsb = 0;
	Components components;
	Chars::iterator c;
	UShort totalAdvance = 0;
	for (c = chars.begin(); c != chars.end(); c ++) {
		totalAdvance += (*c)->getAdvance();
	}
	for (c = chars.begin(); c != chars.end(); c ++) {
		((CompositeChar &)(**c)).addToComponents (components, hm, totalAdvance);
	}
	GlyphPtr newGlyph;
	if (components.empty())
		newGlyph = new Glyph (font, name, hm);
	else {
		newGlyph = new CompositeGlyph (font, name, hm, components);
		hm.lsb = newGlyph->getDisplacement();
		newGlyph->setHorMetric (hm);
	}
	return font.addGlyph (newGlyph);
}
