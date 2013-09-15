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
	Preview.cpp contains the Preview class
*/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <qpainter.h>
#include <qimage.h>
#include "../OTFont/OpenTypeText.h"
#include "../OTFont/OTLayoutTable.h"
#include "../OTFont/OTTags.h"

#include "preview.h"
#include "messagedialog.h"

using namespace OpenType;

/*** InstructedChar ***/

class InstructedChar : public OpenTypeChar {
	smart_ptr <FontCache> fontCache;
	GlyphCachePtr glyph;
	UShort ppemx, ppemy, unitsPerEm;
protected:
	// Get local coordinates from global coordinates.
	virtual Position getLocalCoordinates (Position global) const;
	virtual Position getLocalCoordinates (const Anchor &anchor) const;
public:
	InstructedChar (UShort aGlyphId, OpenTypeFont &aFont, smart_ptr <FontCache> aFontCache,
		UShort appemx, UShort appemy, UShort aUnitsPerEm);
	virtual ~InstructedChar();

	// Find glyph
	virtual void setGlyphId (GlyphId aGlyphId);

	// Get local coordinates
	virtual UShort getAdvance() const;
	virtual Position getPosition () const;
};

/*** InstructedText ***/

class InstructedText : public OpenTypeText {
	smart_ptr <FontCache> fontCache;
	UShort ppemx, ppemy, unitsPerEm;
protected:
	virtual OpenTypeCharPtr newOpenTypeChar (UShort glyphId);
public:
	InstructedText (OpenTypeFont &aFont, smart_ptr <FontCache> aFontCache,
		UShort appemx, UShort appemy, UShort aUnitsPerEm) : OpenTypeText (aFont),
		fontCache (aFontCache), ppemx (appemx), ppemy (appemy), unitsPerEm (aUnitsPerEm) {}

	void paint (QImage &image, bool subPixel);
};

/*** InstructedText ***/

OpenTypeCharPtr InstructedText::newOpenTypeChar (UShort glyphId) {
	return new InstructedChar (glyphId, font, fontCache, ppemx, ppemy, unitsPerEm);
}

void InstructedText::paint (QImage &image, bool subPixel) {
	int xOffset = 0;
	int yOffset = image.height()*5/7;
	for (Chars::iterator i = chars.begin(); i != chars.end(); i ++) {
		/*** Calculate glyph position ***/
		GlyphCachePtr glyph = fontCache->getGlyph ((*i)->getGlyphId());
		OpenTypeChar::Position pos = (*i)->getPosition();
		int thisXOffset = xOffset + pos.x/64;
		int thisYOffset = yOffset - pos.y/64;

		/*** Draw glyph ***/
		if (subPixel)
			glyph->paintGlyphSubPixel(image, thisXOffset, thisYOffset);
		else
			glyph->paintGlyph(image, thisXOffset, thisYOffset);

		/*** Move pen for next glyph ***/
		xOffset += (*i)->getAdvance() / 64;
	}
}


/*** InstructedChar ***/

InstructedChar::InstructedChar (UShort aGlyphId, OpenTypeFont &aFont, smart_ptr <FontCache> aFontCache,
		UShort appemx, UShort appemy, UShort aUnitsPerEm)
		: OpenTypeChar (aGlyphId, aFont), fontCache (aFontCache), ppemx (appemx), ppemy (appemy),
		unitsPerEm (aUnitsPerEm) {
	setGlyphId (aGlyphId);
}

InstructedChar::~InstructedChar() {}

OpenTypeChar::Position InstructedChar::getLocalCoordinates (Position global) const {
	Position local;
	local.x = (F26Dot6) ((((LongLong) global.x) * 64 * ppemx) / unitsPerEm);
	local.y = (F26Dot6) ((((LongLong) global.y) * 64 * ppemx) / unitsPerEm);
	return local;
}

OpenTypeChar::Position InstructedChar::getLocalCoordinates (const Anchor &anchor) const {
	UShort contourPoint = anchor.getContourPoint();
	if (contourPoint == 0xFFFF) {
		Position local;
		local.x = (F26Dot6) ((((LongLong) anchor.getX()) * 64 * ppemx) / unitsPerEm);
		local.y = (F26Dot6) ((((LongLong) anchor.getY()) * 64 * ppemy) / unitsPerEm);
		return local;
	} else {
		const GlyphCache::Points &points = glyph->getPoints();
		if (contourPoint >= points.size())
			throw Exception ("Attachment contour point out of bounds");
		Position local;
		local.x = points [contourPoint].currentX.get_i();
		local.y = points [contourPoint].currentY.get_i();
		return local;
	}
}

void InstructedChar::setGlyphId (GlyphId aGlyphId) {
	OpenTypeChar::setGlyphId (aGlyphId);
	glyph = fontCache->getGlyph (glyphId);
	advance = glyph->getAdvance();
}

UShort InstructedChar::getAdvance() const {
	// round advance
	return (advance + 32) & ~63;
}

OpenTypeChar::Position InstructedChar::getPosition() const {
	// Round position
	Position local;
	local.x = (position.x + 32) & ~63;
	local.y = (position.y + 32) & ~63;
	return local;
}

/*** Preview ***/

Preview::Preview (QWidget *parent, const char *name)
: QWidget(parent, name, WResizeNoErase)
{
	setPalette( QPalette( QColor( 255, 255, 255) ) );
	subPixel = false;
	ppemx = ppemy = 0;

	newImage();
}

Preview::~Preview() {}

void Preview::setMessageDialog (MessageDialog *aMessageDialog) {
	messageDialog = aMessageDialog;
}

void Preview::newImage() {
	if (subPixel)
		image = new QImage(size(), 32, 256);
	else {
		image = new QImage(size(), 8, 256);

		int i;
		for (i=0; i < 256; i++) {
			image->setColor(i, qRgb(i, i, i));
		}
	}
	dirtyImage = true;
}

void Preview::resizeEvent(QResizeEvent *e) {
	e = e;
	newImage();
}

void Preview::paintEvent(QPaintEvent *event)
{
	event = event;
	QPainter p(this);
	unsigned int i;
	
	if (fontCache) {
		if (dirtyImage) {
			try {
				Exception::FontContext c1 (*font);
				Exception::Context c2 ("rendering preview");

				for (i=0; i<(unsigned int)height(); i++) {
					uchar *curPos = image->scanLine(i);
					memset(curPos, 255, image->bytesPerLine());
				}

				/*** OpenType aware implementation ***/
				{
					UShort unitsPerEm = font->getUnitsPerEm();

					vector <GlyphId> glyphIds;
					unsigned int curPos = 0;
					while (curPos < text.length()) {
						QChar c = text.at(curPos);
						switch (c) {
						case '/':
							// c=='/' so get postName, e.g., /alpha or /uni2365
							curPos++;
							if (curPos<text.length()) {
								c = text.at(curPos);
								if (c == '/') {
									glyphIds.push_back (font->getGlyphIndexByUnicode ('/'));
									curPos++;
								} else {
									QString postName;
									while (curPos<text.length() && ((c>='A' && c<='Z') || (c>='a' && c<='z')
										|| (c>='0' && c<='9') || c=='.')) {
										postName.append(c);
										curPos++;
										c = text.at(curPos);
									}

									if (postName.length() == 0)
										glyphIds.push_back (0);
									else
										glyphIds.push_back (font->getGlyphIndex (postName.latin1()));

									// Use spaces as delimiters so skip
									if (c==' ')
										curPos++;
								}
							} else
								glyphIds.push_back (0);
							break;

						case '\\':
							curPos ++;
							if (curPos < text.length()) {
								c = text.at (curPos);
								if (c == '\\') {
									glyphIds.push_back (font->getGlyphIndexByUnicode ('\\'));
									curPos ++;
								} else {
									QString code;
									while (curPos < text.length() &&
										((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')))
									{
										code.append (c);
										curPos ++;
										c = text.at (curPos);
									}

									if (code.length() == 0)
										glyphIds.push_back (0);
									else
										glyphIds.push_back (font->getGlyphIndexByUnicode (code.toInt (0, 16)));

									if (c == ' ')
										curPos ++;
								}
							} else
								glyphIds.push_back (0);
							break;

						default:
							glyphIds.push_back (font->getGlyphIndexByUnicode (c.unicode()));
							curPos++;
						}
						if (c!='/') {
						} else {
						}

					}

					smart_ptr <InstructedText> characters = new InstructedText (*font,
						fontCache, ppemx, ppemy, unitsPerEm);
					characters->setGlyphs (glyphIds);
					try {
						characters->applyLookups (scriptID, languageID, features.get_vector());
					} catch (Exception & e) {
						messageDialog->addMessage (e, true);
					}

					characters->paint (*image, subPixel);
				}
			} catch (Exception & e) {
				messageDialog->addMessage (e, true);
			}

			// Even if there was an error the preview should not be redrawn.
			dirtyImage = false;
		}
		p.drawImage(QPoint(0,0), *image);
	} else {
		p.eraseRect(0, 0, width(), height());
		p.drawText(0, height()-10, "No font loaded");
	}
}

void Preview::setText(const QString &newText) {
	text = newText;
	dirtyImage = true;
    repaint(false);
}

void Preview::setFontCache (smart_ptr <FontCache> aFontCache, smart_ptr <OpenTypeFont> aFont,
						   int aPPEMx, int aPPEMy, bool refresh) {
	fontCache = aFontCache;
	font = aFont;
	ppemx = aPPEMx;
	ppemy = aPPEMy;
	dirtyImage = true;
	if (refresh)
		repaint(false);
}

void Preview::setSubPixel(bool aSubPixel) {
	subPixel = aSubPixel;
	newImage();

	// No repainting because this is going to happen anyway as the ppemx value changes
}

void Preview::setFeatures(ULong aScriptID, ULong aLanguageID, util::shared_vector <Tag> aFeatures) {
	scriptID = aScriptID;
	languageID = aLanguageID;
	features = aFeatures;
	dirtyImage = true;
	repaint(false);
}
