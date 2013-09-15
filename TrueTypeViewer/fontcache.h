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

	In addition, as a special exception, Rogier van Dalen gives permission
	to link the code of this program with Qt non-commercial edition (or with
	modified versions of Qt non-commercial edition that use the	same license
	as Qt non-commercial edition), and distribute linked combinations
	including the two.  You must obey the GNU General Public License in all
	respects for all of the code used other than Qt non-commercial edition.
	If you modify this file, you may extend this exception to your version of
	the file, but you are not obligated to do so.  If you do not wish to do
	so, delete this exception statement from your version.
*/

/**
	FontCache caches instructed glyph points for a certain	ppem value as
	well as raster representations for those glyphs.
*/

#ifndef FONTCACHE_H
#define FONTCACHE_H

#include "../OTFont/OpenTypeFont.h"
#include "../InstructionProcessor/InstructionProcessor.h"
#include "../OTFont/OTGlyph.h"
#include "ftgrays.h"
#include <qpainter.h>
#include <vector>

using std::vector;
using util::smart_ptr;
using namespace OpenType;

class MessageDialog;

/*** MessageInstructionProcessor ***/

class MessageInstructionProcessor : public InstructionProcessor {
protected:
	MessageDialog *messageDialog;
	virtual void addWarning (InstructionExceptionPtr newWarning);
public:
	MessageInstructionProcessor (MessageDialog *aMessageDialog)
		: messageDialog (aMessageDialog) {}
};

/*** RasterCache ***/

typedef struct _RasterCacheSpan {
	short y;
	typedef vector <FT_Span> Spans;
	Spans spans;
} RasterCacheSpan;

#define poolSize 4096


class RasterCache {
protected:
	typedef vector <RasterCacheSpan> RasterCacheSpans;
	RasterCacheSpans spans;
	friend void rasterCallback(int y, int count, FT_Span*  spans, void* user);

public:
	RasterCache (const InstructionProcessor::Points &points);
	virtual ~RasterCache();
	void paintGlyph8bpp (QImage &image, int xOffset, int yOffset) const;
	void paintGlyph32bpp (QImage &image, int xOffset, int yOffset) const;
	void paintGlyphSubPixels (QImage &image, int xOffset, int yOffset) const;
};


class GlyphCache {
public:
	typedef InstructionProcessor::Points Points;
private:
	Points points;
	GlyphId glyphId;
	smart_ptr <RasterCache> raster;

public:
	GlyphCache (smart_ptr <InstructionProcessor> aProc, GlyphId aIndex, MessageDialog *messageDialog);
	virtual ~GlyphCache();

	const Points &getPoints() const;

	void paintGlyph (QImage &image, int xOffset, int yOffset) const;
	void paintGlyphSubPixel (QImage &image, int xOffset, int yOffset) const;

	GlyphId getGlyphId() const { return glyphId; }
	int getAdvance() const;
};

typedef smart_ptr <GlyphCache> GlyphCachePtr;
typedef vector <GlyphCachePtr> GlyphCaches;

class FontCache {
	MessageDialog *messageDialog;
	smart_ptr <OpenTypeFont> font;
	smart_ptr <InstructionProcessor> proc;
	ULong ppemx, ppemy, pointSize;
	GlyphCaches glyphs;

	void newPPEM();

public:
	FontCache (smart_ptr <OpenTypeFont> aFont, smart_ptr <InstructionProcessor> aProc,
		UShort aPPEMx, UShort aPPEMy, UShort aPointSize, MessageDialog *aMessageDialog);
	virtual ~FontCache();
	void setPPEM (ULong appemx, ULong appemy, ULong aPointSize);
	void setFont (smart_ptr <OpenTypeFont> aFont, smart_ptr <InstructionProcessor> aProc);

	GlyphCachePtr getGlyph (GlyphId index);
};

#endif // FONTCACHE_H
