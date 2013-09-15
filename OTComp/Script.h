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
	\file Script.h defines the Script class which represents an OpenType script.
*/

#ifndef SCRIPT_H
#define SCRIPT_H

#include <vector>
#include "../Util/smart_ptr.h"
#include "../Util/Preprocessor.h"
#include "../OTFont/OpenType.h"
#include "ReferenceTable.h"

using namespace OpenType;
using util::String;

struct LookupSubTable;
typedef smart_ptr <LookupSubTable> LookupSubTablePtr;
typedef std::vector <LookupSubTablePtr> LookupSubTables;

typedef std::vector <String> Strings;
typedef std::vector <bool> Bools;

class FeatureIndexer {
public:
	virtual int getFeatureIndex (String name, PreprocessorPosition & pos) const = 0;
};

class Language;
typedef smart_ptr <Language> LanguagePtr;
struct LanguageTag {
	LanguagePtr language;
	Tag tag;
};

typedef std::vector <LanguageTag> Languages;

class Script {
	PreprocessorPosition pos;
	Tag tag;
	Languages languages;
	LanguagePtr defaultLanguage;
public:
	Script (Preprocessor & prep);
	virtual ~Script();

	ReferenceMemoryBlockPtr getTable (const FeatureIndexer & features);
	Tag getTag() const { return tag; }
};

typedef smart_ptr <Script> ScriptPtr;
typedef std::vector <ScriptPtr> Scripts;

class Feature {
	PreprocessorPosition pos;
	String name;
	Tag tag;

	Strings lookups;
	Bools lookupsFound;
public:
	Feature (Preprocessor & prep);
	virtual ~Feature();

	ReferenceMemoryBlockPtr getTable (const LookupSubTables & lookups, Bools & lookupsUsed);
	void checkLookupsFound (Preprocessor & prep) const;
	Tag getTag() const { return tag; }
	String getName() const { return name; }
	const PreprocessorPosition & getPos() const { return pos; }
};

typedef smart_ptr <Feature> FeaturePtr;
typedef std::vector <FeaturePtr> Features;

#endif // SCRIPT_H
