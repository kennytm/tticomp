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

#include <qlistview.h>
#include "f26dot6tostring.h"
#include "graphicsstateviewerdialog.h"
#include "glyphviewerdialog.h"

GraphicsStateViewerDialog::GraphicsStateViewerDialog( QWidget* parent)
: CVTViewerDialogBase( parent, NULL, false, WDestructiveClose ) {
	listItems->setColumnText(0, "Variable Name");
	listItems->addColumn("Value");
}

GraphicsStateViewerDialog::~GraphicsStateViewerDialog() {
	((GlyphViewerDialog *) parent()) -> graphicsStateViewerClosed();
}

void GraphicsStateViewerDialog::setGraphicsState (const GlyphProcessor::GraphicsState & state) {
	listItems->clear();
	new QListViewItem(listItems, QString("Auto Flip"), state.autoFlip ? "true" : "false");
	new QListViewItem(listItems, QString("Control Value Cut-In"), F26Dot6ToString(state.controlValueCutIn));
	new QListViewItem(listItems, QString("Delta base"), QString("%1").arg(state.deltaBase));
	new QListViewItem(listItems, QString("Delta Shift"), QString("%1").arg(state.deltaShift));
	new QListViewItem(listItems, QString("Projection Vector"),
		QString("(%1, %2)").arg(state.projectionVector.x.get()).arg(state.projectionVector.y.get()));
	new QListViewItem(listItems, QString("Dual Projection Vector"),
		QString("(%1, %2)").arg(state.dualProjectionVector.x.get()).arg(state.dualProjectionVector.y.get()));
	new QListViewItem(listItems, QString("Freedom Vector"),
		QString("(%1, %2)").arg(state.freedomVector.x.get()).arg(state.freedomVector.y.get()));
	new QListViewItem(listItems, QString("Zone Pointers"), QString("%1, %2, %3").arg(state.zp[0]).arg(state.zp[1]).arg(state.zp[2]));
	new QListViewItem(listItems, QString("Reference Points"), QString("%1, %2, %3").arg(state.rp[0]).arg(state.rp[1]).arg(state.rp[2]));
	new QListViewItem(listItems, QString("Instruct Control"), QString("%1").arg(state.instructionControl));
	new QListViewItem(listItems, QString("Loop"), QString("%1").arg(state.loop));
	new QListViewItem(listItems, QString("Minimum distance"), F26Dot6ToString(state.minimumDistance));
	new QListViewItem(listItems, QString("Round Phase"), F26Dot6ToString(state.roundPhase));
	new QListViewItem(listItems, QString("Round Period"), F26Dot6ToString(state.roundPeriod));
	new QListViewItem(listItems, QString("Round Threshold"), F26Dot6ToString(state.roundThreshold));
	new QListViewItem(listItems, QString("Scan Control"), QString("%1").arg(state.scanControl));
	new QListViewItem(listItems, QString("Single-Width Cut-in"), F26Dot6ToString (state.singleWidthCutIn));
	new QListViewItem(listItems, QString("Single-Width Value"), F26Dot6ToString (state.singleWidthValue));
}
