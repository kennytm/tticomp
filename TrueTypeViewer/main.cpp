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

#ifdef WIN32
#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#include <qapplication.h>
#include <cassert>
#include "truetypeviewerdialog.h"

bool tryFont(QApplication& app, QString fontName, int pointSize) {
	QFont font(fontName, pointSize);
	QFontInfo info(font);
	if (info.family() == fontName) {
		app.setFont(font);
		return true;
	} else
		return false;
}

int main( int argc, char** argv )
{
#ifdef _MSC_VER
#ifdef _DEBUG
	// Set _crtBreakAlloc to break
//	_CrtSetReportMode (_CRT_WARN, _CRTDBG_MODE_WNDW);
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
#endif

	QApplication app( argc, argv );

	// Pick Legendum font as a default (of course)
	if (!tryFont(app, "Legendum", 8)) {
		if (!tryFont(app, "Verdana", 8))
			tryFont(app, "Tahoma", 8);
	}

	TrueTypeViewerDialog dialog( 0, 0, false );
	app.setMainWidget(&dialog);

	dialog.show();
	app.exec();

	return 0;
}

