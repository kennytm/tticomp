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

#include <algorithm>
#include "../OTFont/OTGlyph.h"
#include "../OTFont/OTException.h"

#include "GlyphList.h"
#include "Identifier.h"

using std::vector;
using std::set_difference;
using std::back_inserter;
using std::endl;

void writeToStream (const GlyphIds & glyphIds, ostream & o, OpenTypeFont & font) {
	o << "[ ";
	for (GlyphIds::const_iterator i = glyphIds.begin(); i != glyphIds.end(); i ++) {
		o << font.getGlyph (*i)->getName() << " ";
	}
	o << "]";
}

/*** GlyphList ***/

GlyphList::GlyphList (Preprocessor & prep, OpenTypeFont & font, const Groups &groups) 
: pos (prep.getCurrentPosition()) {
	String buffer = prep.get();
	if (buffer == "[") {
		int errorNum = 0;
		do {
			PreprocessorPosition pos = prep.getCurrentPosition();
			buffer = prep.peek();
			if (buffer == "@") {
				prep.deleteToken();
				buffer = prep.get();
				Groups::const_iterator i = groups.lower_bound (buffer);
				if (i == groups.end() || i->first != buffer) {
					prep.startError (pos) << "Undefined group: \"@" << buffer << "\"." << endl;
					errorNum ++;
				} else
				{
					GlyphListPtr group = i->second->getGlyphs();
					buffer = prep.peek();
					if (buffer == '-')
					{	// Subtract list from group
						prep.deleteToken();
						GlyphListPtr list = new GlyphList (prep, font, groups);
						// group is already sorted, list is not yet.
						// set_difference requires two sorted lists so list should be sorted first.
						list->sort();
						set_difference (group->begin(), group->end(), list->begin(), list->end(),
							back_inserter (*this));
					} else
						// Just insert the whole group
						insert (end(), group->begin(), group->end());
				}
			} else {
				if (buffer == "[") {
					GlyphListPtr list1 = new GlyphList (prep, font, groups);
					buffer = prep.peek();
					if (buffer == '-') 
					{	// Subtract list2 from list1
						prep.deleteToken();
						GlyphListPtr list2 = new GlyphList (prep, font, groups);
						list1->sort();
						list2->sort();
						set_difference (list1->begin(), list1->end(), list2->begin(), list2->end(),
							back_inserter (*this));
					} else
						// Insert the whole list
						insert (end(), list1->begin(), list1->end());
				} else {
					String firstGlyphName = prep.get();
					try {
						GlyphId firstGlyph = font.getGlyphIndex (firstGlyphName, true);
						String buffer = prep.peek();
						if (buffer == '-') {
							prep.deleteToken();
							String lastGlyphName = prep.get();
							GlyphId lastGlyph = font.getGlyphIndex (lastGlyphName, true);
							if (firstGlyph <= lastGlyph) {
								for (GlyphId i = firstGlyph; i <= lastGlyph; i ++) {
									push_back (i);
								}
							} else {
								prep.startError (pos) << "[" << firstGlyphName << " - " <<
									lastGlyphName << "] is not a valid range." << endl;
								errorNum ++;
							}
						} else
							push_back (firstGlyph);
					} catch (Exception) {
						prep.startError (pos) << "Glyph \"" << firstGlyphName << "\" not found." << endl;
						errorNum ++;
					}
				}
			}
			buffer = prep.peek();
		} while (buffer != "]" && errorNum < 4);
		if (buffer == "]")
			prep.deleteToken();
	} else {
		if (buffer == "@") {
			buffer = prep.get();
			Groups::const_iterator i = groups.lower_bound (buffer);
			if (i == groups.end() || i->first != buffer)
				prep.startError (pos) << "Undefined group: \"@" << buffer << "\"." << endl;
			else {
				GlyphListPtr group = i->second->getGlyphs();
				std::copy (group->begin(), group->end(), back_inserter (*this));
			}
		} else {
			PreprocessorPosition pos = prep.getCurrentPosition();
			try {
				GlyphId glyphId = font.getGlyphIndex (buffer, true);
				push_back (glyphId);
			} catch (Exception) {
				prep.startError (pos) << "Glyph \"" << buffer << "\" not found." << endl;
			}
		}
	}
}

GlyphList::~GlyphList() {}

void GlyphList::sort() {
	std::sort (begin(), end());
}

void GlyphList::checkDuplicates (OpenTypeFont & font) {
	if (!empty()) {
		GlyphIds duplicates;
		GlyphId last = front();
		for (GlyphList::iterator i = begin() + 1; i != end();) {
			if (*i == last) {
				duplicates.push_back (last);
				i = erase (i);
			} else {
				last = *i;
				i ++;
			}
		}

		if (!duplicates.empty()) {
			ostream &o = pos.prep.startError (pos);
			o << "Duplicate glyphs found in range: ";
			writeToStream (duplicates, o, font);
			o << "." << endl;
		}
	}
}



/*** GlyphGroup ***/

GlyphGroup::GlyphGroup (Preprocessor & prep, OpenTypeFont & font, const Groups &groups) {
	String buffer = prep.get();
	assert (buffer == "group");
	if (!isExpected (prep, "@"))
		return;
	name = getIdentifier (prep);
	glyphs = new GlyphList (prep, font, groups);
	glyphs->sort();
	glyphs->checkDuplicates (font);

	buffer = prep.peek();
	if (buffer == "base")
		glyphClass = gcBaseGlyph;
	else if (buffer == "ligature")
		glyphClass = gcLigature;
	else if (buffer == "mark")
		glyphClass = gcMark;
	else if (buffer == "component")
		glyphClass = gcComponent;
	else {
		glyphClass = gcUndefined;
		isExpected (prep, ";");
		return;
	}

	prep.deleteToken();
	isExpected (prep, ";");
}

GlyphGroup::~GlyphGroup() {}

/*** Groups ***/

Groups::Groups() {}

Groups::~Groups() {}

UShort Groups::getMarkGroupIndex (String name, Preprocessor & prep) {
	for (MarkGroups::iterator i = markGroups.begin(); i != markGroups.end(); i ++) {
		// This has already been defined as a mark group.
		if ((*i)->getName() == name)
			// Counting starts at 1
			return i - markGroups.begin() + 1;
	}
	
	// Not found so add to the list
	if (markGroups.size() == 255) {
		prep.startError() << "No more than 255 mark groups may be used." << endl;
		return 0;
	}

	GlyphGroupPtr group;
	iterator g = lower_bound (name);
	if (g == end() || g->first != name) {
		prep.startError() << "Glyph group \"@" << name << "\" was not found." << endl;
		return 0;
	} else
		group = g->second;

	if (group->getGlyphClass() != gcMark) {
		prep.startError() << "Glyph group \"@" << name << "\" is not a mark group." << endl;
		return 0;
	}
	
	markGroups.push_back (group);
	// Counting starts at 1
	return markGroups.size();
}

ReferenceMemoryBlockPtr Groups::getMarkClassDef (OpenTypeFont &font) const {
	GlyphClasses markClasses (font.getGlyphNum(), gcUndefined);
	for (MarkGroups::const_iterator i = markGroups.begin(); i != markGroups.end(); i ++) {
		GlyphClass markClass = GlyphClass (i - markGroups.begin() + 1);
		for (GlyphList::const_iterator j = (*i)->getGlyphs()->begin(); j != (*i)->getGlyphs()->end(); j ++) {
			assert (*j < markClasses.size());
			assert (markClasses [*j] == gcUndefined);
			markClasses [*j] = markClass;
		}
	}

	return getClassDefTable (markClasses);
}

/*** getClassDefTable ***/

ReferenceMemoryBlockPtr getClassDefTable (const GlyphClasses & classes) {
	// The Class Definition Table comes in two different flavours.
	// Let's see which one gives the smallest file size
	// Glyphs in class 0 may be implicitly or explicitly in that class

	GlyphClasses::const_iterator i;
	ReferenceMemoryBlockPtr table = new ReferenceMemoryBlock();
	ReferenceMemoryPen pen (table);

	// For format 1
	GlyphClasses::const_iterator beginNonZero = classes.end();
	GlyphClasses::const_iterator endNonZero = classes.end();

	// For format 2
	int rangeNum = 0;

	UShort lastClass = 0;

	for (i = classes.begin(); i != classes.end(); i ++) {
		if (*i) {
			endNonZero = i+1;
			if (beginNonZero == classes.end())
				beginNonZero = i;

			if (*i != lastClass) {
				rangeNum ++;
				lastClass = *i;
			}
		} else
			lastClass = 0;
	}

	// Format 1 lists all glyph classes in a certain range:
	// [beginNonZero, endNonZero) in this case.
	// Format 2 lists ranges: rangeNum ranges.
	// See which of these two produces the smallest table size

	if (3 + (endNonZero - beginNonZero) < 2 + 3 * rangeNum)
	{	// Use table format 1
		pen.writeUShort (1);
		// Start glyph
		pen.writeUShort (beginNonZero - classes.begin());
		// Glyph count
		pen.writeUShort (endNonZero - beginNonZero);
		// Glyph classes
		for (i = beginNonZero; i != endNonZero; i ++)
			pen.writeUShort (*i);
	} else
	{
		// Format 2
		pen.writeUShort (2);
		pen.writeUShort (rangeNum);
		int curClass = 0;

		i = beginNonZero;
		while (i != endNonZero) {
			if (*i != 0) {
				curClass = *i;
				// Start index
				pen.writeUShort (i - classes.begin());
				GlyphClasses::const_iterator j = i;
				do {
					j++;
				} while (j != endNonZero && *j == curClass);
				// End index
				pen.writeUShort (j - classes.begin() - 1);
				// Class
				pen.writeUShort (curClass);

				curClass = *i;
				i = j;
			} else
				i ++;
		}
	}

	return table;
}
