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

#ifndef COMPOSITEOTTEXT_H
#define COMPOSITEOTTEXT_H

#include "../OTFont/OpenTypeText.h"
#include "../OTFont/OTGlyph.h"

/**
	CompositeChar represents an OpenType character that may be used as a 
	component of a composite character.
*/

class CompositeChar : public OpenType::OpenTypeChar {
	/// The contour point on this glyph that attached to the baseAttachPoint
	/// point on the base glyph.
	/// 0xFFFF if this is not attached to a point.
	OpenType::UShort thisAttachPoint;
	/// The contour point on the base glyph this glyph is attached to.
	/// 0xFFFF if this is not attached to a point.
	OpenType::UShort baseAttachPoint;
	/// The first point index from this component. It is used to determine
	/// the attachment point indices.
	OpenType::UShort firstPoint;
protected:
	friend class CompositeText;
	void setFirstPoint (OpenType::UShort &firstPoint);
public:
	CompositeChar (OpenType::UShort aGlyphId, OpenType::OpenTypeFont &aFont);
	virtual ~CompositeChar();

	virtual void attach (const OpenType::Anchor &thisAnchor,
		OpenType::OpenTypeText::Chars::const_iterator base, const OpenType::Anchor &baseAnchor);
	virtual void move (OpenType::Short aX, OpenType::Short aY,
		OpenType::Short aXAdvance, OpenType::Short aYAdvance);
	void addToComponents (OpenType::Components &components,
		OpenType::HorMetric &hm, OpenType::UShort totalAdvance);
};

/**
	CompositeText represents an OpenType text that can be added to a font,
	for example to provide procomposed characters.
*/

class CompositeText : public OpenType::OpenTypeText {
protected:
	virtual OpenType::OpenTypeCharPtr newOpenTypeChar (OpenType::UShort glyphId);
	virtual void beforePositioning();
public:
	CompositeText (OpenType::OpenTypeFont &aFont);
	virtual ~CompositeText();

	/// Add the text to the font as a composite glyph.
	OpenType::UShort addToFont (util::String name);
};

#endif	// COMPOSITEOTTEXT_H
