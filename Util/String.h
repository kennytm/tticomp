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

/****************************************************************************
	\file String.h defines a nice and easy string implementation.

  When using Visual C++, you may want to add these lines to you AUTOEXP.DATA
  file to show strings in your watch window:

	; String class
	String =<characters->characters,st> length=<characters->length>

****************************************************************************/

#ifndef STRING_H
#define STRING_H

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>

namespace util {

	class StringCharacters;
	class const_StringIterator;
	class Char;

	class const_StringIterator {
	protected:
	#ifdef _DEBUG
		bool valid;
	#endif

		const char *c;

		friend class String;
	#ifdef _DEBUG
		const_StringIterator (const char *aC) : c (aC), valid (true) {}

		void invalidate() { valid = false; }
	#else
		const_StringIterator (const char *aC) : c (aC) {}
	#endif

	public:
		const_StringIterator () : c (NULL) {
	#ifdef _DEBUG
			valid = false;
	#endif
		}

		const_StringIterator & operator = (const const_StringIterator & i) {
	#ifdef _DEBUG
			valid = i.valid;
	#endif
			c = i.c;
			return *this;
		}

		char operator * () const {
	#ifdef _DEBUG
			assert (valid);
	#endif
			return *c;
		}

		bool operator == (const const_StringIterator & i) const {
			return c == i.c;
		}

		bool operator != (const const_StringIterator & i) const {
			return c != i.c;
		}

		const_StringIterator &operator ++ () {
			c++;
			return *this;
		}

		const_StringIterator operator ++ (int) {
			const_StringIterator temp = *this;
			c++;
			return temp;
		}

		const_StringIterator &operator -- () {
			c--;
			return *this;
		}

		const_StringIterator operator -- (int) {
			const_StringIterator temp = *this;
			c--;
			return temp;
		}

		const_StringIterator &operator += (int i) {
			c += i;
			return *this;
		}

		const_StringIterator &operator -= (int i) {
			c -= i;
			return *this;
		}

		const_StringIterator operator + (int i) const {
			return const_StringIterator (c + i);
		}
		const_StringIterator operator - (int i) const {
			return const_StringIterator (c - i);
		}
		int operator - (const const_StringIterator &i) const {
			return c - i.c;
		}
	};

	class String {
		StringCharacters *characters;

		void releaseCharacters();

		String (StringCharacters *s);

	#ifdef _DEBUG
		typedef std::vector <const_StringIterator *> Iterators;
		Iterators validIterators;
		void invalidateIterators () {
			for (Iterators::iterator i = validIterators.begin(); i != validIterators.end(); i ++)
				(*i)->invalidate();
			validIterators.clear();
		}
	#else
		void invalidateIterators() {}
	#endif

	protected:
		friend class Char;
		void setCharacter (int i, char c);
		char getCharacter (int i) const;

	public:
		typedef const_StringIterator const_iterator;

		// Empty string
		String();
		// Shallow copy
		String (const String &s);
		// Deep copy
		String (const char *s);
		// Deep copy
		String (const char *s, int length);
		// Deep copy
		explicit String (char c);
		// Deep copy
		String (std::vector<char> &v);
		// Deep copy
		String (const const_StringIterator &i1, const const_StringIterator &i2);

		// int as string
		explicit String (int i, int base = 10, int length = 1);
		explicit String (unsigned int i, int base = 10, int length = 1);
		explicit String (unsigned long i, int base = 10, int length = 1);

		~String();

		bool empty() const;
		int length() const;

		char operator[] (int index) const;
		Char operator[] (int index);
		const char *getChars() const;
		String substring (int first, int length) const;

		const_iterator begin() const;
		const_iterator end() const;

		int indexOf (const String &s) const;
		int indexOf (const char *s) const;
		int indexOf (char c) const;

		void deleteCharacters (int first, int num);

		String & operator = (const String &s);
		String & operator = (const char *s);
		String & operator = (char c);
		String & operator += (const String &s);
		String & operator += (const char *s);
		String & operator += (char c);

		String operator + (const String &s2) const;
		String operator + (const char *s2) const;
		String operator + (char c) const;

		int strcmp (const String &s2) const;

		bool operator == (const String &s2) const;
		bool operator == (const char *s2) const;
		bool operator == (char c) const;
		bool operator != (const String &s2) const { return !(*this == s2); }
		bool operator != (const char *s2) const { return !(*this == s2); }
		bool operator != (char c) const { return !(*this == c); }

		bool operator < (const String &s2) const;
		bool operator < (const char *s2) const;
		bool operator > (const String &s2) const;
		bool operator > (const char *s2) const;

		bool operator <= (const String &s2) const { return !(*this > s2); }
		bool operator <= (const char *s2) const { return !(*this > s2); }
		bool operator >= (const String &s2) const { return !(*this < s2); }
		bool operator >= (const char *s2) const { return !(*this < s2); }

	#ifdef CAST_STRING_TO_CONST_CHAR
		// Casting to const char has a worst-case complexity of n (ave. exp(n))
		// but it is extremely convenient for keeping code working with const char *
		// working.
		operator const char *();
	#endif // CAST_STRING_TO_CONST_CHAR
		// This works as well
		const char *getCString();

		void write(std::ostream &o) const;
	};

	class Char {
		String &s;
		int pos;
	protected:
		friend class String;
		Char (String &aS, int aPos);
	public:
		char operator = (char c);
		operator char () const;
	};

	//String operator + (const char *s1, const String &s2);
	//String operator + (char c, const String &s2);

	//std::ostream & operator << (std::ostream &o, const String& s);

	/*** StringCharacters ***/

	class StringCharacters {
		unsigned int refCount;
		int capacity;
		int length;
		char *characters;

		// Set the capacity to an initial value of at least minCapacity
		// Does not reserve memory.
		void setCapacity (int minCapacity);
		// Resize the capacity to a value of minCapacity.
		// Provides exponential behaviour.
		// Does not reserve memory.
		void resizeCapacity (int minCapacity);

	protected:
		StringCharacters (const char *s, int sLength);
		StringCharacters (const char *sFirst, const char *sLast);
		StringCharacters (char c);
		StringCharacters (std::vector<char> &v);

		// Get a StringCharacters object from a number
		StringCharacters (int i, int base, int length);
		StringCharacters (unsigned int i, int base, int length);

		// Produce a StringCharacters object with string s2 added to it
		StringCharacters (const StringCharacters &s1, const StringCharacters &s2);
		StringCharacters (const StringCharacters &s1, const char *s2, int s2Length);
		StringCharacters (const char *s1, int s1Length, const char *s2, int s2Length);

		~StringCharacters ();

		char at (int index) const;
		const char *getChars() const;
		const char *getCString();
		int getLength() const;

		// Return true if the strings are exactly the same
		bool equals (const StringCharacters &s2) const;
		bool equals (const char *s2, int s2Length) const;

		int strcmp (const StringCharacters &s2) const;
		int strcmp (const char *s2, int s2Length) const;

		StringCharacters *newSubstring (int first, int num);

		// append() may change the current object, or return a new one, depending
		// on the reference count.
		StringCharacters * append (const StringCharacters &s2);
		StringCharacters * append (const char *s, int size);
		StringCharacters * deleteCharacters (int first, int num);
		StringCharacters * set (int index, char c);

		int indexOf (const StringCharacters &s) const;
		int indexOf (const char *s, int sLength) const;

		// Add another owner
		void increaseRefCount();
		// Deletes itself if this was the last owner
		void release();

		friend class String;
	};


	inline StringCharacters::StringCharacters (const char *s, int sLength) {
		length = sLength;
		setCapacity (length);
		memcpy (characters, s, length);
		refCount = 1;
	}

	inline StringCharacters::StringCharacters (const char *sFirst, const char *sLast) {
		length = sLast - sFirst;
		setCapacity (length);
		memcpy (characters, sFirst, length);
		refCount = 1;
	}

	inline StringCharacters::StringCharacters (char c) {
		length = 1;
		setCapacity (length);
		characters [0] = c;
		refCount = 1;
	}

	inline StringCharacters::StringCharacters (const StringCharacters &s1, const StringCharacters &s2) {
		length = s1.length + s2.length;
		setCapacity (length);

		memcpy (characters, s1.characters, s1.length);
		memcpy (& characters [s1.length], s2.characters, s2.length);
		refCount = 1;
	}

	inline StringCharacters::StringCharacters (const StringCharacters &s1, const char *s2, int s2Length) {
		length = s1.length + s2Length;
		setCapacity (length);

		memcpy (characters, s1.characters, s1.length);
		memcpy (& characters [s1.length], s2, s2Length);
		refCount = 1;
	}

	inline StringCharacters::StringCharacters (const char *s1, int s1Length, const char *s2, int s2Length) {
		length = s1Length + s2Length;
		setCapacity (length);

		memcpy (characters, s1, s1Length);
		memcpy (& characters [s1Length], s2, s2Length);
		refCount = 1;
	}

	inline StringCharacters::StringCharacters (std::vector<char> &v) {
		length = v.size();
		setCapacity (length);

		typedef std::vector<char> CharVector;
		CharVector::iterator i;
		int j = 0;
		for (i = v.begin(); i != v.end(); ++ i) {
			characters [j] = *i;
			j ++;
		}
		refCount = 1;
	}

	inline StringCharacters::~StringCharacters () {
		assert (characters != NULL);
		delete[] characters;
	}

	// Private methods

	inline void StringCharacters::setCapacity (int minCapacity) {
		assert (minCapacity > 0);
		if (minCapacity < 0x80)
			capacity = (minCapacity + 0x07) & ~0x07;
		else
			capacity = (minCapacity + 0x3F) & ~0x3F;
		characters = new char [capacity];
	}

	inline void StringCharacters::resizeCapacity (int minCapacity) {
		if (capacity < minCapacity) {
			do {
				capacity *= 2;
			} while (capacity < minCapacity);
			char *newCharacters = new char [capacity];
			memcpy (newCharacters, characters, length);
			delete[] characters;
			characters = newCharacters;
		}
	}

	// Protected methods

	inline char StringCharacters::at (int index) const {
		assert (index < length);
		return characters[index];
	}

	inline const char *StringCharacters::getChars() const {
		return characters;
	}

	inline const char *StringCharacters::getCString() {
	#ifdef _DEBUG
		// Check whether this would give a valid C string
		int i;
		for (i = 0; i < length; i++)
			assert (characters [i] != '\0');
	#endif
		// Reserve one extra byte for the final '\0'
		resizeCapacity (length + 1);
		characters [length] = '\0';
		return characters;
	}

	inline int StringCharacters::getLength() const {
		return length;
	}

	inline StringCharacters * StringCharacters::append (const StringCharacters &s2) {
		return append (s2.characters, s2.length);
	}

	inline StringCharacters * StringCharacters::append (const char *s, int newSize) {
		// If the reference count is 1, i.e., this is owned by exactly one String,
		// the characters may be added to this. Otherwise a deep copy must be
		// returned.
		if (refCount == 1) {
			resizeCapacity (length + newSize);
			memcpy (& characters [length], s, newSize);
			length += newSize;
			return this;
		} else {
			// Get new object
			StringCharacters *newCharacters =  new StringCharacters (*this, s, newSize);
			release();
			return newCharacters;
		}
	}

	inline StringCharacters * StringCharacters::deleteCharacters (int first, int num) {
		if (num==0)
			return this;

		assert (first <= length);
		assert (first + num <= length);
		if (refCount == 1) {
			if (first + num < length) {
				memcpy(& characters [first], & characters [first + num], length - (first+num));
				length -= num;
			}
			return this;
		} else {
			// Get new object
			StringCharacters *newCharacters =
				new StringCharacters (characters, first, & characters[first + num], length - (first+num));
			release();
			return newCharacters;
		}
	}

	inline StringCharacters * StringCharacters::set (int index, char c) {
		assert (index < length);

		if (refCount == 1) {
			characters [index] = c;
			return this;
		}else {
			// Get new object
			StringCharacters *newCharacters =
				new StringCharacters (characters, length);
			newCharacters->characters [index] = c;
			release();
			return newCharacters;
		}
	}


	// Return true if the strings are exactly the same
	inline bool StringCharacters::equals (const StringCharacters &s2) const {
		return this == &s2 || equals (s2.characters, s2.length);
	}

	inline bool StringCharacters::equals (const char *s2, int s2Length) const {
		if (length != s2Length)
			return false;
		else
			return memcmp (characters, s2, length) == 0;
	}

	inline int StringCharacters::strcmp (const StringCharacters &s2) const {
		if (this == &s2)
			return 0;
		else
			return strcmp (s2.characters, s2.length);
	}


	inline int StringCharacters::strcmp (const char *s2, int s2Length) const {
		if (length == s2Length)
			return memcmp (characters, s2, length);
		else {
			if (length < s2Length) {
				int compare = memcmp (characters, s2, length);
				return compare ? compare : -1;
			} else {
				int compare = memcmp (characters, s2, s2Length);
				return compare ? compare : 1;
			}
		}
	}


		
	inline StringCharacters *StringCharacters::newSubstring (int first, int num) {
		assert (first < length);
		assert (first + num <= length);
		return new StringCharacters (& characters [first], num);
	}

	inline int StringCharacters::indexOf (const StringCharacters &s) const {
		return indexOf (s.characters, s.length);
	}

	inline int StringCharacters::indexOf (const char *s, int sLength) const {
		if (sLength == 0)
			return 0;

		const char *curStart = characters;
		while (curStart + sLength < characters + length) {
			const char *cur = curStart;
			const char *curEnd = cur + sLength;
			const char *sCur = s;
			while (*cur == *sCur) {
				cur ++;
				sCur ++;
				if (cur == curEnd)
					return curStart - characters;
			}
			
			curStart ++;
		}
		return -1;
	}


	inline void StringCharacters::increaseRefCount() {
		refCount ++;
	}

	inline void StringCharacters::release() {
		refCount --;
		if (refCount == 0)
			delete this;
	}


	/*** String ***/

	inline String::String() {
		characters = NULL;
	}

	inline String::String(StringCharacters *s) {
		characters = s;
	}

	// Shallow copy
	inline String::String (const String &s) {
		characters = s.characters;
		if (characters)
			characters->increaseRefCount();
	}

	inline String::String (const char *s) {
		if (s == NULL || *s == '\0')
			characters = NULL;
		else
			characters = new StringCharacters (s, strlen(s));
	}

	inline String::String (const char *s, int length) {
		if (s == NULL || length == 0)
			characters = NULL;
		else
			characters = new StringCharacters (s, length);
	}

	inline String::String (char c) {
		characters = new StringCharacters (c);
	}

	inline String::String (std::vector<char> &v) {
		if (v.empty())
			characters = NULL;
		else
			characters = new StringCharacters (v);
	}

	inline String::String (const const_StringIterator &i1, const const_StringIterator &i2) {
	#ifdef _DEBUG
		assert (i1.valid);
		assert (i2.valid);
	#endif
		if (i1 == i2)
			characters = NULL;
		else
			characters = new StringCharacters (i1.c, i2.c);
	}

	inline String::String (int i, int base, int length) {
		characters = new StringCharacters (i, base, length);
	}

	inline String::String (unsigned int i, int base, int length) {
		characters = new StringCharacters (i, base, length);
	}

	inline String::String (unsigned long i, int base, int length) {
		// Just a workaround when `size_t` is not an `unsigned int`.
		// Assume `i` is not very large.
		characters = new StringCharacters ((unsigned)i, base, length);
	}

	inline String::~String() {
		releaseCharacters ();
	}

	inline void String::releaseCharacters() {
		if (characters) {
			characters->release();
			characters = NULL;
		}
	}


	// Protected method

	inline void String::setCharacter (int i, char c) {
		invalidateIterators();
		assert (characters != NULL);
		characters = characters->set (i, c);
	}

	inline char String::getCharacter (int i) const {
		assert (characters != NULL);
		return characters->at (i);
	}


	// Public methods


	inline bool String::empty() const {
		return (characters == 0);
	}

	inline int String::length() const {
		if (characters)
			return characters->getLength();
		else
			return 0;
	}


	inline char String::operator [] (int index) const {
		assert (characters != NULL);
		return characters->at(index);
	}

	inline Char String::operator [] (int index) {
		assert (characters != NULL);
		return Char (*this, index);
	}

	inline const char *String::getChars() const {
		if (characters)
			return characters->getChars();
		else
			return 0;
	}

	inline String String::substring (int first, int num) const {
		if (num == 0)
			return String();
		assert (characters != NULL);
		int length = this->length();
		assert (first < length);
		if (num < 0 || first + num > length)
			num = length - first;

		return String (characters->newSubstring (first, num));
	}

	inline const_StringIterator String::begin() const {
		if (characters)
			return const_StringIterator (characters->getChars());
		else
			return const_StringIterator (NULL);
	}

	inline const_StringIterator String::end() const {
		if (characters)
			return const_StringIterator (characters->getChars() + characters->getLength());
		else
			return const_StringIterator (NULL);
	}

	inline int String::indexOf (const String &s) const {
		if (characters)
			return characters->indexOf(*s.characters);
		else
			return -1;
	}

	inline int String::indexOf (const char *s) const {
		if (characters)
			return characters->indexOf (s, strlen(s));
		else
			return -1;
	}

	inline int String::indexOf (char c) const {
		if (characters)
			return characters->indexOf (&c, 1);
		else
			return -1;
	}

	/*** String changing operators ***/

	inline void String::deleteCharacters (int first, int num) {
		invalidateIterators();
		assert (characters != NULL);
		characters = characters->deleteCharacters (first, num);
	}

	// Shallow copy
	inline String & String::operator = (const String &s) {
		invalidateIterators();
		releaseCharacters();
		if (s.characters) {
			characters = s.characters;
			characters->increaseRefCount();
		}
		return *this;
	}

	// Deep copy
	inline String & String::operator = (const char *s) {
		invalidateIterators();
		releaseCharacters();
		if (*s)
			characters = new StringCharacters (s, strlen(s));
		return *this;
	}

	inline String & String::operator = (char c) {
		invalidateIterators();
		releaseCharacters();
		characters = new StringCharacters (c);
		return *this;
	}

	// Deep copy or shallow copy
	inline String & String::operator += (const String &s) {
		invalidateIterators();
		if (s.characters) {
			if (characters)
				characters = characters->append (*s.characters);
			else {
				characters = s.characters;
				characters->increaseRefCount();
			}
		}
		return *this;
	}

	inline String & String::operator += (const char *s) {
		invalidateIterators();
		assert (s != NULL);
		if (*s) {
			if (characters)
				characters = characters->append (s, strlen (s));
			else
				characters = new StringCharacters (s, strlen(s));
		}
		return *this;
	}

	inline String & String::operator += (char c) {
		invalidateIterators();
		if (characters)
			characters = characters->append (&c, 1);
		else
			characters = new StringCharacters (c);
		return *this;
	}

	/*** String returning operators ***/

	inline String String::operator + (const String &s2) const {
		if (characters) {
			if (s2.characters)
				return String (new StringCharacters (*characters, *s2.characters));
			else
				return String (*this);
		} else {
			if (s2.characters)
				return String (s2);
			else
				return String();
		}
	}

	inline String String::operator + (const char *s2) const {
		if (characters) {
			if (s2 != NULL && *s2 != '\0')
				return String (new StringCharacters (*characters, s2, strlen(s2)));
			else
				return String (*this);
		} else {
			if (s2 != NULL && *s2 != '\0')
				return String (s2);
			else
				return String();
		}
	}

	inline String String::operator + (char c) const {
		if (characters)
			return String (new StringCharacters (*characters, c));
		else
			return String (c);
	}


	/*** String testing operators ***/

	inline int String::strcmp (const String &s2) const {
		if (characters) {
			if (s2.characters)
				return characters->strcmp (*s2.characters);
			else
				return 0;
		} else {
			if (s2.characters)
				return -1;
			else
				return 0;
		}
	}

	inline bool String::operator == (const String &s2) const {
		if (characters) {
			if (s2.characters)
				return characters->equals (*s2.characters);
			else
				return true;
		} else
			return s2.characters == NULL;
	}

	inline bool String::operator == (const char *s2) const {
		assert (s2 != NULL);
		if (characters)
			return characters->equals (s2, strlen (s2));
		else
			return *s2 == '\0';
	}

	inline bool String::operator == (char c) const {
		if (characters)
			return characters->equals (&c, 1);
		else
			return false;
	}

	inline bool String::operator < (const String &s2) const {
		if (characters) {
			if (s2.characters)
				return characters->strcmp (*s2.characters) < 0;
			else
				return false;
		} else
			return s2.characters != NULL;
	}

	inline bool String::operator < (const char *s2) const {
		assert (s2 != NULL);
		if (characters)
			return characters->strcmp (s2, strlen (s2)) < 0;
		else
			return *s2 != '\0';
	}

	inline bool String::operator > (const String &s2) const {
		if (characters) {
			if (s2.characters)
				return characters->strcmp (*s2.characters) > 0;
			else
				return false;
		} else
			return false;
	}

	inline bool String::operator > (const char *s2) const {
		assert (s2 != NULL);
		if (characters)
			return characters->strcmp (s2, strlen (s2)) < 0;
		else
			return false;
	}

	inline const char *String::getCString() {
		invalidateIterators();
		static const char emptyString = '\0';
		if (characters)
			return characters->getCString();
		else
			return &emptyString;
	}

	#ifdef CAST_STRING_TO_CONST_CHAR
	inline String::operator const char *() {
		return getCString();
	}
	#endif


	inline void String::write (std::ostream &o) const {
		// Too slow, probably.
		if (characters) {
			int i;
			for (i = 0; i < characters->getLength(); i++)
				o << characters->at (i);
		}
	}

	/*** Char ***/

	inline Char::Char (String &aS, int aPos) : s (aS), pos (aPos) {}

	inline char Char::operator = (char c) {
		s.setCharacter (pos, c);
		return c;
	}

	inline Char::operator char () const {
		return s.getCharacter (pos);
	}


	/*** operators ***/

	inline String operator + (const char *s1, const String &s2) {
		return String (s1) + s2;
	}

	inline String operator + (char c, const String &s2) {
		return String (c) + s2;
	}

	inline std::ostream & operator << (std::ostream &o, const String &s) {
		s.write(o);
		return o;
	}

} // namespace util

#endif	// STRING_H
