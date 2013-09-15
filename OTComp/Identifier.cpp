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
*/

#include <iostream>
#include "Identifier.h"

using std::endl;

bool isExpected (Preprocessor &prep, String expected) {
	PreprocessorPosition pos = prep.getCurrentPosition();
	String buffer = prep.get();
	if (buffer != expected) {
		prep.startError (pos) << '\"' << expected << "\" expected instead of \""
			<< buffer << "\"." << endl;
		return false;
	}
	return true;
}

Tag getTag (Preprocessor &prep) {
	PreprocessorPosition pos = prep.getCurrentPosition();
	String buffer = prep.get();
	if (!buffer.empty() && buffer.length() <= 4) {
		// Fill with spaces
		while (buffer.length() < 4)
			buffer += ' ';
		return (Tag &) *buffer.getChars();
	}
	prep.startError (pos) << "OpenType tag expected instead of \"" << buffer << "\"." << endl;
	return 0;
}

#define isIdentifierChar(c) (isLetterChar (c) || (c) == '_' || (c) == '.')

String getIdentifier (Preprocessor &prep) {
	PreprocessorPosition pos = prep.getCurrentPosition();
	String buffer = prep.get();
	if (!buffer.empty()) {
		if (isIdentifierChar (buffer [0])) {
			if (buffer.length() == 1 || isIdentifierChar (buffer [1]) || isNumberChar (buffer [1]))
				return buffer;
		}
	}
	prep.startError (pos) << "Identifier expected instead of \"" << buffer << "\"." << endl;
	return "<Unknown>";
}
