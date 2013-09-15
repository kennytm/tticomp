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
	\file Decompositions contains all Unicode decompositions and is able to
	recursively decompose a character.
*/

#ifndef DECOMPOSITIONS_H
#define DECOMPOSITIONS_H

#include <vector>
#include <iostream>

#include "../Util/smart_ptr.h"
#include "../Util/String.h"

#include "../OTFont/OpenType.h"

using std::vector;
using std::ostream;

using namespace OpenType;

class Decomposition {
public:
	typedef vector <ULong> Components;
private:
	ULong character;
	vector <ULong> components;
	util::String name;
public:
	Decomposition(ULong aCharacter, const Components &aComponents, util::String aName);
	~Decomposition();
	ULong getCharacter() const { return character; }
	const Components &getComponents() const { return components; }
	util::String getName() const { return name; }
};

typedef util::smart_ptr <Decomposition> DecompositionPtr;

ostream & operator << (ostream &o, const Decomposition &d);

class Decompositions {
	typedef vector <DecompositionPtr> DecompositionVector;
	DecompositionVector decompositions;
public:
	Decompositions (util::String fileName);
	~Decompositions();
	DecompositionPtr getRecursiveDecomposition (ULong character);
	DecompositionPtr get (ULong index);
	ULong getNum();
};

// File reading helper routines

#define isTagCharacter(a) (((a) >= 'A' && (a) <= 'Z') || \
							((a) >= 'a' && (a) <= 'z') || \
							((a) >= '0' && (a) <= '9'))

Byte getNumber (Byte c);
ULong getHex (Byte * &pos);
util::String getCode (ULong code);
void findNextField (Byte * &pos);
void findNewLine (Byte * &pos);
Tag getTag (Byte * &pos);


#endif	//	DECOMPOSITIONS_H
