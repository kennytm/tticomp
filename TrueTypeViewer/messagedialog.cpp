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

#include <qmultilineedit.h>
#include "../OTFont/OTException.h"
#include "messagedialog.h"

using namespace OpenType;
using util::String;

MessageDialog::MessageDialog (QWidget *parent) : MessageDialogBase (parent) {
	editMessages->clear();
}

MessageDialog::~MessageDialog() {}

void MessageDialog::addMessage (const Exception &e, bool error) {
	typedef std::vector <String> Strings;
	if (error)
		editMessages->append ("Error:");
	else
		editMessages->append ("Warning:");
	Strings context = e.getContext();
	Strings::iterator i;
	for (i = context.begin(); i != context.end(); i ++) {
		if (i + 1 != context.end())
			editMessages->append (QString ("    ") + (*i).getCString());
		else
			editMessages->append (QString ("    ") + (*i).getCString() + ":");
	}
	Strings descriptions = e.getDescriptions();
	for (i = descriptions.begin(); i != descriptions.end(); i ++) {
		if (i + 1 != descriptions.end())
			editMessages->append (QString ("    ") + (*i).getCString() + ";");
		else
			editMessages->append (QString ("    ") + (*i).getCString() + ".");
	}
	show();
}

void MessageDialog::clearList() {
	editMessages->clear();
}
