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


#ifndef OTMAXPTABLE_H
#define OTMAXPTABLE_H

#include "OpenTypeFile.h"
#include "OTTable.h"

namespace OpenType {

	class Glyph;

	class maxpTable : public Table {
			// The number of glyphs in the font. 
		UShort glyphNum;
			// Maximum points in a non-composite glyph. 
		UShort maxPoints;
			// Maximum contours in a non-composite glyph. 
		UShort maxContours;
			// Maximum points in a composite glyph. 
		UShort maxCompositePoints;
			// Maximum contours in a composite glyph. 
		UShort maxCompositeContours;
			// 1 if instructions do not use the twilight zone (Z0), or 2 if instructions do use Z0; should be set to 2 in most cases. 
		UShort maxZones;
			// Maximum points used in Z0. 
		UShort maxTwilightPoints;
			// Number of Storage Area locations.  
		UShort maxStorage;
			// Number of FDEFs. 
		UShort maxFunctionDefs;
			// Number of IDEFs. 
		UShort maxInstructionDefs;
			// Maximum stack depth2. 
		UShort maxStackElements;
			// Maximum byte count for glyph instructions. 
		UShort maxSizeOfInstructions;
			// Maximum number of components referenced at "top level" for any composite glyph. 
		UShort maxComponentElements;
			// Maximum levels of recursion; 1 for simple components. 
		UShort maxComponentDepth;

	public:
		maxpTable (OpenTypeFile &aFont, MemoryBlockPtr aMemory);
		virtual ~maxpTable();

		virtual MemoryBlockPtr getMemory() const;
		virtual Tag getTag() const;

		// Setters and getters
	protected:
		friend class hmtxTable;
		friend class locaTable;
		UShort getGlyphNum();

		friend class OpenTypeFont;
		UShort getMaxStorage();
		UShort getMaxStackElements();
		UShort getMaxTwilightPoints();
		UShort getMaxFunctionDefs();

		void setMaxStorage (UShort aMaxStorage);
		void setMaxFunctionDefs (UShort aMaxFunctionDefs);
		void setMaxInstructionDefs (UShort aMaxInstructionDefs);
		void setMaxSizeOfInstructions (UShort aMaxSizeOfInstructions);
		void setMaxStackElements (UShort aMaxStackElements);
		void setMaxZones (UShort aMaxZones);
		void setMaxTwilightPoints (UShort aMaxTwilightPoints);

		void reset (const std::vector <util::smart_ptr <Glyph> > &glyphs);

		friend class Glyph;
		friend class SimpleGlyph;
		friend class CompositeGlyph;
		void setMaximumProfile (UShort aMaxPoints, UShort aMaxContours,
			UShort aMaxCompositePoints, UShort aMaxCompositeContours,
			UShort aMaxSizeOfInstructions, UShort aMaxComponentElements,
			UShort aMaxComponentDepth);
	};
}

#endif // OTMAXPTABLE_H
