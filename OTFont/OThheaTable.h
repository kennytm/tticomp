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

#ifndef OTHHEATABLE_H
#define OTHHEATABLE_H

#include "OpenTypeFile.h"
#include "OTTable.h"

namespace OpenType {

	class Glyph;

	class hheaTable : public Table {
		// Typographic ascent. (Distance from baseline of highest ascender) 
		FWord ascender;
		// Typographic descent. (Distance from baseline of lowest descender)
		FWord descender;
		// Typographic line gap. Negative LineGap values are treated as zero in Windows 3.1, System 6, and System 7. 
		FWord lineGap;
		// Maximum advance width value in 'hmtx' table. 
		UFWord advanceWidthMax;
		// Minimum left sidebearing value in 'hmtx' table. 
		FWord minLeftSideBearing;
		// Minimum right sidebearing value; calculated as Min(aw - lsb - (xMax - xMin)). 
		FWord minRightSideBearing;
		// Max(lsb + (xMax - xMin)). 
		FWord xMaxExtent;
		// Used to calculate the slope of the cursor (rise/run); 1 for vertical. 
		Short caretSlopeRise;
		// 0 for vertical. 
		Short caretSlopeRun;
		// The amount by which a slanted highlight on a glyph needs to be shifted to produce the best appearance. Set to 0 for non-slanted fonts 
		Short caretOffset;
		// 0 for current format. 
		Short metricDataFormat;
		// Number of hMetric entries in 'hmtx' table 
		UShort hMetricsNum;

	protected:
		friend class hmtxTable;
		UShort getHMetricsNum();
		void setHMetricsNum (UShort aHMetricsNum);

		friend class OpenTypeFont;
		void reset (const std::vector <util::smart_ptr <Glyph> > &glyphs);

		friend class Glyph;
		void setHorizontalMaxima (UFWord aAdvanceWidthMax, FWord aMinLeftSideBearing,
			FWord aMinRightSideBearing, FWord aXMaxExtent);
		void setAdvanceWidthMax (UFWord aAdvanceWidthMax);

	public:
		hheaTable (OpenTypeFile &aFont, MemoryBlockPtr memory);
		virtual ~hheaTable();

		virtual MemoryBlockPtr getMemory() const;
		virtual Tag getTag() const;
	};
}

#endif // OTHHEATABLE_H
