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
#include <vector>
#include <algorithm>

#include "../OTFont/OTException.h"
#include "../OTFont/OpenTypeFont.h"
#include "../OTFont/OTTags.h"
#include "../OTFont/OTGlyph.h"
#include "ReferenceTable.h"

#include "OTCompPreprocessor.h"
#include "Script.h"
#include "GlyphList.h"
#include "Lookup.h"

using namespace OpenType;
using std::cout;
using std::endl;
using std::vector;


/*** coutOpenTypeFont ***/

class coutOpenTypeFont : public OpenTypeFont {
protected:
	virtual void addWarning (ExceptionPtr aWarning) {
		cout << "Warning: " << aWarning << endl;
	}
public:
	coutOpenTypeFont () {}
};

void printUsage() {
	cout << "OTComp: OpenType Layout Compiler by Rogier van Dalen" << endl
		<< "   OTComp <filename.ot>" << endl
		<< "  where <filename.ot> is the name of the input file." << endl;
}

bool compareFeatures (FeaturePtr f1, FeaturePtr f2) {
	return compareTags (f1->getTag(), f2->getTag());
}

typedef vector<int> Ints;

class LayoutFeatureIndexer : public FeatureIndexer {
	const Features & features;
	const Ints &featureIndices;
	Bools &featuresUsed;
public:
	LayoutFeatureIndexer (const Features & _features, const Ints & _featureIndices,
		Bools & _featuresUsed)
		: features (_features), featureIndices (_featureIndices), featuresUsed (_featuresUsed) {}
	~LayoutFeatureIndexer() {}

	virtual int getFeatureIndex (String name, PreprocessorPosition & pos) const {
		Features::const_iterator f = features.begin();
		Ints::const_iterator i = featureIndices.begin();
		Bools::iterator b = featuresUsed.begin();
		for (; f != features.end(); f ++, i ++, b ++) {
			if ((*f)->getName() == name) {
				*b = true;
				return *i;
			}
		}
		pos.prep.startError (pos) << "Feature \"" << name << "\" not found." << endl;
		return 0;
	}
};

ReferenceTablePtr getLayoutTable (Scripts & scripts, Features & features, Bools & featuresUsed,
								  LookupSubTables & lookups, Preprocessor & prep,
								  OpenTypeFile & font, Tag tag, LookupSubTable::LookupPlace place)
{
	// Assemble lookup data

	if (!lookups.empty()) {
		for (LookupSubTables::iterator lookup = lookups.begin() + 1; lookup != lookups.end();) {
			if ((*lookup)->name == (*(lookup - 1))->name &&
				(*lookup)->place == (*(lookup - 1))->place &&
				(*lookup)->type == (*(lookup - 1))->type &&
				(*lookup)->lookupFlag == (*(lookup - 1))->lookupFlag)
			{
				// Merge these two lookups
				std::copy ((*lookup)->memory.begin(), (*lookup)->memory.end(),
					std::back_insert_iterator <ReferenceMemoryBlocks> ((*(lookup - 1))->memory));
				lookup = lookups.erase (lookup);
			} else
				lookup ++;
		}
	}

	ReferenceMemoryBlockPtr lookupList (new ReferenceMemoryBlock);
	ReferenceMemoryPen lookupListPen (lookupList);
	lookupListPen.writeUShort (lookups.size());
	LookupSubTables::iterator l;
	for (l = lookups.begin(); l != lookups.end(); l ++) {
		ReferenceMemoryBlockPtr lookupTable (new ReferenceMemoryBlock);
		ReferenceMemoryPen lookupTablePen (lookupTable);
		lookupTablePen.writeUShort ((*l)->type);
		lookupTablePen.writeUShort ((*l)->lookupFlag);
		lookupTablePen.writeUShort ((*l)->memory.size());
		for (ReferenceMemoryBlocks::iterator m = (*l)->memory.begin(); m != (*l)->memory.end(); m ++)
			lookupTablePen.writeReference (*m);

		lookupListPen.writeReference (lookupTable);
	}

	// Assemble feature data

	Ints featureIndices (features.size(), -1);

	UShort featureNum = 0;
	ReferenceMemoryBlockPtr featureList (new ReferenceMemoryBlock);
	ReferenceMemoryPen featureListPen (featureList);
	ReferenceMemoryPen featureNumPen = featureListPen;
	Bools lookupsUsed (lookups.size(), false);

	writeReferenceLookups (lookups, lookupsUsed, place);

	// write 0 for now
	featureListPen.writeUShort (0);
	Features::iterator f = features.begin();
	Ints::iterator index = featureIndices.begin();
	for (; f != features.end(); f ++, index ++) {
		ReferenceMemoryBlockPtr featureTable = (*f)->getTable (lookups, lookupsUsed);
		if (featureTable) {
			*index = featureNum;
			featureNum ++;
			featureListPen.writeTag ((*f)->getTag());
			featureListPen.writeReference (featureTable);
		}
	}
	featureNumPen.writeUShort (featureNum);

	Bools::iterator b = lookupsUsed.begin();
	l = lookups.begin();
	for (; l != lookups.end(); b++, l ++) {
		if (!*b)
			prep.startError ((*l)->pos) << "Unused lookup \"" << (*l)->name << "\"." << endl;
	}

	// Assemble script data

	LayoutFeatureIndexer indexer (features, featureIndices, featuresUsed);

	ReferenceMemoryBlockPtr scriptList (new ReferenceMemoryBlock);
	ReferenceMemoryPen scriptListPen (scriptList);
	scriptListPen.writeUShort (scripts.size());
	for (Scripts::iterator s = scripts.begin(); s != scripts.end(); s ++) {
		scriptListPen.writeTag ((*s)->getTag());
		scriptListPen.writeReference ((*s)->getTable (indexer));
	}

	// Table header

	ReferenceMemoryBlockPtr header (new ReferenceMemoryBlock);
	ReferenceMemoryPen headerPen (header);
	// version
	headerPen.writeFixed (0x00010000);
	headerPen.writeReference (scriptList);
	headerPen.writeReference (featureList);
	headerPen.writeReference (lookupList);

	ReferenceTablePtr table (new ReferenceTable (font, tag));
	table->add (header);

	return table;
}

ReferenceTablePtr getGDEFTable (const GlyphClasses &glyphClasses, const Groups & groups,
								OpenTypeFont & font) {
	ReferenceMemoryBlockPtr header (new ReferenceMemoryBlock);
	ReferenceMemoryPen headerPen (header);

	ReferenceMemoryBlockPtr glyphClassDef;
	ReferenceMemoryBlockPtr markAttachClassDef;

	if (!glyphClasses.empty()) {
		glyphClassDef = getClassDefTable (glyphClasses);
	}

	markAttachClassDef = groups.getMarkClassDef (font);

	// version
	headerPen.writeFixed (0x00010000);
	headerPen.writeReference (glyphClassDef);
	headerPen.writeReference (NULL);
	headerPen.writeReference (NULL);
	headerPen.writeReference (markAttachClassDef);

	ReferenceTablePtr table (new ReferenceTable (font, GDEFTag));
	table->add (header);

	return table;
}

/*** main ***/

int main (int argCount, char *argValues[]) {
#ifdef WIN32
#ifdef _DEBUG
	// Set _crtBreakAlloc to break
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
#endif

	if (argCount!=2) {
		printUsage();
		return -1;
	}

	smart_ptr <OTCompPreprocessor> prep;
	smart_ptr <OpenTypeFont> font;
	Scripts scripts;
	Features features;
	Groups groups;
	GlyphClasses glyphClasses;
	LookupSubTables GSUBLookups;
	LookupSubTables GPOSLookups;
	try {
		try {
			prep = new OTCompPreprocessor (argValues[1]);
		} catch (String s) {
			cout << s << endl;
			cout << "Exiting." << endl;
			return -1;
		}

		PreprocessorPosition pos = prep->getCurrentPosition();
		
		// Read input and output files

		String buffer;
		buffer = prep->peek();
		String inputFileName = prep->getInputFileName();
		String outputFileName = prep->getOutputFileName();
		if (inputFileName.empty()) {
			prep->startError (pos) << "Error: no input file name specified with \"#input\"." << endl;
			return -1;
		}
		if (outputFileName.empty()) {
			prep->startError (pos) << "Error: no output file name specified with \"#output\"." << endl;
			return -1;
		}

		smart_ptr <OpenTypeFont> font = new coutOpenTypeFont();
		font->readFromFile (inputFileName);

		// Read statements
		while (!prep->eof()) {
			PreprocessorPosition pos = prep->getCurrentPosition();
			buffer = prep->peek();
			if (buffer == "script")
				scripts.push_back (new Script (*prep));
			else if (buffer == "feature")
				features.push_back (new Feature (*prep));
			else if (buffer == "group") {
				GlyphGroupPtr group = new GlyphGroup (*prep, *font, groups);
				GlyphClass glyphClass = group->getGlyphClass();
				if (glyphClass) {
					if (glyphClasses.empty())
						glyphClasses = GlyphClasses (font->getGlyphNum(), gcUndefined);
					const GlyphListPtr glyphs = group->getGlyphs();
					for (GlyphList::const_iterator i = glyphs->begin(); i != glyphs->end(); i ++) {
						if (glyphClasses [*i] == gcUndefined)
							glyphClasses [*i] = glyphClass;
						else {
							prep->startError() << "Glyph \"" << font->getGlyph (*i)->getName() <<
								"\" has a glyph class already." << endl;
						}
					}
				}
				groups.insert (Groups::value_type (group->getName(), group));
			} else if (buffer == "lookup") {
				LookupSubTablePtr lookup = (getLookup (*prep, *font, groups));
				if (lookup->place == LookupSubTable::GSUBLookup)
					GSUBLookups.push_back (lookup);
				else
					GPOSLookups.push_back (lookup);
			} else {
				prep->startError (pos) << "Script, feature or lookup definition expected instead of \""
					<< buffer << "\"." << endl;
				prep->deleteToken();
			}
		}

		cout << argValues[1] << ": " << prep->getErrorNum() << " errors, "
			<< prep->getWarningNum() << " warnings." << endl;
		if (prep->getErrorNum()) {
			return prep->getErrorNum();
		}

		Anchor::writeContourPointAnchors (*font);

		std::sort (features.begin(), features.end(), compareFeatures);

		Bools featuresUsed (features.size(), false);
		font->setGSUB (getLayoutTable (scripts, features, featuresUsed, GSUBLookups, *prep,
			*font, GSUBTag, LookupSubTable::GSUBLookup));
		font->setGPOS (getLayoutTable (scripts, features, featuresUsed, GPOSLookups, *prep,
			*font, GPOSTag, LookupSubTable::GPOSLookup));
		font->setGDEF (getGDEFTable (glyphClasses, groups, *font));

		Features::iterator f;
		for (f = features.begin(); f != features.end(); f ++)
			(*f)->checkLookupsFound (*prep);

		f = features.begin();
		Bools::iterator b = featuresUsed.begin();
		for (; f != features.end(); f ++, b ++) {
			if (!*b)
				prep->startError ((*f)->getPos()) << "Feature \"" << (*f)->getName() << "\" unused." << endl;
		}

		cout << argValues[1] << ": " << prep->getErrorNum() << " errors, "
			<< prep->getWarningNum() << " warnings." << endl;
		if (prep->getErrorNum()) {
			return prep->getErrorNum();
		}

		// Write file
		font->writeToFile (outputFileName);
	} catch (Exception &e) {
		cout << "Error: " << e << endl;
		return -1;
	} catch (TooManyErrorsException) {
		cout << "Too many errors." << endl;
		return -1;
	}
	return 0;
}

