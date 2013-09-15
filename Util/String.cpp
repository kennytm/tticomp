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
	along with Foobar; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <string.h>
#include "String.h"

namespace util {

StringCharacters::StringCharacters (int i, int base, int minLength) {
	length = 0;
	int j = i;
	do {
		length ++;
		j /= base;
	} while (j);
	
	if (length < minLength)
		length = minLength;

	char *begin, *cur;
	if (i < 0) {
		length ++;
		setCapacity (length);
		*characters = '-';
		begin = characters + 1;
		i = -i;
	} else {
		setCapacity (length);
		begin = characters;
	}
	cur = characters + length - 1;

	while (cur >= begin) {
		char n = i % base;
		if (n < 10)
			*cur = n + '0';
		else
			*cur = n + ('A' - 10);
		i /= base;
		cur --;
	}

	refCount = 1;
}

StringCharacters::StringCharacters (unsigned int i, int base, int minLength) {
	length = 0;
	unsigned int j = i;
	do {
		length ++;
		j /= base;
	} while (j);
	
	if (length < minLength)
		length = minLength;

	char *begin, *cur;
	setCapacity (length);
	begin = characters;
	cur = begin + length - 1;

	while (cur >= begin) {
		char n = i % base;
		if (n < 10)
			*cur = n + '0';
		else
			*cur = n + ('A' - 10);
		i /= base;
		cur --;
	}

	refCount = 1;
}

}	// namespace util
