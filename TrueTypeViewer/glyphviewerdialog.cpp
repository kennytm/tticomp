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

#include <qcombobox.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qtextview.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtextstream.h>
#include <qapplication.h>

#include <cassert>

#include "../OTFont/OpenTypeFont.h"
#include "glyphviewerdialog.h"
#include "truetypeviewerdialog.h"
#include "glyphview.h"
#include "messagedialog.h"
#include "glyphviewerdialogextension.h"

using util::String;

GlyphViewerDialog::GlyphViewerDialog (MessageDialog *aMessageDialog, QWidget* parent)
: GlyphViewerDialogBase( parent, NULL, false, WDestructiveClose ),
glyphID (0), messageDialog (aMessageDialog), stackViewer (NULL), storageViewer (NULL),
cvtViewer (NULL), graphicsStateViewer (NULL)
{
	setOrientation (Vertical);
	dialogExtension = new GlyphViewerDialogExtension (this);
	setExtension (dialogExtension);
	extensionShown = false;
	setCaption ("Glyph Viewer");
}

GlyphViewerDialog::~GlyphViewerDialog() {
	((TrueTypeViewerDialog *) parent())->subdialogClosed(this);
	if (stackViewer) {
		stackViewer->close();
	}
	if (storageViewer) {
		storageViewer->close();
	}
	if (cvtViewer) {
		cvtViewer->close();
	}
	if (graphicsStateViewer) {
		graphicsStateViewer->close();
	}
}


void GlyphViewerDialog::setPPEM (int appemx, int appemy, int aPointSize) {
	try {
		ppemx = appemx;
		ppemy = appemy;
		pointSize = aPointSize;
		if (font) {
			proc->setPPEM (ppemx, ppemy, pointSize);
			resetGlyphView();
		}
	} catch (Exception & e) {
		messageDialog->addMessage (e, true);
	}
}

void GlyphViewerDialog::setFont (smart_ptr <OpenTypeFont> aFont, const InstructionProcessor & aProc) {
    QApplication::setOverrideCursor( Qt::waitCursor );
	font = aFont;

	try {
		if (!proc)
			proc = new GlyphProcessor (messageDialog);
		proc->setFont (aProc);
		proc->setPPEM (ppemx, ppemy, pointSize);
	} catch (Exception & e) {
		messageDialog->addMessage (e, true);

	}

	// Fill postscript name combo box

	UShort i;
	glyphID = 0;
	QString oldPostscriptName = comboPostscriptNames->currentText();
	comboPostscriptNames->clear();
	UShort glyphNum = font->getGlyphNum();

	for (i = 0; i < glyphNum; i++) {
		QString string = font->getGlyph (i)->getName().getCString();
		if (string.isEmpty()) {
			string = QString("glyph ") + QString("%1").setNum (i, 16);
		}
		comboPostscriptNames->insertItem (string);
		if (string == oldPostscriptName) {
			int selectItem = comboPostscriptNames->count()-1;
			comboPostscriptNames->setCurrentItem (selectItem);
			glyphID = selectItem;
		}
	}

	if (buttonDebug->isOn()) {
		dialogExtension->textFontProgram->setText("");

		showDebug(true);
	}

	buttonDebug->setEnabled(true);
	resetGlyphView();
	QApplication::restoreOverrideCursor();
}

void GlyphViewerDialog::selectGlyph (int aGlyphID) {
	glyphID = aGlyphID;
	resetGlyphView();
}


void GlyphViewerDialog::resetGlyphView() {
	if (font) {
		proc->setGlyphId (glyphID);
		proc->restart();
		proc->runToEnd();
		glyphView->setGlyph (proc->getPoints(), (double)ppemy/ppemx);
	}

	if (buttonDebug->isOn())
		showDebug(true);
}

void GlyphViewerDialog::setSubPixel(bool subPixel) {
	glyphView->setSubPixel(subPixel);
}

// Note: This method is pretty slow; that's why it's called as little as possible in 
// showDebug(). This may be because of instructionText.append(). Needs investigation.
// Note 2: Replacing QString by (my own) String seems to help. Maybe QString
// does not provide exponential behaviour when appending???
void GlyphViewerDialog::appendToText (const InstructionProcessor::Instructions &instructions, QString &s) {
/*	for (InstructionProcessor::Instructions::const_iterator i = instructions.begin(); i != instructions.end(); i ++) {
		QString thisInstr;
		thisInstr.sprintf("%4d: %s\n", (*i)->getOffset(), (*i)->getName().getCString());
		s.append(thisInstr);
	}*/
	String instructionDescriptions;
	for (InstructionProcessor::Instructions::const_iterator i = instructions.begin(); i != instructions.end(); i ++) {
		instructionDescriptions += String ((*i)->getOffset()) + ' ' + (*i)->getName() + '\n';
	}
	s.append (instructionDescriptions.getCString());
}

void GlyphViewerDialog::showDebug(bool show) {
	if (show) {
	    QApplication::setOverrideCursor( Qt::waitCursor );

		QString instructionText;

		if (dialogExtension->textFontProgram->text().isEmpty()) {
			QString instructionText = "Font program:\n";
			appendToText (proc->getFontProgram(), instructionText);
			instructionText.append ("\n\nCVT Program");
			appendToText (proc->getCVTProgram(), instructionText);
			dialogExtension->textFontProgram->setText (instructionText);
		}

		instructionText = "Glyph Program:\n";
		appendToText(proc->getGlyphProgram(), instructionText);

		dialogExtension->textGlyphProgram->setText(instructionText);

		setCurrentInstructionText();
		QApplication::restoreOverrideCursor();
	} else {
		if (stackViewer)
			stackViewer->close();
		if (storageViewer)
			storageViewer->close();
		if (cvtViewer)
			cvtViewer->close();
		if (graphicsStateViewer)
			graphicsStateViewer->close();
	}
	if (extensionShown != show) {
		showExtension (show);
		extensionShown = show;
	}
}

void GlyphViewerDialog::firstInstruction() {
	proc->restart();
	glyphView->setGlyph (proc->getPoints(), (double)ppemy/ppemx);

	setCurrentInstructionText();
}

void GlyphViewerDialog::backInstruction() {
	int instructionCount = proc->getCurrentInstructionCount();
	proc->restart();
	if (instructionCount > 10)
		proc->runToCount (instructionCount-10);

	glyphView->setGlyph (proc->getPoints(), (double)ppemy/ppemx);
	setCurrentInstructionText();
}

void GlyphViewerDialog::previousInstruction() {
	int instructionCount = proc->getCurrentInstructionCount();
	proc->restart();
	proc->runToCount (instructionCount-1);

	glyphView->setGlyph (proc->getPoints(), (double)ppemy/ppemx);
	setCurrentInstructionText();
}

void GlyphViewerDialog::nextInstruction() {
	int instructionCount = proc->getCurrentInstructionCount();
	proc->runToCount(instructionCount+1);

	glyphView->setGlyph (proc->getPoints(), (double)ppemy/ppemx);
	setCurrentInstructionText();
}

void GlyphViewerDialog::forwardInstruction() {
	int instructionCount = proc->getCurrentInstructionCount();
	proc->runToCount(instructionCount+10);

	glyphView->setGlyph (proc->getPoints(), (double)ppemy/ppemx);
	setCurrentInstructionText();
}

void GlyphViewerDialog::lastInstruction() {
	proc->runToEnd();

	glyphView->setGlyph (proc->getPoints(), (double)ppemy/ppemx);
	setCurrentInstructionText();
}

void GlyphViewerDialog::setCurrentInstructionText() {
	QApplication::setOverrideCursor( Qt::waitCursor );

	QString s;
	QTextOStream o(&s);
	int instructionCount = proc->getCurrentInstructionCount();
	o << instructionCount;
	dialogExtension->labelInstructionCount->setText(s);

	dialogExtension->labelPosition->setText (proc->getCurrentInstructionPosition());
	dialogExtension->labelCurrentInstruction->setText (proc->getCurrentInstructionDescription());

	dialogExtension->buttonFirstInstruction->setEnabled (instructionCount > 0);
	dialogExtension->buttonBackInstruction->setEnabled (instructionCount > 0);
	dialogExtension->buttonPreviousInstruction->setEnabled (instructionCount > 0);
	bool notLastInstruction = !proc->isLastInstruction();
	dialogExtension->buttonNextInstruction->setEnabled (notLastInstruction);
	dialogExtension->buttonForwardInstruction->setEnabled (notLastInstruction);
	dialogExtension->buttonLastInstruction->setEnabled (notLastInstruction);

	if (stackViewer)
		stackViewer->setStack (proc->getStack());
	if (storageViewer)
		storageViewer->setStorage (proc->getStorageElements());
	if (cvtViewer)
		cvtViewer->setCVT (proc->getCVTEntries(), proc->getPPEMPixels());
	if (graphicsStateViewer)
		graphicsStateViewer->setGraphicsState (proc->getGraphicsState());

	QApplication::restoreOverrideCursor();
}


void GlyphViewerDialog::openStackViewer() {
	assert(!stackViewer);
	dialogExtension->buttonStackViewer->setEnabled(false);

	QApplication::setOverrideCursor( Qt::waitCursor );
	stackViewer = new StackViewerDialog(this);
	stackViewer->setStack (proc->getStack());
	stackViewer->show();
	QApplication::restoreOverrideCursor();
}

void GlyphViewerDialog::stackViewerClosed() {
	stackViewer = NULL;
	dialogExtension->buttonStackViewer->setEnabled(true);
}

void GlyphViewerDialog::openStorageViewer() {
	assert(!storageViewer);
	dialogExtension->buttonStorageViewer->setEnabled(false);

	QApplication::setOverrideCursor( Qt::waitCursor );
	storageViewer = new StorageViewerDialog(this);
	storageViewer->setStorage (proc->getStorageElements());
	storageViewer->show();
	QApplication::restoreOverrideCursor();
}

void GlyphViewerDialog::storageViewerClosed() {
	storageViewer = NULL;
	dialogExtension->buttonStorageViewer->setEnabled(true);
}

void GlyphViewerDialog::openCVTViewer() {
	assert(!cvtViewer);
	dialogExtension->buttonCVTViewer->setEnabled(false);

	QApplication::setOverrideCursor( Qt::waitCursor );
	cvtViewer = new CVTViewerDialog(this);
	cvtViewer->setCVT (proc->getCVTEntries(), proc->getPPEMPixels());
	cvtViewer->show();
	QApplication::restoreOverrideCursor();
}

void GlyphViewerDialog::cvtViewerClosed() {
	cvtViewer = NULL;
	dialogExtension->buttonCVTViewer->setEnabled(true);
}

void GlyphViewerDialog::openGraphicsStateViewer() {
	assert(!graphicsStateViewer);
	dialogExtension->buttonGraphicsStateViewer->setEnabled(false);

	QApplication::setOverrideCursor( Qt::waitCursor );
	graphicsStateViewer = new GraphicsStateViewerDialog(this);
	graphicsStateViewer->setGraphicsState(proc->getGraphicsState());
	graphicsStateViewer->show();
	QApplication::restoreOverrideCursor();
}

void GlyphViewerDialog::graphicsStateViewerClosed() {
	graphicsStateViewer = NULL;
	dialogExtension->buttonGraphicsStateViewer->setEnabled(true);
}

