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

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <cassert>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include "truetypeviewerdialog.h"
#include "featuredialog.h"
#include "messagedialog.h"
#include "preview.h"
#include "glyphviewerdialog.h"

using util::String;
using util::shared_vector;

TrueTypeViewerDialog::TrueTypeViewerDialog( QWidget* parent, const char* name, bool modal, WFlags f )
	: TrueTypeViewerDialogBase( parent, name, modal, f )
{
	featureDialog = new FeatureDialog (this);
	messageDialog = new MessageDialog (this);
	preview->setMessageDialog (messageDialog);
	updatingPointSize = false;

	// Initialise widgets
	preview->setFontCache(cache, font, 16, 16);
	editPreviewText->setText("Preview b/epsilon d\\25a");

	spinPointSize->setValue(12);
	spinDPIx->setValue(96);
	spinDPIy->setValue(96);
}

TrueTypeViewerDialog::~TrueTypeViewerDialog() {
	GlyphViewerDialog *dlg;
	while (dlg = subdialogs.getFirst()) {
		dlg->close();
	}
	featureDialog->close();
	delete featureDialog;

	messageDialog->close();
	delete messageDialog;
}

class MessageOpenTypeFont : public OpenTypeFont {
	MessageDialog *messageDialog;
public:
	MessageOpenTypeFont (MessageDialog *aMessageDialog) : messageDialog (aMessageDialog) {}

	virtual void addWarning (smart_ptr <Exception> warning) {
		messageDialog->addMessage (*warning, false);
	}
};

void TrueTypeViewerDialog::loadFont() {
    QApplication::setOverrideCursor( Qt::waitCursor );

	smart_ptr <OpenTypeFont> newFont;
	try {
		newFont = new MessageOpenTypeFont (messageDialog);
		newFont->readFromFile (editFileName->text().latin1());
		// Force glyphs to be read
		newFont->getGlyph (0);
	} catch (Exception &e) {
		messageDialog->addMessage (e, true);
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(this, "Error loading TrueType file",
			String(e.getDescriptions().front()).getCString(), 1, 0);
		return;
	}

	int ppemx = spinPPEMx->value();
	int ppemy = spinPPEMy->value();
	int pointSize = spinPointSize->value();

	try {
		font = newFont;
		proc = new MessageInstructionProcessor (messageDialog);
		proc->setFont (font);
	} catch (Exception &e) {
		messageDialog->addMessage (e, true);
	}

	try {
		cache = new FontCache (font, proc, ppemx, ppemy, pointSize, messageDialog);
		preview->setFontCache (cache, font, ppemx, ppemy, false);
		featureDialog->setFont(font);
	} catch (Exception &e) {
		messageDialog->addMessage (e, true);
	}

	// Update glyph viewer dialogs

	for (QListIterator<GlyphViewerDialog> iterator(subdialogs);
		iterator.current(); ++iterator) {
		iterator.current()->setFont (font, *proc);
	}

	QApplication::restoreOverrideCursor();
}

void TrueTypeViewerDialog::browseForFileName() {
	QString fileName = 
		QFileDialog::getOpenFileName(editFileName->text(), "TrueType files (*.ttf *.otf)", this, "Open TrueType File", "");
	if (fileName) {
		editFileName->setText(fileName);
		loadFont();
	}
}

void TrueTypeViewerDialog::setPPEMx(int appemx) {
	if (checkSquare->isChecked())
		spinPPEMy->setValue(appemx);
	int pointSize = spinPointSize->value();
	if ((spinPointSize->value() * spinDPIx->value() + 36) / 72 != appemx)
		spinDPIx->setValue((appemx * 72 + (pointSize/2)) / pointSize);

	if (!updatingPointSize)
		updatePPEM();
}

void TrueTypeViewerDialog::setPPEMy(int appemy) {
	if (checkSquare->isChecked())
		spinPPEMx->setValue(appemy);
	int pointSize = spinPointSize->value();
	if ((spinPointSize->value() * spinDPIy->value() + 36) / 72 != appemy)
		spinDPIy->setValue((appemy * 72 + (pointSize/2)) / pointSize);

	if (!updatingPointSize)
		updatePPEM();
}

void TrueTypeViewerDialog::setDPIx(int aDPIx) {
	if (checkSquare->isChecked())
		spinDPIy->setValue(aDPIx);
	int pointSize = spinPointSize->value();
	if ((pointSize * aDPIx + 36) / 72 != spinPPEMx->value())
		spinPPEMx->setValue((pointSize * aDPIx + 36) / 72);

	updatePPEM();
}

void TrueTypeViewerDialog::setDPIy(int aDPIy) {
	if (checkSquare->isChecked())
		spinDPIx->setValue(aDPIy);
	int pointSize = spinPointSize->value();
	if ((pointSize * aDPIy + 36) / 72 != spinPPEMy->value())
		spinPPEMy->setValue((pointSize * aDPIy + 36) / 72);

	updatePPEM();
}

void TrueTypeViewerDialog::setPointSize(int aPointSize) {
	updatingPointSize = true;
	spinPPEMx->setValue((aPointSize * spinDPIx->value() + 36) / 72);
	spinPPEMy->setValue((aPointSize * spinDPIy->value() + 36) / 72);
	updatingPointSize = false;
	updatePPEM();
}

void TrueTypeViewerDialog::setSquare(bool aSquare) {
	if (aSquare) {
		spinDPIy->setValue(spinDPIx->value());
	}
}

void TrueTypeViewerDialog::updatePPEM() {
    QApplication::setOverrideCursor( Qt::waitCursor );

	if (cache) {
		int ppemx = spinPPEMx->value();
		int ppemy = spinPPEMy->value();
		cache->setPPEM (ppemx, ppemy, spinPointSize->value());
		preview->setFontCache(cache, font, ppemx, ppemy);
	}
	// Update glyph viewer dialogs

	for (QListIterator<GlyphViewerDialog> iterator(subdialogs);
		iterator.current(); ++iterator) {
		iterator.current()->setPPEM(spinPPEMx->value(), spinPPEMy->value(), spinPointSize->value());
	}

	QApplication::restoreOverrideCursor();
}

void TrueTypeViewerDialog::newGlyphWindow() {
    QApplication::setOverrideCursor( Qt::waitCursor );
	GlyphViewerDialog *dlg = new GlyphViewerDialog(messageDialog, this);
	dlg->show();
	subdialogs.append(dlg);
	dlg->setPPEM(spinPPEMx->value(), spinPPEMy->value(), spinPointSize->value());
	if (font) {
		dlg->setSubPixel (checkSubPixel->isChecked());
		dlg->setFont (font, *proc);
	}
	QApplication::restoreOverrideCursor();
}

void TrueTypeViewerDialog::newFeatureWindow() {
	QApplication::setOverrideCursor(Qt::waitCursor);
	featureDialog->show();
	QApplication::restoreOverrideCursor();
}

void TrueTypeViewerDialog::featureDialogClosed() {
	buttonFeatures->setEnabled(true);
}

void TrueTypeViewerDialog::newMessageWindow() {
	QApplication::setOverrideCursor(Qt::waitCursor);
	messageDialog->show();
	QApplication::restoreOverrideCursor();
}

void TrueTypeViewerDialog::subdialogClosed(GlyphViewerDialog *dlg) {
	subdialogs.remove(dlg);
}

void TrueTypeViewerDialog::setSubPixel(bool subPixel) {
	preview->setSubPixel(subPixel);

	for (QListIterator<GlyphViewerDialog> iterator(subdialogs); iterator.current(); ++iterator) {
		iterator.current()->setSubPixel(subPixel);
	}

	if (checkSquare->isChecked())
		checkSquare->setChecked(false);
	if (subPixel)
		spinDPIx->setValue(spinDPIx->value() * 3);
	else
		spinDPIx->setValue(spinDPIx->value() / 3);
}

void TrueTypeViewerDialog::setFeatures(Tag aScriptID, Tag aLanguageID, const shared_vector <Tag> &aFeatures) {
	features = aFeatures;
	preview->setFeatures(aScriptID, aLanguageID, features);
}
