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

#include <cassert>
#include <iostream>
#include "Script.h"
#include "Identifier.h"
#include "Lookup.h"

using std::endl;
using std::vector;

class Language {
	PreprocessorPosition pos;
	vector <String> features;
	String requiredFeature;
public:
	Language (Preprocessor &prep);
	virtual ~Language();

	ReferenceMemoryBlockPtr getTable (const FeatureIndexer & features);
};

/*** Script ***/

Script::Script (Preprocessor &prep) : pos (prep.getCurrentPosition()) {
	String buffer = prep.get();
	assert (buffer == "script");
	tag = ::getTag (prep);
	if (!isExpected (prep, "{"))
		return;


	buffer = prep.peek();
	if (buffer == "feature" || buffer == "required")
		defaultLanguage = new Language (prep);
	
	PreprocessorPosition pos = prep.getCurrentPosition();
	buffer = prep.get();
	while (buffer != "}") {
		if (buffer != "language") {
			prep.startError (pos) << "\"language\" expected instead of \"" << buffer << "\"." << endl;
			return;
		}
		LanguageTag language;
		language.tag = ::getTag (prep);
		if (!isExpected (prep, "{"))
			return;
		language.language = new Language (prep);
		languages.push_back (language);
		if (!isExpected (prep, "}"))
			return;
		buffer = prep.get();
	}
}

Script::~Script() {}

ReferenceMemoryBlockPtr Script::getTable (const FeatureIndexer & features) {
	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);
	if (defaultLanguage)
		pen.writeReference (defaultLanguage->getTable (features));
	else
		pen.writeReference (NULL);
	pen.writeUShort (languages.size());
	for (Languages::iterator l = languages.begin(); l != languages.end(); l ++) {
		pen.writeTag (l->tag);
		pen.writeReference (l->language->getTable (features));
	}
	return memory;
}


/*** Language ***/

Language::Language (Preprocessor & prep) : pos (prep.getCurrentPosition()) {
	String buffer = prep.peek();
	while (buffer == "feature" || buffer == "required") {
		prep.deleteToken();
		if (buffer == "feature") {
			String name = getIdentifier (prep);
			features.push_back (name);
			if (!isExpected (prep, ";"))
				return;
		} else {
			if (!isExpected (prep, "feature"))
				return;
			String name = getIdentifier (prep);
			if (!isExpected (prep, ";"))
				return;
			if (!requiredFeature.empty())
				prep.startError (pos) << "There may be only one required feature per language." << endl;
			else
				requiredFeature = name;
		}
		buffer = prep.peek();
	}
}

Language::~Language() {}

ReferenceMemoryBlockPtr Language::getTable (const FeatureIndexer & featureIndices) {
	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);
	pen.writeReference (NULL);
	int index;
	if (requiredFeature.empty() ||
		(index = featureIndices.getFeatureIndex (requiredFeature, pos)) == -1)
		pen.writeOffset (0xFFFF);
	else {
		pen.writeUShort ((UShort) index);
	}

	UShort featureNum = 0;
	ReferenceMemoryPen featureNumPen = pen;
	pen.writeUShort (0);
	for (Strings::iterator f = features.begin(); f != features.end(); f ++) {
		index = featureIndices.getFeatureIndex (*f, pos);
		if (index != -1) {
			pen.writeOffset ((UShort) index);
			featureNum ++;
		}
	}
	featureNumPen.writeUShort (featureNum);
	return memory;
}

/*** Feature ***/

Feature::Feature (Preprocessor & prep) : pos (prep.getCurrentPosition()) {
	String buffer = prep.get();
	assert (buffer == "feature");
	name = getIdentifier (prep);
	tag = ::getTag (prep);
	if (!isExpected (prep, "{"))
		return;
	buffer = prep.peek();
	while (buffer != "}") {
		if (!isExpected (prep, "lookup"))
			return;
		String lookupName = getIdentifier (prep);
		lookups.push_back (lookupName);
		lookupsFound.push_back (false);
		if (!isExpected (prep, ";"))
			return;

		buffer = prep.peek();
	}
	prep.deleteToken();
}

Feature::~Feature() {}

ReferenceMemoryBlockPtr Feature::getTable (const LookupSubTables & lookupTables, Bools & lookupsUsed) {
	UShort featureNum = 0;
	ReferenceMemoryBlockPtr memory (new ReferenceMemoryBlock);
	ReferenceMemoryPen pen (memory);
	pen.writeReference (NULL);
	// Write 0 as featureCount for now
	ReferenceMemoryPen featureNumPen = pen;
	pen.writeUShort (0);

	Strings::iterator s = lookups.begin();
	Bools::iterator b = lookupsFound.begin();
	for (; s != lookups.end(); s ++, b++) {
		LookupSubTables::const_iterator l = lookupTables.begin();
		Bools::iterator used = lookupsUsed.begin();
		for (; l != lookupTables.end(); l ++, used ++) {
			if (*s == (*l)->name) {
				featureNum ++;
				*b = true;
				*used = true;
				// Write lookup table index
				pen.writeUShort (l - lookupTables.begin());
			}
		}
	}

	if (featureNum) {
		featureNumPen.writeUShort (featureNum);
		return memory;
	} else
		return NULL;
}

void Feature::checkLookupsFound (Preprocessor & prep) const {
	Bools::const_iterator b = lookupsFound.begin();
	Strings::const_iterator s = lookups.begin();
	for (; b != lookupsFound.end(); b ++, s ++) {
		if (!*b)
			prep.startError (pos) << "Lookup \"" << *s << "\" not found." << endl;
	}
}
