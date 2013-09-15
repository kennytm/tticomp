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

#ifdef _MSC_VER
#ifdef _DEBUG
// Testing for memory leaks
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#include <iostream>
#include <cassert>
#include "../OTFont/OpenTypeFont.h"
#include "../OTFont/OTException.h"

#include "CompositeOTText.h"
#include "Decompositions.h"
#include "PostscriptNameList.h"
#include "UnicodeRanges.h"

using std::cout;
using std::endl;
using util::shared_vector;

void printUsage() {
}

class coutOpenTypeFont : public OpenTypeFont {
public:
	virtual void addWarning (ExceptionPtr aWarning) {
		cout << "Warning: " << aWarning << endl;
	}
};

int main(int argCount, char *argValues[]) {
#ifdef _MSC_VER
#ifdef _DEBUG
// Testing for memory leaks
	// Set _crtBreakAlloc to break
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
#endif
	try {
		bool messages = true;

		if (argCount!=3) {
			printUsage();
			return -1;
		}

		coutOpenTypeFont font;
		font.readFromFile (argValues[1]);
		ULong i, j;

		/*** Precomposed characters ***/

		Decompositions decomp ("UnicodeData.txt");
		PostscriptNameList postNames ("glyphlist.txt");
		UnicodeRanges ranges ("UnicodeRanges.txt");
		RangeVector::iterator range = ranges.begin();
		cout << "Making legacy composite characters..." << endl;

		Tag script, language;
		bool useLanguage;
		
		script = (*range)->getScript();
		language = (*range)->getLanguage();
		shared_vector <Tag> languages = font.getLanguages(script);
		useLanguage = script != 0;
		vector<ULong> features = (*range)->getFeatures();
		
		for (i=0; i<decomp.getNum(); i++) {
			DecompositionPtr d = decomp.get(i);
			ULong unicode = d->getCharacter();
			int compare;
			if (range < ranges.end())
				compare = (*range)->compare(unicode);
			while (compare>0) {
				range++;
				if (range<ranges.end()) {
					compare = (*range)->compare(unicode);
					script = (*range)->getScript();
					language = (*range)->getLanguage();
					features = (*range)->getFeatures();
					useLanguage = (script != 0);
				} else
					compare = -1;
			}

			if (compare >= 0 && useLanguage) {
				if (font.getGlyphIndexByUnicode (unicode)) {
					if (messages) {
						cout << "Mapping for " << *d << " already exists." << endl;
					}
				} else
				{
					bool available = true;
					for (j=0; j < d->getComponents().size(); j++) {
						if (font.getGlyphIndexByUnicode (d->getComponents()[j]) == 0) {
							available = false;
							break;
						}
					}
					if (available)
					{	// All component characters are available
						if (compare < 0) {
							if (messages)
								cout << d << " falls outside all ranges." << endl;
						} else {
							CompositeText text (font);
							text.setUnicodes (d->getComponents());
							text.applyLookups (script, language, features);

							font.addUnicodeMapping(unicode, text.addToFont(postNames.getPostscriptName(unicode)));
						}
					}
				}
			}
		}

		font.writeToFile(argValues[2]);
		cout << "Legacy OpenType font successfully written to " << argValues[2] << "." << endl;
	} catch (Exception &e) {
		cout << "Error: " << e << endl;
		return -1;
	}
	return 0;
}
