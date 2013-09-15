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

#ifndef GLYPHVIEWERDIALOG_H
#define GLYPHVIEWERDIALOG_H

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif


#include "glyphviewerdialogbase.h"
#include "glyphprocessor.h"
#include "stackviewerdialog.h"
#include "storageviewerdialog.h"
#include "cvtviewerdialog.h"
#include "graphicsstateviewerdialog.h"

class MessageDialog;
class GlyphViewerDialogExtension;

class GlyphViewerDialog : public GlyphViewerDialogBase
{
	Q_OBJECT

	smart_ptr <OpenTypeFont> font;
	smart_ptr <GlyphProcessor> proc;
	int ppemx, ppemy, pointSize;
	UShort glyphID;

	GlyphViewerDialogExtension * dialogExtension;
	bool extensionShown;

	MessageDialog *messageDialog;
	StackViewerDialog *stackViewer;
	StorageViewerDialog *storageViewer;
	CVTViewerDialog *cvtViewer;
	GraphicsStateViewerDialog *graphicsStateViewer;

	void appendToText (const InstructionProcessor::Instructions &instructions, QString &s);

public:
	GlyphViewerDialog (MessageDialog * aMessageDialog, QWidget* parent = 0);
	~GlyphViewerDialog();

public slots:
	virtual void setPPEM (int appemx, int appemy, int aPointSize);
	virtual void setFont (smart_ptr <OpenTypeFont> aFont, const InstructionProcessor & aProc);
	virtual void selectGlyph (int aGlyphId);
	virtual void resetGlyphView();
	virtual void setSubPixel (bool subPixel);
	virtual void showDebug (bool show);

	virtual void firstInstruction();
	virtual void backInstruction();
	virtual void previousInstruction();
	virtual void nextInstruction();
	virtual void forwardInstruction();
	virtual void lastInstruction();

	virtual void setCurrentInstructionText();

	virtual void openStackViewer();
	virtual void stackViewerClosed();
	virtual void openStorageViewer();
	virtual void storageViewerClosed();
	virtual void openCVTViewer();
	virtual void cvtViewerClosed();
	virtual void openGraphicsStateViewer();
	virtual void graphicsStateViewerClosed();
};

#endif // GLYPHVIEWERDIALOG_H
