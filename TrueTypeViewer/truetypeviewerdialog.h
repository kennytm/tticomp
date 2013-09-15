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

#ifndef TRUETYPEVIEWERDIALOG_H
#define TRUETYPEVIEWERDIALOG_H

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <qlist.h>
#include "../Util/smart_ptr.h"
#include "../OTFont/OpenTypeFont.h"
#include "truetypeviewerdialogbase.h"
#include "glyphviewerdialog.h"

class FontCache;

class FeatureDialog;
class MessageDialog;

using namespace OpenType;

class TrueTypeViewerDialog : public TrueTypeViewerDialogBase
{
	Q_OBJECT

	smart_ptr <OpenTypeFont> font;
	smart_ptr <InstructionProcessor> proc;
	smart_ptr <FontCache> cache;
	QList <GlyphViewerDialog> subdialogs;

	// The featureDialog dialog always exists; only it isn't always visible.
	FeatureDialog *featureDialog;
	util::shared_vector <Tag> features;

	MessageDialog *messageDialog;

	void updatePPEM();
	bool updatingPointSize;

public:
	TrueTypeViewerDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags f = 0 );
	~TrueTypeViewerDialog();

public slots:
    virtual void browseForFileName();
	virtual void loadFont();
	virtual void setPPEMx(int aPPEMx);
	virtual void setPPEMy(int aPPEMy);
	virtual void setDPIx(int aDPIx);
	virtual void setDPIy(int aDPIy);
	virtual void setPointSize(int aPointSize);
	virtual void setSquare(bool aSquare);
	virtual void newGlyphWindow();
	virtual void newFeatureWindow();
	virtual void newMessageWindow();
	virtual void subdialogClosed(GlyphViewerDialog* dlg);
	virtual void featureDialogClosed();
	virtual void setSubPixel (bool subPixel);
	virtual void setFeatures (Tag aScriptID, Tag aLanguageID, const util::shared_vector <Tag> &aFeatures);
};

#endif	// TRUETYPEVIEWERDIALOG_H
