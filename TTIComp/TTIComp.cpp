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
	\file TTIComp compiles a .TTI file into an instructed TrueType file.
*/

/*** TODOs
  -	various statements (for, break...)
  - checking return values
  - inline functions
  - optimise storage location assignment
***/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#ifdef _MSC_VER
#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#include <iostream>
#include "../Util/smart_ptr.h"
#include "../Util/Preprocessor.h"

#include "../OTFont/OpenTypeFont.h"
#include "../OTFont/OTGlyph.h"

#include "FunctionScope.h"
#include "TTICompPreprocessor.h"

using std::endl;
using std::cout;

bool optimise = false;
bool listing = false;

void printUsage() {
	cout<< "    TTIComp  compiles a .TTI file into an instructed TrueType file" << endl
		<< "Usage : TTIComp [-o] [-l] filename.tti" << endl
		<< "where" << endl
		<< "  -o   produce optimised code" << endl
		<< "  -l   print a listing of the compiled code" << endl;
}

class coutOpenTypeFont : public OpenTypeFont {
public:
	coutOpenTypeFont () {}
	virtual void addWarning (ExceptionPtr aWarning) {
		cout << "Warning: " << aWarning << endl;
	}
};


int main(int argCount, char *argValues[]) {
//	chdir("F:/Van Dalen/Mijn documenten/Rogier/Fonts/fonts");
//	chdir("c:/Documents and Settings/Rogier van Dalen/My Documents/Fonts/fonts");
#ifdef _MSC_VER
#ifdef _DEBUG
	// Set _crtBreakAlloc to break
//	_CrtSetReportMode (_CRT_WARN, _CRTDBG_MODE_WNDW);
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
#endif
	try {
		int i;
		String buffer;
		if (argCount<2) {
			printUsage();
			return -1;
		}

		i = 1;
		while (i < argCount-1) {
			if (strcmp(argValues[i], "-o")==0)
				optimise = true;
			else {
				if (strcmp(argValues[i], "-l")==0)
					listing = true;
				else {
					cout << "Argument " << i << "could not be parsed" << endl;
					printUsage();
					return -1;
				}
			}
			i++;
		}

		smart_ptr <FunctionScope> scope;
		smart_ptr <TTICompPreprocessor> prep;
		try {
			prep = new TTICompPreprocessor(argValues[argCount-1]);
			// Compile program
			scope = new FunctionScope (*prep);
			prep->setScope (scope);
			scope->readFile();
		} catch (String s) {
			cout << "Fatal error: " << s << endl;
			return -1;
		} catch (TooManyErrorsException &e) {
			e;
			cout << "Too many errors; exiting." << endl;
			return -1;
		}

		int errorNum, warningNum;
		errorNum = prep->getErrorNum();
		warningNum = prep->getWarningNum();

		cout << argValues[argCount-1] << ": " << errorNum << " errors, " << warningNum << " warnings." << endl;

		if (errorNum == 0) {
			String inputFileName = prep->getInputFileName();
			if (inputFileName.empty()) {
				cout << "Error: No OpenType input file was specified through #input \"<filename>\"." << endl;
				return -1;
			}

			String outputFileName = prep->getOutputFileName();
			if (outputFileName.empty()) {
				cout << "Error: No OpenType input file was specified through #output \"<filename>\"." << endl;
				return -1;
			}

			cout << "Linking up..." << endl;

			// Try to load font

			smart_ptr <OpenTypeFont> font = new coutOpenTypeFont ();
			font->readFromFile (inputFileName);

			// Wire up function calls

			FunctionDefinitionStatementPtr functionDef;
			TypeVector noParams;	// Empty parameters
			UShort glyphNum = font->getGlyphNum();
			for (i = 0; i<glyphNum; i++) {
				// Try to connect the postscript name to the function
				String glyphName = font->getGlyph (i)->getName();
				while (true) {
					int index = glyphName.indexOf('.');
					if (index == -1)
						break;
					glyphName.deleteCharacters(index, 1);
				}

				buffer = "glyph";

				// Find <glypName>() function
				functionDef = scope->getFunctionDefinition (glyphName, noParams);

				if (!functionDef)
					// Find glyph<glyphId> function
					functionDef = scope->getFunctionDefinition (buffer + String(i), noParams);

				if (functionDef)
					functionDef->setCalled(true, i);
			}
			functionDef = scope->getFunctionDefinition ("prep", noParams);
			if (functionDef)
				functionDef->setCalled(true);

			scope->deleteUncalledDefinitions();

			errorNum = prep->getErrorNum();
			warningNum = prep->getWarningNum();
			cout << "Linked " << argValues[argCount-1] << ": " << errorNum << " errors, " << warningNum << " warnings." << endl;
			if (errorNum) {
				return -1;
			}

			if (listing)
				cout << "Program listing:" << endl << scope;

			scope->compileFont(font);

			// Set up gasp and cvt tables if specified
			prep->setTables(&*font);

			// Write font
			font->writeToFile(outputFileName);
		}
	} catch (Exception &e) {
		cout << "Error: " << e << endl;
		return -1;
	}
	return 0;
}
