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

#include <iostream>
#include <iterator>
#include <algorithm>
#include <cassert>
#include "Decompositions.h"

using std::cout;
using std::endl;
using std::sort;
using util::String;

/*** Decomposition ***/

Decomposition::Decomposition(ULong aCharacter, const Components &aComponents,
							 String aName)
							 : character (aCharacter), components (aComponents), name (aName) {}

Decomposition::~Decomposition() {}

ostream &operator << (ostream &o, const Decomposition &d) {
	return o << "U+" << getCode(d.getCharacter()) << " " << d.getName();
}


/*** Decompositions ***/


Byte getNumber(Byte c) {
	if (c>='0' && c<='9')
		return c - '0';
	if (c>='A' && c<='F')
		return c - 'A'+10;
	if (c>='a' && c<='f')
		return c - 'a'+10;
	return 0xFF;
}

ULong getHex(Byte * &pos) {
	ULong number = 0;
	do {
		Byte x = getNumber(*pos);
		if (x==0xFF)
			return number;
		number *= 0x10;
		number += x;
		pos++;
	} while (true);
}

String getCode(ULong code) {
	if (code <= 0xFFFF)
		return String (code, 16, 4);
	else
		return String (code, 16, 8);
}

void findNextField(Byte * &pos) {
	while (*pos != ';') {
		pos ++;
	}
	pos ++;
}

void findNewLine(Byte * &pos) {
	while (*pos != '\n' && *pos != '\0') {
		pos ++;
	}
	do {
		pos ++;
	} while (*pos == '\n' || *pos == '\r');
}

ULong getTag(Byte * &pos) {
	if (isTagCharacter(*pos)) {
		ULong tag = 0x20202020;
		Byte *tagPos = (Byte*) &tag;
		int i;
		for (i=0; i<4 && isTagCharacter(*pos); i++) {
			*tagPos = *pos;
			tagPos ++;
			pos ++;
		}
		return tag;
	} else
		return 0;
}

bool orderDecompositions(DecompositionPtr d1, DecompositionPtr d2) {
	return d1->getCharacter() < d2->getCharacter();
}

Decompositions::Decompositions(String fileName) {
	FILE *file = fopen(fileName.getCString(), "rb");
	if (file==NULL)
	{
		cout << "File not found." << endl;
		return;
	}
	
	// obtain file size.
	fseek (file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);
	
	// allocate memory to contain the whole file.
	Byte *buffer = new Byte[size+1];
	buffer[size] = '\0';
	
	// copy the file into the buffer.
	if (fread(buffer, 1, size, file) != size) {
		cout << "Error reading file." << endl;
		delete[] buffer;
		return;
	}
	// terminate
	fclose (file);
	
	/*** the whole file is loaded in the buffer. ***/
	Byte *curPos = buffer;
	while (curPos < buffer+size) {
		if (*curPos != '#') {
			ULong character = getHex(curPos);
			findNextField(curPos);
			char *name = (char*) curPos;
			findNextField(curPos);
			*(curPos-1) = '\0';
			findNextField(curPos);
			findNextField(curPos);
			findNextField(curPos);
			Decomposition::Components components;
			while(true) {
				if (strncmp((char *)curPos, "<compat> ", 9) == 0)
					curPos += 9;
				ULong hex = getHex(curPos);
				if (hex)
					components.push_back (hex);
				else {
					if (!components.empty()) {
						// Add this character to the list
						decompositions.push_back(new Decomposition(character, components, name));
					}
					break;
				}
				if (*curPos == ' ') curPos ++;
			}
		}
		findNewLine(curPos);
	}
	
	delete[] buffer;
	sort(decompositions.begin(), decompositions.end(), orderDecompositions);
}

Decompositions::~Decompositions() {}

DecompositionPtr Decompositions::getRecursiveDecomposition(ULong character) {
	DecompositionVector::iterator guess, lower, upper;
	lower = decompositions.begin();
	upper = decompositions.end();
	while (lower != upper) {
		guess = lower + (upper - lower) / 2;
		ULong foundCharacter = (*guess)->getCharacter();
		if (foundCharacter < character)
			lower = guess + 1;
		else {
			if (character < foundCharacter)
				upper = guess;
			else
				return get(guess - decompositions.begin());
		}
	}

	return DecompositionPtr();
}

DecompositionPtr Decompositions::get(ULong index) {
	assert(index < decompositions.size());
	DecompositionPtr original = decompositions[index];
	const Decomposition::Components originalComponents = original->getComponents();
	Decomposition::Components components;
	Decomposition::Components::const_iterator o;
	for (o = originalComponents.begin(); o != originalComponents.end(); o ++) {
		DecompositionPtr recursive = getRecursiveDecomposition (*o);
		if (!recursive)
			components.push_back (*o);
		else
			components.insert (components.end(), recursive->getComponents().begin(), recursive->getComponents().end());
	}
	return new Decomposition (original->getCharacter(), components, original->getName());
}

ULong Decompositions::getNum() {
	return decompositions.size();
}
