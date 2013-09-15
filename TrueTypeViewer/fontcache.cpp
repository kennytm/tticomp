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
	\file FontCache caches instructed glyph points for a certain ppem value as
	well as raster representations for those glyphs.
*/

#include <cassert>
#include <qglobal.h>
#include <qimage.h>
#include "fontcache.h"
#include "messagedialog.h"

/*** MessageInstructionProcessor ***/

void MessageInstructionProcessor::addWarning (InstructionExceptionPtr newWarning) {
	messageDialog->addMessage (*newWarning, false);
}


/*** RasterCache ***/

RasterCache::RasterCache (const InstructionProcessor::Points &points)
{
	FT_Raster raster;

	// Raster cache
	ft_grays_raster.raster_new (NULL, &raster);
	void *pool = malloc (poolSize);
	ft_grays_raster.raster_reset (raster, (unsigned char*) pool, poolSize);

	// Render the font using the FreeType greyscale renderer

	vector <FT_Vector> ft_points;
	vector <char> tags;
	vector <short> contours;

	for (InstructionProcessor::Points::const_iterator i = points.begin(); i != points.end(); i++) {
		FT_Vector v;
		v.x = i->currentX.get_i();
		v.y = i->currentY.get_i();
		ft_points.push_back (v);
		if (i->onCurve)
			tags.push_back (1);
		else
			tags.push_back (0);
		if (i->lastContourPoint)
			contours.push_back (i - points.begin());
	}

	FT_Outline outline = {contours.size(), points.size(), &*ft_points.begin(),
		&*tags.begin(), &*contours.begin(), 0};

	FT_Raster_Params params = {NULL, &outline, ft_raster_flag_aa | ft_raster_flag_direct, 
		&rasterCallback, NULL, NULL, NULL, this, {0,0,0,0}};


	ft_grays_raster.raster_render(raster, &params);

	free(pool);
}

void rasterCallback(int y, int count, FT_Span* spans, void* user) {
	RasterCache *cache = (RasterCache*) user;
	if (count!=0) {
		RasterCacheSpan newSpan;

		newSpan.y = y;
		newSpan.spans = vector <FT_Span> (spans, spans + count);

		cache->spans.push_back (newSpan);
	}
}

RasterCache::~RasterCache() {}

void RasterCache::paintGlyph8bpp(QImage &image, int xOffset, int yOffset) const {
	for (RasterCacheSpans::const_iterator span = spans.begin(); span != spans.end(); span ++) {
		if (yOffset >= span->y && yOffset - span->y < image.height()) {
			uchar *scanLine = image.scanLine(yOffset - span->y);
			uchar *endLine = scanLine + image.width();
			if (scanLine && scanLine!=(uchar*)-1) {
				RasterCacheSpan::Spans::const_iterator curSpan = span->spans.begin();
				uchar *curPos = scanLine + curSpan->x + xOffset;
				while (curSpan != span->spans.end() && curPos + curSpan->len < scanLine)
				{	// while span is fully hidden
					curSpan++;
					curPos = scanLine + curSpan->x + xOffset;
				}
				if (curSpan != span->spans.end() && curPos < scanLine)
				{	// span is partially hidden
					curPos = scanLine;
					uchar *endSpan = scanLine + curSpan->x + xOffset + curSpan->len;
					while (curPos<endSpan && curPos < endLine) {
						*curPos = ((*curPos) * (255 - curSpan->coverage)) / 255;
						curPos++;
					}
					curSpan++;
				}
				while (curSpan != span->spans.end() && curPos < endLine) {
					curPos = scanLine + curSpan->x + xOffset;
					for (int i=0; i<curSpan->len && curPos < endLine; i++) {
						*curPos = ((*curPos) * (255 - curSpan->coverage)) / 255;
						curPos++;
					}
					curSpan ++;
				}
			}
		}
	}
}

void RasterCache::paintGlyph32bpp(QImage &image, int xOffset, int yOffset) const {
	for (RasterCacheSpans::const_iterator span = spans.begin(); span != spans.end(); span ++) {
		if (yOffset >= span->y && yOffset - span->y < image.height()) {
			QRgb *scanLine = (QRgb *) image.scanLine (yOffset - span->y);
			QRgb *endLine = scanLine + image.width();
			if (scanLine && scanLine != (QRgb *)-1) {
				RasterCacheSpan::Spans::const_iterator curSpan = span->spans.begin();
				QRgb *curPos = scanLine + curSpan->x + xOffset;
				while (curSpan != span->spans.end() && curPos + curSpan->len < scanLine)
				{	// while span is fully hidden
					curSpan++;
					curPos = scanLine + curSpan->x + xOffset;
				}
				if (curSpan != span->spans.end() && curPos < scanLine)
				{	// span is partially hidden
					curPos = scanLine;
					QRgb *endSpan = scanLine + curSpan->x + xOffset + curSpan->len;
					while (curPos<endSpan && curPos < endLine) {
						int factor = 255 - curSpan->coverage;
						*curPos = qRgb((qRed(*curPos) * factor) / 255,
							(qGreen(*curPos) * factor) / 255,
							(qBlue(*curPos) * factor) / 255);
						curPos++;
					}
					curSpan++;
				}
				while (curSpan != span->spans.end() && curPos < endLine) {
					curPos = scanLine + curSpan->x + xOffset;
					for (int i=0; i<curSpan->len && curPos < endLine; i++) {
						int factor = 255 - curSpan->coverage;
						*curPos = qRgb((qRed(*curPos) * factor) / 255,
							(qGreen(*curPos) * factor) / 255,
							(qBlue(*curPos) * factor) / 255);
						curPos++;
					}
					curSpan ++;
				}
			}
		}
	}
}

void RasterCache::paintGlyphSubPixels(QImage &image, int xOffset, int yOffset) const {
	for (RasterCacheSpans::const_iterator span = spans.begin(); span != spans.end(); span ++) {
		if (yOffset >= span->y) {
			QRgb *scanLine = (QRgb*) image.scanLine(yOffset - span->y);
			QRgb *endLine = scanLine + image.width();
			if (scanLine) {
				RasterCacheSpan::Spans::const_iterator curSpan = span->spans.begin();
				QRgb *curPos = scanLine + (curSpan->x + xOffset)/3;
				while (curSpan != span->spans.end() && curPos < endLine) {
					curPos = scanLine + (curSpan->x + xOffset)/3;
					int subPixelPos = (curSpan->x + xOffset)%3;
					for (int i=0; i<curSpan->len && curPos < endLine; i++) {
						int factor = 255 - curSpan->coverage;

						switch(subPixelPos) {
						case 0:
							*curPos = qRgb((qRed(*curPos) * factor) / 255,
								qGreen(*curPos),qBlue(*curPos));
							break;
						case 1:
							*curPos = qRgb(qRed(*curPos),
								(qGreen(*curPos) * factor) / 255,
								qBlue(*curPos));
							break;
						case 2:
							*curPos = qRgb(qRed(*curPos), qGreen(*curPos),
								(qBlue(*curPos) * factor) / 255);
							break;
						}

						subPixelPos++;
						if (subPixelPos==3) {
							subPixelPos = 0;
							curPos++;
						}
					}
					curSpan ++;
				}
			}
		}
	}
}


/*** GlyphCache ***/


GlyphCache::GlyphCache (smart_ptr <InstructionProcessor> aProc, GlyphId aGlyphId,
						MessageDialog *messageDialog) : glyphId (aGlyphId)
{
	try {
		points = aProc->getGlyphPoints (aGlyphId);

		// Translate so that lsb point is at x=0
		Points::iterator lsb = points.end() - 4;
		NewF26Dot6 translation = lsb->currentX.round();
		for (Points::iterator i = points.begin(); i != points.end(); i++)
			i->currentX -= translation;

		raster = new RasterCache (points);
	} catch (Exception &e) {
		messageDialog->addMessage (e, true);
	}
}

GlyphCache::~GlyphCache() {}

const GlyphCache::Points & GlyphCache::getPoints() const {
	return points;
}

void GlyphCache::paintGlyph (QImage &image, int xOffset, int yOffset) const {
	if (raster)
		raster->paintGlyph8bpp(image, xOffset, yOffset);
}

void GlyphCache::paintGlyphSubPixel (QImage &image, int xOffset, int yOffset) const {
	if (raster)
		raster->paintGlyphSubPixels(image, xOffset, yOffset);
}


int GlyphCache::getAdvance() const {
	if (points.empty())
		return 0;
	else
		return ((points.end() - 3)->currentX - (points.end() - 4)->currentX).get_i();
}

/*** FontCache ***/

FontCache::FontCache (smart_ptr <OpenTypeFont> aFont, smart_ptr <InstructionProcessor> aProc,
					  UShort aPPEMx, UShort aPPEMy, UShort aPointSize, MessageDialog *aMessageDialog)
					  : ppemx (aPPEMx), ppemy (aPPEMy), pointSize (aPointSize),
					  messageDialog (aMessageDialog)
{
	setFont (aFont, aProc);
}

FontCache::~FontCache() {}

void FontCache::newPPEM() {
	glyphs.clear();
	if (proc) {
		try {
			proc->setPPEM(ppemx, ppemy, pointSize);
		} catch (Exception & e) {
			messageDialog->addMessage (e, true);
		}
	}
}

void FontCache::setFont (smart_ptr <OpenTypeFont> aFont, smart_ptr <InstructionProcessor> aProc) {
	try {
		font = aFont;
		proc = aProc;
	} catch (Exception & e) {
		messageDialog->addMessage (e, true);
	}

	newPPEM();
}

void FontCache::setPPEM (ULong appemx, ULong appemy, ULong aPointSize) {
	ppemx = appemx;
	ppemy = appemy;
	pointSize = aPointSize;
	newPPEM();
}

GlyphCachePtr FontCache::getGlyph (GlyphId index) {
	GlyphCaches::iterator lower, upper, guess;
	lower = glyphs.begin();
	upper = glyphs.end();
	while (lower != upper) {
		guess = lower + (upper - lower) / 2;
		if ((*guess)->getGlyphId() < index)
			lower = guess + 1;
		else {
			if (index < (*guess)->getGlyphId())
				upper = guess;
			else
				return *guess;
		}
	}

	GlyphCachePtr newGlyph = new GlyphCache (proc, index, messageDialog);
	glyphs.insert (lower, newGlyph);
	return newGlyph;
}

