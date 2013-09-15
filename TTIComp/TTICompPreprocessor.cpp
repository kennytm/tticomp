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

/**
	\file TTICompPreprocessor provides a preprocessor specifically for .TTI files.
*/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <cassert>
#include "../OTFont/OpenType.h"
#include "TTICompPreprocessor.h"
#include "Statement.h"
#include "Scope.h"

using std::endl;

// Never mind the warning using "*this" may yield: it's not used yet so it is not a problem.
TTICompPreprocessor::TTICompPreprocessor(String fileName)
: Preprocessor(fileName), stackSize (false), twilightPoints (false),
internalPosition (*this, "internal", 0, 0) {
	specialChars['='] = true;
	specialChars['>'] = true;
	specialChars['<'] = true;
	specialChars['!'] = true;
	specialChars['|'] = true;
	specialChars['&'] = true;
}

TTICompPreprocessor::~TTICompPreprocessor() {}

void TTICompPreprocessor::setScope (smart_ptr <Scope> aScope) {
	scope = aScope;
}

bool TTICompPreprocessor::isSpecialToken (char firstChar, char secondChar) {
	return (firstChar == '=' && secondChar == '=') ||
		(firstChar == '!' && secondChar == '=') ||
		(firstChar == '|' && secondChar == '|') ||
		(firstChar == '&' && secondChar == '&') ||
		(firstChar == '>' && secondChar == '=') ||
		(firstChar == '<' && secondChar == '=');
}


void TTICompPreprocessor::addOption(String option, String param, PreprocessorPosition &pos) {
	// Input and output files
	bool input = (option == "input");
	if (input || option == "output") {
		if (param.length() < 2 || param[0] != '\"' || param[param.length()-1] != '\"') {
			startError(pos) << "Syntax error: single string enclosed in \"s expected after \"#" << option <<
				"\" instead of " << param << '.' << endl;
			return;
		}

		if ((input && !inputFileName.empty()) || (!input && !outputFileName.empty())) {
			startError(pos) << "Error: second \"#" << option << "\" option." << endl;
			return;
		}

		if (input)
			inputFileName = param.substring(1, param.length()-2);
		else
			outputFileName = param.substring(1, param.length()-2);

		return;
	}

	// gasp table entry

	if (option == "gasp") {
		if (!gaspRanges.empty()) {
			startError(pos) << "Error: second \"#gasp\" option." << endl;
			return;
		}

		String::const_iterator i = param.begin();
		UShort gaspLastMax = 0;
		do {
			GridFittingBehaviour::Range r;
			if (*i != 'I' && *i != 'N' && *i != 'A') {
				startError(pos) << "Syntax error: \"A\", \"I\" or \"N\" expected instead of \""
					<< *i << "\"." << endl;
				r.behaviour = GridFittingBehaviour::both;
				r.maxPPEM = 0xFFFF;
				gaspRanges.push_back (r);
				return;
			}

			if (*i == 'N')
			{	// Nothing; skip to next character and set gaspFlag to 0.
				i++;
				r.behaviour = GridFittingBehaviour::nothing;
			} else {
				if (*i=='I')
				{	// do instructions
					i++;
					if (i != param.end() && *i=='A') {
						r.behaviour = GridFittingBehaviour::both;
						i++;
					} else
						r.behaviour = GridFittingBehaviour::gridFit;
				} else
				{	// do anti-aliasing
					assert(*i=='A');
					i++;
					if (i != param.end() && *i=='I') {
						r.behaviour = GridFittingBehaviour::both;
						i++;
					} else
						r.behaviour = GridFittingBehaviour::greyscale;
				}
			}

			if (i == param.end()) {
				// End of parameter
				r.maxPPEM = 0xFFFF;
				gaspRanges.push_back (r);
				return;
			}
			if (!isWhiteSpace(*i)) {
				startError(pos) << "Syntax error: white space expected instead of \""
					<< *i << "\"." << endl;
				r.maxPPEM = 0xFFFF;
				gaspRanges.push_back (r);
				return;
			}
			do i++; while (i != param.end() && isWhiteSpace(*i));

			if (i == param.end()) {
				startError (pos) << "Syntax error: unexpected end in #gasp parameter." << endl;
				r.maxPPEM = 0xFFFF;
				gaspRanges.push_back (r);
				return;
			}

			if (!isNumberChar(*i)) {
				startError(pos) << "Syntax error: number expected instead of \""
					<< *i << "\"." << endl;
				r.maxPPEM = 0xFFFF;
				gaspRanges.push_back (r);
				return;
			}

			r.maxPPEM = 0;
			do {
				r.maxPPEM *= 10;
				r.maxPPEM += (*i-'0');
				i++;
			} while (i != param.end() && isNumberChar(*i));

			if (r.maxPPEM <= gaspLastMax) {
				startError(pos) << "Syntax error: gasp values should be increasing, but " <<
					gaspLastMax << " is larger than " << r.maxPPEM << "." << endl;
				r.maxPPEM = 0xFFFF;
				gaspRanges.push_back (r);
				return;
			}

			// Add middle range to gasp table
			gaspRanges.push_back (r);
			
			if (i != param.end() && !isWhiteSpace(*i)) {
				startError(pos) << "Syntax error: white space expected instead of \""
					<< *i << "\"." << endl;
				r.behaviour = GridFittingBehaviour::both;
				r.maxPPEM = 0xFFFF;
				gaspRanges.push_back (r);
				return;
			}
			do i++; while (i != param.end() && isWhiteSpace(*i));
		} while (true);
	}

	if (option == "cvt") {
		if (!cvtValues.empty()) {
			startError(pos) << "Error: second \"#cvt\" option." << endl;
			return;
		}
		String::const_iterator i = param.begin();
		while (i != param.end()) {
			if (isLetterChar(*i) || *i=='_')
			{	// identifier
				String::const_iterator j = i;
				do {
					j ++;
				} while (isLetterChar(*j) || isNumberChar(*j) || *j == '_');

		/*		char *idName = new char [j-i+1];
				memcpy(idName, &*i, j-i);
				idName[j-i] = '\0';*/
				String idName = String (i, j);
				if (*j != ':') {
					startError() << "Syntax error: \":\" expected after identifier \""
						<< idName <<"\"." << endl;
					return;
				}
				VariableDefinitionStatementPtr idStatement = new VariableDefinitionStatement(
					*scope, pos, tyUint, idName,
					new ConstantExpression(tyUint, cvtValues.size(), *scope, pos), true);
				scope->addVariableDef (idStatement);

				i = j+1;
			} else
			{	// Number
				if (!isNumberChar(*i) && *i!='-') {
					startError(pos) << "Syntax error: identifier or number expected instead of \""
						<< *i << "\"." << endl;
					return;
				}

				bool negative = false;
				if (*i=='-') {
					negative = true;
					i++;
				}

				Short cur = 0;
				do {
					cur *= 10;
					cur += (*i - '0');
					i++;
				} while (i != param.end() && isNumberChar(*i));

				if (i != param.end() && *i != ' ') {
					startError(pos) << "Syntax error: space expected instead of \""
						<< *i << "\"." << endl;
					return;
				}

				if (negative) cur = -cur;

				// Add new cvt value

				cvtValues.push_back (cur);
			}
			while (i != param.end() && isWhiteSpace(*i)) i++;
		}
		return;
	}

	// stack option

	if (option == "stack") {
		if (stackSize) {
			startError(pos) << "Error: second \"#stack\" option." << endl;
			return;
		}
		maxStackSize = 0;
		int i = 0;
		do {
			if (!isNumberChar(param[i])) {
				startError(pos) << "Syntax error: number expected instead of \"" << param[i] << "\"." << endl;
				return;
			}
			maxStackSize *= 10;
			maxStackSize+= (param[i]-'0');
			i ++;
		} while (i < param.length());
		stackSize = true;
		return;
	}

	// twilight option

	if (option == "twilight") {
		if (twilightPoints) {
			startError(pos) << "Error: second \"#twilight\" option." << endl;
			return;
		}
		maxTwilightPoints = 0;
		int i = 0;
		do {
			if (!isNumberChar(param[i])) {
				startError(pos) << "Syntax error: number expected instead of \"" << param[i] << "\"." << endl;
				return;
			}
			maxTwilightPoints *= 10;
			maxTwilightPoints += (param[i]-'0');
			i ++;
		} while (i < param.length());
		twilightPoints = true;
		return;
	}

	Preprocessor::addOption(option, param, pos);
}

PreprocessorPosition &TTICompPreprocessor::getInternalPosition() {
	return internalPosition;
}

String TTICompPreprocessor::getInputFileName() {
	return inputFileName;
}

String TTICompPreprocessor::getOutputFileName() {
	return outputFileName;
}

void TTICompPreprocessor::setTables(OpenTypeFont *font) {
	if (!gaspRanges.empty())
		font->setGridFittingBehaviour (gaspRanges);

	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);
	typedef vector <Short> UShortVector;
	UShortVector::iterator i;
	for (i = cvtValues.begin(); i != cvtValues.end(); i ++)
		pen.writeShort (*i);
	font->setcvt (memory);

	font->setMaxStackElements(maxStackSize);
	
	if (twilightPoints) {
		if (maxTwilightPoints)
			font->setMaxZones(2);
		else
			font->setMaxZones(1);
		font->setMaxTwilightPoints (maxTwilightPoints);
	}
}
