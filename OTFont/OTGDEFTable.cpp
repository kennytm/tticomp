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
	\file Glyph definition table
*/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include "OTLayoutTable.h"
#include "OTLayoutInternal.h"
#include "OTException.h"
#include "OTTags.h"
#include "OpenTypeFile.h"

using util::String;

namespace OpenType {

GDEFTable::GDEFTable (OpenTypeFile &aFont, MemoryBlockPtr aMemory)
: Table (aFont) {
	memory = aMemory;
	MemoryPen start (memory);
	MemoryPen pen (memory);

	Fixed version = pen.readFixed();
	if (version != 0x00010000)
		throw Exception ("Unknown table version: " +
			String (version, 16));

	try {
		Offset glyphClassesOffset = pen.readOffset();
		if (glyphClassesOffset) {
			if (glyphClassesOffset < 12)
				font.addWarning (new Exception ("Curiously low Glyph Class Table offset: " +
					String (glyphClassesOffset)));
			glyphClasses = getClassDefTable (start + glyphClassesOffset);
			if (glyphClasses->getClassNum() > 5)
				throw Exception (
					"Invalid glyph class definition table: it defines more than 5 classes");
		}
	} catch (Exception &e) {
		e.addDescription ("Class definition table will be ignored");
		font.addWarning (new Exception (e));
		glyphClasses = NULL;
	}

	// AttachList
	pen.readOffset();
	// LigCaretList
	pen.readOffset();

	try {
		Offset markAttachOffset = pen.readOffset();
		if (markAttachOffset) {
			if (markAttachOffset < 12) {
				font.addWarning (new Exception ("Curiously low Mark Attachment Table offset: " +
					String (markAttachOffset)));
			}
			markAttachmentClasses = getClassDefTable (start + markAttachOffset);
		}
	} catch (Exception &e) {
		e.addDescription ("Mark attachment table will be ignored");
		font.addWarning (new Exception (e));
		glyphClasses = NULL;
	}
}

GDEFTable::~GDEFTable() {}

Tag GDEFTable::getTag() const {
	return GDEFTag;
}

MemoryBlockPtr GDEFTable::getMemory() const {
	return memory;
}

UShort GDEFTable::getGlyphClass (GlyphId glyphId) const {
	if (glyphClasses)
		return glyphClasses->getClass (glyphId);
	else
		return 0;
}

UShort GDEFTable::getMarkAttachmentClass (GlyphId glyphId) const {
	if (markAttachmentClasses)
		return markAttachmentClasses->getClass (glyphId);
	else
		return 0;
}

} // end namespace OpenType
