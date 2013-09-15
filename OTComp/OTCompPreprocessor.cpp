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

#include <string.h>
#include "OTCompPreprocessor.h"
using std::endl;

OTCompPreprocessor::OTCompPreprocessor (String fileName) : Preprocessor(fileName) {
	firstIdentifierChars ['.'] = true;
	nextIdentifierChars ['.'] = true;
	specialChars ['-'] = true;
}

OTCompPreprocessor::~OTCompPreprocessor() {
}

bool OTCompPreprocessor::isSpecialToken (char firstChar, char secondChar) {
	return (firstChar == '-' && secondChar == '>');
}

void OTCompPreprocessor::addOption (String option, String param, PreprocessorPosition &pos) {
	bool input = false;

	// Input and output files
	if (option == "input")
		input = true;
	if (input || option == "output") {
		int length = param.length();
		if (length < 2 || param[0]!='\"' || param[length-1] != '\"') {
			startError(pos) << "Syntax error: single string enclosed in \"s expected after \"#" << option <<
				"\" instead of " << param << '.' << endl;
			return;
		}

		if ((input && !inputFileName.empty()) || (!input && !outputFileName.empty())) {
			startError(pos) << "Error: second \"#" << option << "\" option." << endl;
			return;
		}

		String newFile = param.substring(1, param.length()-2);

		if (input)
			inputFileName = newFile;
		else
			outputFileName = newFile;

		return;
	}
}

String OTCompPreprocessor::getInputFileName() {
	return inputFileName;
}

String OTCompPreprocessor::getOutputFileName() {
	return outputFileName;
}

