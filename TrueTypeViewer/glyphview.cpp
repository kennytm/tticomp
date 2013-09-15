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
	The GlyphView class shows a magnified version of one glyph at a certain PPEM.
*/

#include "glyphview.h"
#include <qimage.h>
#include <qtooltip.h>
#include <qtextstream.h>

#define getPixelPos(a) (((a) + (1<<5))>>6)

// Bitmap digits

// These are larger bitmaps
#define digitHeight 7
#define digitWidth 5
const bool digits[10][digitHeight*digitWidth] = {
	{false,	true,	true,	true,	false,
	 true,	false,	false,	false,	true,
	 true,	false,	false,	false,	true,
	 true,	false,	false,	false,	true,
	 true,	false,	false,	false,	true,
	 true,	false,	false,	false,	true,
	 false,	true,	true,	true,	false},

	{false,	false,	true,	false,	false,
	 false,	true,	true,	false,	false,
	 false,	false,	true,	false,	false,
	 false,	false,	true,	false,	false,
	 false,	false,	true,	false,	false,
	 false,	false,	true,	false,	false,
	 false,	true,	true,	true,	false},

	{false,	true,	true,	true,	false,
	 true,	false,	false,	false,	true,
	 false,	false,	false,	false,	true,
	 false,	false,	false,	true,	false,
	 false,	false,	true,	false,	false,
	 false,	true,	false,	false,	false,
	 true,	true,	true,	true,	true},

	{false,	true,	true,	true,	false,
	 true,	false,	false,	false,	true,
	 false,	false,	false,	false,	true,
	 false,	true,	true,	true,	false,
	 false,	false,	false,	false,	true,
	 true,	false,	false,	false,	true,
	 false,	true,	true,	true,	false},

	{false,	false,	false,	true,	false,
	 false,	false,	true,	true,	false,
	 false,	true,	false,	true,	false,
	 false,	true,	false,	true,	false,
	 true,	false,	false,	true,	false,
	 true,	true,	true,	true,	true,
	 false,	false,	false,	true,	false},

	{false,	true,	true,	true,	true,
	 false,	true,	false,	false,	false,
	 true,	false,	false,	false,	false,
	 true,	true,	true,	true,	false,
	 false,	false,	false,	false,	true,
	 false,	false,	false,	false,	true,
	 true,	true,	true,	true,	false},

	{false,	false,	true,	true,	false,
	 false,	true,	false,	false,	false,
	 true,	false,	false,	false,	false,
	 true,	true,	true,	true,	false,
	 true,	false,	false,	false,	true,
	 true,	false,	false,	false,	true,
	 false,	true,	true,	true,	false},

	{true,	true,	true,	true,	true,
	 false,	false,	false,	false,	true,
	 false,	false,	false,	true,	false,
	 false,	false,	true,	false,	false,
	 false,	false,	true,	false,	false,
	 false,	true,	false,	false,	false,
	 false,	true,	false,	false,	false},

	{false,	true,	true,	true,	false,
	 true,	false,	false,	false,	true,
	 true,	false,	false,	false,	true,
	 false,	true,	true,	true,	false,
	 true,	false,	false,	false,	true,
	 true,	false,	false,	false,	true,
	 false,	true,	true,	true,	false},

	{false,	true,	true,	true,	false,
	 true,	false,	false,	false,	true,
	 true,	false,	false,	false,	true,
	 false,	true,	true,	true,	true,
	 false,	false,	false,	false,	true,
	 false,	false,	false,	true,	false,
	 false,	true,	true,	false,	false}
};

/*
	{false,	false,	false,	false,	false,
	 false,	false,	false,	false,	false,
	 false,	false,	false,	false,	false,
	 false,	false,	false,	false,	false,
	 false,	false,	false,	false,	false,
	 false,	false,	false,	false,	false,
	 false,	false,	false,	false,	false}
*/

// These are smaller bitmaps
/*#define digitHeight 5
#define digitWidth 4
const bool digits[10][digitHeight*digitWidth] = {
	//0
	{false,	true,	true,	false,
	 true,	false,	false,	true,
	 true,	false,	false,	true,
	 true,	false,	false,	true,
	 false,	true,	true,	false},

	 //1
	{false,	false,	true,	false,
	 false,	true,	true,	false,
	 false,	false,	true,	false,
	 false,	false,	true,	false,
	 false,	true,	true,	true},

	 //2
	{false,	true,	true,	false,
	 true,	false,	false,	true,
	 false,	false,	true,	false,
	 false,	true,	false,	false,
	 true,	true,	true,	true},

	 //3
	{true,	true,	true,	false,
	 false,	false,	false,	true,
	 false,	true,	true,	false,
	 false,	false,	false,	true,
	 true,	true,	true,	false},

	 //4
	{false,	false,	false,	true,
	 false,	false,	true,	true,
	 false,	true,	false,	true,
	 true,	true,	true,	true,
	 false,	false,	false,	true},

	 //5
	{true,	true,	true,	true,
	 true,	false,	false,	false,
	 true,	true,	true,	false,
	 false,	false,	false,	true,
	 true,	true,	true,	false},

	 //6
	{false,	true,	true,	false,
	 true,	false,	false,	false,
	 true,	true,	true,	false,
	 true,	false,	false,	true,
	 false,	true,	true,	false},

	 //7
	{true,	true,	true,	true,
	 false,	false,	false,	true,
	 false,	false,	true,	false,
	 false,	true,	false,	false,
	 false,	true,	false,	false},

	 //8
	{false,	true,	true,	false,
	 true,	false,	false,	true,
	 false,	true,	true,	false,
	 true,	false,	false,	true,
	 false,	true,	true,	false},

	 //9
	{false,	true,	true,	false,
	 true,	false,	false,	true,
	 false,	true,	true,	true,
	 false,	false,	false,	true,
	 false,	true,	true,	false},
};*/

/*
	{false,	false,	false,	false,
	 false,	false,	false,	false,
	 false,	false,	false,	false,
	 false,	false,	false,	false,
	 false,	false,	false,	false},
*/

class GlyphViewToolTip : public QToolTip {
public:
	GlyphViewToolTip(GlyphView *widget);
	virtual void maybeTip(const QPoint &point);
};

/*QString F26Dot6ToString (F26Dot6 i) {
	return QString ("%1").arg (double (i)/64, 0, 'f', 2);
}*/

QString F26Dot6ToString (NewF26Dot6 i) {
	return QString ("%1").arg (i.get(), 0, 'f', 2);
}

GlyphViewToolTip::GlyphViewToolTip(GlyphView *widget) : QToolTip(widget, 0) {}


void GlyphViewToolTip::maybeTip (const QPoint &point) {
	GlyphView* gv = ((GlyphView*)parentWidget());
	for (InstructionProcessor::Points::const_iterator i = gv->points.begin(); i != gv->points.end(); i++) {
		int x = (i->currentX * gv->gridSizeX + gv->originX).round();
		int y = gv->height() - (i->currentY * gv->gridSizeY + gv->originY).round();
		if (point.x() >= x-2 && point.x() <= x+2 &&
			point.y() >= y-2 && point.y() <= y+2)
		{
			QString str;
			QTextOStream stream(&str);
			if (i->onCurve)
				stream << "On curve point " << (i - gv->points.begin());
			else
				stream << "Off curve point " << (i - gv->points.begin());

			stream << "\n(";
			if (i->touchedX)
				stream << "<" << F26Dot6ToString(i->currentX) << ">";
			else
				stream << F26Dot6ToString(i->currentX);
			stream << ", ";

			if (i->touchedY)
				stream << "<" << F26Dot6ToString(i->currentY) << ">";
			else
				stream << F26Dot6ToString(i->currentY);
			stream << ")";

			if (i->currentX != i->originalX ||
				i->currentY != i->originalY) {
				stream << "\nOriginally (" <<
					F26Dot6ToString(i->originalX) << ", " <<
					F26Dot6ToString(i->originalY) << ")";
			}

			tip(QRect(point.x()-2, point.y()-2, 5, 5), str);
			return;
		}
	}
}

/*** GlyphView ***/

GlyphView::GlyphView(QWidget *parent, const char *name)
	: QWidget(parent, name, WRepaintNoErase | WResizeNoErase),
	image (new QImage(size(), 32))
{
	toolTip = new GlyphViewToolTip(this);
	dirtyImage = true;
	numbers = false;
	subPixel = 0;
}

GlyphView::~GlyphView() {
}

void GlyphView::resizeEvent(QResizeEvent *e) {
	e = e;
	image = new QImage(size(), 32);

	dirtyImage = true;
	if (rasterCache)
		resetGlyphView();
}

#define gridLineColour qRgb(192, 192, 216)
#define lightGridLineColour qRgb(216, 216, 240)
#define baseLineColour qRgb(64, 64, 128)
#define borderLineColour qRgb(64, 128, 255)

#define backgroundColour qRgb(255,255,255)
#define redBackgroundColour qRgb(255, 248, 248)
#define greenBackgroundColour qRgb(248, 255, 248)
#define blueBackgroundColour qRgb(248, 248, 255)

#define rectangleBorderColour qRgb(192, 0, 0)
#define rectangleFillRed 255
#define rectangleFillGreen 224
#define rectangleFillBlue 224
#define rectangleFillAlpha 192
#define rectangleTextDarkColour qRgb(192, 32, 32)
#define rectangleTextLightColour qRgb(255, 96, 96)

#define circleBorderColour qRgb(128, 64, 0)
#define circleFillRed 255
#define circleFillGreen 255
#define circleFillBlue 224
#define circleFillAlpha 192
#define circleTextDarkColour qRgb(192, 128, 0)
#define circleTextLightColour qRgb(255, 192, 64)


void GlyphView::paintEvent(QPaintEvent *) {
	QPainter p(this);
	if (dirtyImage) {
		int i;
		for (i=0; i<height(); i++) {
			uchar *curPos = image->scanLine(i);
			memset(curPos, 255, image->bytesPerLine());
		}

		if (rasterCache) {

			QRgb *curPos;

			// Draw vertical grid lines on first scanline

			curPos = (QRgb*) image->scanLine(0);
			int cur = 0;
			NewF26Dot6 nextLine = NewF26Dot6 (originX.get_i() % gridSizeX.get_i(), util::fixed_fraction());

			if (subPixel) {
				int curBgColour = ((unsigned int) 1-(originX / gridSizeX).get_integer()) % 3;
				while (cur < width()) {
					if (cur >= nextLine.round()) {
						if (curBgColour==2)
							*curPos = gridLineColour;
						else
							*curPos = lightGridLineColour;
						while (cur >= nextLine.round()) {
							nextLine += gridSizeX;
							curBgColour = (curBgColour+1) % 3;
						}
					}
					else {
						switch(curBgColour) {
						case 0:
							*curPos = redBackgroundColour;
							break;
						case 1:
							*curPos = greenBackgroundColour;
							break;
						case 2:
							*curPos = blueBackgroundColour;
							break;
						}
					}

					cur ++;
					curPos++;
				}
			} else {
				while (cur < width()) {
					if (cur >= nextLine.round()) {
						*curPos = gridLineColour;
						while (cur >= nextLine.round())
							nextLine += gridSizeX;
					}
					else {
						*curPos = backgroundColour;
					}
					cur ++;
					curPos++;
				}
			}

			// Draw left sidebearing
			image->setPixel (((points.end()-4)->currentX * gridSizeX + originX).round(),
				0, borderLineColour);

			// Draw right sidebearing
			image->setPixel (((points.end()-3)->currentX * gridSizeX + originX).round(),
				0, borderLineColour);

			// Copy to subsequent scanlines
			curPos = (QRgb*) image->scanLine(0);
			for (i=1; i<height(); i++)
				memcpy(image->scanLine(i), curPos, image->bytesPerLine());

			// Draw horizontal grid lines
			NewF26Dot6 cur2 = NewF26Dot6 (originY.get_i() % gridSizeY.get_i(), util::fixed_fraction());
			while (cur2 < NewF26Dot6 (height()) - NewF26Dot6 (.5)) {
				curPos = (QRgb*) image->scanLine (cur2.round());
				for (i=0; i < width(); i++) {
					*curPos = gridLineColour;
					curPos++;
				}
				cur2 += gridSizeY;
			}

			// Draw baseline
			curPos = (QRgb*) image->scanLine(height() - originY.round());
			for (i=0; i<width(); i++) {
				*curPos = baseLineColour;
				curPos++;
			}

			// Draw upper sidebearing
			curPos = (QRgb*) image->scanLine(height() -
				(((points.end() - 2)->currentY * gridSizeY) + originY).round());
			for (i=0; i<width(); i++) {
				*curPos = borderLineColour;
				curPos++;
			}

			// Draw lower sidebearing
			curPos = (QRgb*) image->scanLine(height() -
				(((points.end() - 1)->currentY * gridSizeY) + originY).round());
			for (i=0; i<width(); i++) {
				*curPos = borderLineColour;
				curPos++;
			}

			// Draw glyph
			rasterCache->paintGlyph32bpp(*image, 0, height());

			// Draw points
			for (Points::iterator point = points.begin(); point != points.end(); point ++) {
				int x = (point->currentX * gridSizeX + originX).round();
				int y = height() - (point->currentY * gridSizeY + originY).round();

				if (point->onCurve) {
					// Draw rectangle

					curPos = ((QRgb*) image->scanLine(y-2)) + x-2;
					*curPos = rectangleBorderColour;
					curPos++;
					*curPos = rectangleBorderColour;
					curPos++;
					*curPos = rectangleBorderColour;
					curPos++;
					*curPos = rectangleBorderColour;
					curPos++;
					*curPos = rectangleBorderColour;

					curPos = ((QRgb*) image->scanLine(y-1)) + x-2;
					*curPos = rectangleBorderColour;
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-rectangleFillAlpha) + rectangleFillRed * rectangleFillAlpha) / 255,
						(qGreen(*curPos) * (255-rectangleFillAlpha) + rectangleFillGreen * rectangleFillAlpha) / 255,
						(qBlue(*curPos) * (255-rectangleFillAlpha) + rectangleFillBlue * rectangleFillAlpha) / 255);
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-rectangleFillAlpha) + rectangleFillRed * rectangleFillAlpha) / 255,
						(qGreen(*curPos) * (255-rectangleFillAlpha) + rectangleFillGreen * rectangleFillAlpha) / 255,
						(qBlue(*curPos) * (255-rectangleFillAlpha) + rectangleFillBlue * rectangleFillAlpha) / 255);
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-rectangleFillAlpha) + rectangleFillRed * rectangleFillAlpha) / 255,
						(qGreen(*curPos) * (255-rectangleFillAlpha) + rectangleFillGreen * rectangleFillAlpha) / 255,
						(qBlue(*curPos) * (255-rectangleFillAlpha) + rectangleFillBlue * rectangleFillAlpha) / 255);
					curPos++;
					*curPos = rectangleBorderColour;

					curPos = ((QRgb*) image->scanLine(y-0)) + x-2;
					*curPos = rectangleBorderColour;
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-rectangleFillAlpha) + rectangleFillRed * rectangleFillAlpha) / 255,
						(qGreen(*curPos) * (255-rectangleFillAlpha) + rectangleFillGreen * rectangleFillAlpha) / 255,
						(qBlue(*curPos) * (255-rectangleFillAlpha) + rectangleFillBlue * rectangleFillAlpha) / 255);
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-rectangleFillAlpha) + rectangleFillRed * rectangleFillAlpha) / 255,
						(qGreen(*curPos) * (255-rectangleFillAlpha) + rectangleFillGreen * rectangleFillAlpha) / 255,
						(qBlue(*curPos) * (255-rectangleFillAlpha) + rectangleFillBlue * rectangleFillAlpha) / 255);
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-rectangleFillAlpha) + rectangleFillRed * rectangleFillAlpha) / 255,
						(qGreen(*curPos) * (255-rectangleFillAlpha) + rectangleFillGreen * rectangleFillAlpha) / 255,
						(qBlue(*curPos) * (255-rectangleFillAlpha) + rectangleFillBlue * rectangleFillAlpha) / 255);
					curPos++;
					*curPos = rectangleBorderColour;

					curPos = ((QRgb*) image->scanLine(y+1)) + x-2;
					*curPos = rectangleBorderColour;
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-rectangleFillAlpha) + rectangleFillRed * rectangleFillAlpha) / 255,
						(qGreen(*curPos) * (255-rectangleFillAlpha) + rectangleFillGreen * rectangleFillAlpha) / 255,
						(qBlue(*curPos) * (255-rectangleFillAlpha) + rectangleFillBlue * rectangleFillAlpha) / 255);
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-rectangleFillAlpha) + rectangleFillRed * rectangleFillAlpha) / 255,
						(qGreen(*curPos) * (255-rectangleFillAlpha) + rectangleFillGreen * rectangleFillAlpha) / 255,
						(qBlue(*curPos) * (255-rectangleFillAlpha) + rectangleFillBlue * rectangleFillAlpha) / 255);
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-rectangleFillAlpha) + rectangleFillRed * rectangleFillAlpha) / 255,
						(qGreen(*curPos) * (255-rectangleFillAlpha) + rectangleFillGreen * rectangleFillAlpha) / 255,
						(qBlue(*curPos) * (255-rectangleFillAlpha) + rectangleFillBlue * rectangleFillAlpha) / 255);
					curPos++;
					*curPos = rectangleBorderColour;

					curPos = ((QRgb*) image->scanLine(y+2)) + x-2;
					*curPos = rectangleBorderColour;
					curPos++;
					*curPos = rectangleBorderColour;
					curPos++;
					*curPos = rectangleBorderColour;
					curPos++;
					*curPos = rectangleBorderColour;
					curPos++;
					*curPos = rectangleBorderColour;
				} else {
					// Draw circle
					curPos = ((QRgb*) image->scanLine(y-2)) + x-2;
					//*curPos = qRgb(0, 0, 0);
					curPos++;
					//*curPos = qRgb(qRed(*curPos)/2, qGreen(*curPos)/2, qBlue(*curPos)/2);
					curPos++;
					*curPos = circleBorderColour;
					curPos++;
					//*curPos = qRgb(qRed(*curPos)/2, qGreen(*curPos)/2, qBlue(*curPos)/2);
					curPos++;
					//*curPos = qRgb(0, 0, 0);

					curPos = ((QRgb*) image->scanLine(y-1)) + x-2;
					//*curPos = qRgb(qRed(*curPos)/2, qGreen(*curPos)/2, qBlue(*curPos)/2);
					curPos++;
					*curPos = circleBorderColour;
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-circleFillAlpha) + circleFillRed * circleFillAlpha) / 255,
						(qGreen(*curPos) * (255-circleFillAlpha) + circleFillGreen * circleFillAlpha) / 255,
						(qBlue(*curPos) * (255-circleFillAlpha) + circleFillBlue * circleFillAlpha) / 255);
					curPos++;
					*curPos = circleBorderColour;
					curPos++;
					//*curPos = qRgb(qRed(*curPos)/2, qGreen(*curPos)/2, qBlue(*curPos)/2);

					curPos = ((QRgb*) image->scanLine(y-0)) + x-2;
					*curPos = circleBorderColour;
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-circleFillAlpha) + circleFillRed * circleFillAlpha) / 255,
						(qGreen(*curPos) * (255-circleFillAlpha) + circleFillGreen * circleFillAlpha) / 255,
						(qBlue(*curPos) * (255-circleFillAlpha) + circleFillBlue * circleFillAlpha) / 255);
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-circleFillAlpha) + circleFillRed * circleFillAlpha) / 255,
						(qGreen(*curPos) * (255-circleFillAlpha) + circleFillGreen * circleFillAlpha) / 255,
						(qBlue(*curPos) * (255-circleFillAlpha) + circleFillBlue * circleFillAlpha) / 255);
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-circleFillAlpha) + circleFillRed * circleFillAlpha) / 255,
						(qGreen(*curPos) * (255-circleFillAlpha) + circleFillGreen * circleFillAlpha) / 255,
						(qBlue(*curPos) * (255-circleFillAlpha) + circleFillBlue * circleFillAlpha) / 255);
					curPos++;
					*curPos = circleBorderColour;

					curPos = ((QRgb*) image->scanLine(y+1)) + x-2;
					//*curPos = qRgb(qRed(*curPos)/2, qGreen(*curPos)/2, qBlue(*curPos)/2);
					curPos++;
					*curPos = circleBorderColour;
					curPos++;
					*curPos = qRgb((qRed(*curPos) * (255-circleFillAlpha) + circleFillRed * circleFillAlpha) / 255,
						(qGreen(*curPos) * (255-circleFillAlpha) + circleFillGreen * circleFillAlpha) / 255,
						(qBlue(*curPos) * (255-circleFillAlpha) + circleFillBlue * circleFillAlpha) / 255);
					curPos++;
					*curPos = circleBorderColour;
					curPos++;
					//*curPos = qRgb(qRed(*curPos)/2, qGreen(*curPos)/2, qBlue(*curPos)/2);

					curPos = ((QRgb*) image->scanLine(y+2)) + x-2;
					//*curPos = qRgb(0, 0, 0);
					curPos++;
					//*curPos = qRgb(qRed(*curPos)/2, qGreen(*curPos)/2, qBlue(*curPos)/2);
					curPos++;
					*curPos = circleBorderColour;
					curPos++;
					//*curPos = qRgb(qRed(*curPos)/2, qGreen(*curPos)/2, qBlue(*curPos)/2);
					curPos++;
					//*curPos = qRgb(0, 0, 0);
				}

				if (numbers) {
					int horPos = x+3;
					int currentNum = point - points.begin();
					int currentDigit;
					int currentMask = 10000000;
					while (currentNum < currentMask && currentMask>1)
						currentMask /= 10;

					do {
						currentDigit = currentNum/currentMask;
						QRgb lightColour, darkColour;
						if (point->onCurve) {
							darkColour = rectangleTextDarkColour;
							lightColour = rectangleTextLightColour;
						} else {
							darkColour = circleTextDarkColour;
							lightColour = circleTextLightColour;
						}
						int j,k;
						for (j=0; j<digitHeight && y+j+1 < height(); j++) {
							curPos = ((QRgb*) image->scanLine(y+j+1))+horPos;
							for (k=0; k<digitWidth && horPos+k < width(); k++) {
								if (digits[currentDigit][j*digitWidth+k]) {
									if (qRed(*curPos) + qGreen(*curPos) + qBlue(*curPos) > 200)
										*curPos = darkColour;
									else
										*curPos = lightColour;
								}
								curPos++;
							}
						}

						currentNum %= currentMask;
						currentMask /= 10;
						horPos += digitWidth+1;
					} while (currentMask);
				}
			}
		}
		dirtyImage = false;
	}

	p.drawImage(0, 0, *image);
	/*if (numbers) {
		int i;
		for (i=0; i<pointNum; i++) {
			int x = getPixelPos(OT_Mult(points[i].currentX, gridSizeX) + originX);
			int y = height() - getPixelPos(OT_Mult(points[i].currentY, gridSizeY) + originY);
			p.drawText(x, y, QString("%1").arg(i));
		}
	}*/
}

#define glyphViewBorder 6
#define minimumPixelView 3

void GlyphView::setGlyph (const Points &aPoints, double aAspectRatio) {
	points = aPoints;
	aspectRatio = aAspectRatio;
	resetGlyphView();
}

void GlyphView::resetGlyphView() {
	// Enlarge glyph to fit canvas

	assert (!points.empty());

	NewF26Dot6 minX, maxX, minY, maxY;
	minX = maxX = points.front().currentX;
	minY = maxY = points.front().currentY;

	Points::iterator i;
	for (i = points.begin() + 1; i != points.end(); i++) {
		if (i->currentX > maxX)
			maxX = i->currentX;
		if (i->currentX < minX) 
			minX = i->currentX;
		if (i->currentY > maxY)
			maxY = i->currentY;
		if (i->currentY < minY)
			minY = i->currentY;
	}
	// Round down
	minX = minX.get_floor();
	minY = minY.get_floor();

	// Round up
	/*if (maxX.get_fraction() != 0)
		maxX = maxX.get_floor() + 1;
	if (maxY.get_fraction() != 0)
		maxY = maxY.get_floor() + 1;*/
	maxX = maxX.get_ceiling();
	maxY = maxY.get_ceiling();

	NewF26Dot6 pixelNumX = maxX - minX;
	NewF26Dot6 pixelNumY = maxY - minY;
	if (pixelNumX < minimumPixelView)
		pixelNumX = minimumPixelView;
	if (pixelNumY < minimumPixelView)
		pixelNumY = minimumPixelView;

	int gridWidth = width() - 2*glyphViewBorder;
	int gridHeight = height() - 2*glyphViewBorder;

	NewF26Dot6 maxGridSizeX = gridWidth / pixelNumX;
	NewF26Dot6 maxGridSizeY = gridHeight / pixelNumY;
	if (maxGridSizeX < maxGridSizeY * aspectRatio) {
		gridSizeX = maxGridSizeX;
		gridSizeY = NewF26Dot6 (maxGridSizeX / aspectRatio);
	}
	else {
		gridSizeX = NewF26Dot6 (maxGridSizeY * aspectRatio);
		gridSizeY = maxGridSizeY;
	}
	
	originX = glyphViewBorder + (gridWidth - pixelNumX * gridSizeX)/2 - minX * gridSizeX;
	originY = glyphViewBorder + (gridHeight - pixelNumY * gridSizeY)/2 - minY * gridSizeY;

	Points largePoints;
	largePoints.reserve (points.size());

	for (i = points.begin(); i != points.end(); i ++) {
		InstructionProcessor::GridFittedPoint largePoint;
		largePoint.currentX = i->currentX * gridSizeX + originX;
		largePoint.currentY = i->currentY * gridSizeY + originY;
		largePoint.onCurve = i->onCurve;
		largePoint.lastContourPoint = i->lastContourPoint;

		largePoints.push_back (largePoint);
	}

	rasterCache = new RasterCache (largePoints);

	dirtyImage = true;
	repaint(false);
}

void GlyphView::setSubPixel(bool aSubPixel) {
	subPixel = aSubPixel;

	// No repainting needed as the ppemx value will be changed anyway
}

void GlyphView::setNumbers(bool aShowNumbers) {
	numbers = aShowNumbers;
	dirtyImage = true;
	repaint(false);
}
