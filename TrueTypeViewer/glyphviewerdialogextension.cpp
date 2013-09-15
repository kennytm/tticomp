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

#include <qpushbutton.h>
#include "glyphviewerdialogextension.h"

GlyphViewerDialogExtension::GlyphViewerDialogExtension (QWidget* parent)
: GlyphViewerDialogExtensionBase (parent)
{
	connect( buttonFirstInstruction, SIGNAL( clicked() ), parent, SLOT( firstInstruction() ) );
	connect( buttonBackInstruction, SIGNAL( clicked() ), parent, SLOT( backInstruction() ) );
	connect( buttonPreviousInstruction, SIGNAL( clicked() ), parent, SLOT( previousInstruction() ) );
	connect( buttonNextInstruction, SIGNAL( clicked() ), parent, SLOT( nextInstruction() ) );
	connect( buttonForwardInstruction, SIGNAL( clicked() ), parent, SLOT( forwardInstruction() ) );
	connect( buttonLastInstruction, SIGNAL( clicked() ), parent, SLOT( lastInstruction() ) );
	connect( buttonStackViewer, SIGNAL( clicked() ), parent, SLOT( openStackViewer() ) );
	connect( buttonStorageViewer, SIGNAL( clicked() ), parent, SLOT( openStorageViewer() ) );
	connect( buttonCVTViewer, SIGNAL( clicked() ), parent, SLOT( openCVTViewer() ) );
	connect( buttonGraphicsStateViewer, SIGNAL( clicked() ), parent, SLOT( openGraphicsStateViewer() ) );
}

GlyphViewerDialogExtension::~GlyphViewerDialogExtension() {}

