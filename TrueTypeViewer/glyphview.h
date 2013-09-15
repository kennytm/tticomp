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
	The GlyphView class shows one glyph at a certain PPEM.
*/

#ifndef GLYPHVIEW_H
#define GLYPHVIEW_H

#include <qwidget.h>
#include "fontcache.h"

using namespace OpenType;

class GlyphViewToolTip;

class GlyphView : public QWidget {
	Q_OBJECT

	typedef InstructionProcessor::Points Points;
protected:
	Points points;

	smart_ptr <RasterCache> rasterCache;
	smart_ptr <QImage> image;
	bool dirtyImage;
	bool numbers;
	smart_ptr <GlyphViewToolTip> toolTip;
	friend class GlyphViewToolTip;


	NewF26Dot6 gridSizeX, gridSizeY, originX, originY;
	double aspectRatio;
	bool subPixel;

public:
	GlyphView (QWidget *parent = 0, const char *name = 0);
	~GlyphView();

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void resizeEvent(QResizeEvent *);

public slots:
	virtual void setGlyph (const Points &points, double aAspectRatio);
	virtual void resetGlyphView();
	virtual void setSubPixel(bool aSubPixel);
	virtual void setNumbers(bool aShowNumbers);
};


#endif // GLYPHVIEW_H
