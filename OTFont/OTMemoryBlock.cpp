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

#include "OTMemoryBlock.h"
#include "OTException.h"
#include <vector>

using std::ifstream;
using std::ofstream;

using util::String;

namespace OpenType {

MemoryBlock::MemoryBlock(ULong aSize) : size(aSize) {
	setCapacity (size);
}

MemoryBlock::MemoryBlock (const Byte *aMemory, ULong aSize) : size (aSize) {
	setCapacity (size);
	memcpy (memory, aMemory, aSize);
}

MemoryBlock::MemoryBlock (ifstream &file, ULong aSize) : size (aSize) {
	setCapacity (size);
	if ((unsigned)file.rdbuf()->sgetn ((char *) memory, size) != size) {
		// Delete memory because it might not be automatically deleted
		// when an exception is thrown in the constructor...
		// (SmartPtr will never be initialised)
		delete [] memory;
		memory = 0;
		capacity = 0;
		size = 0;
		throw Exception ("Could not read file");
	}
}

MemoryBlock::~MemoryBlock() {
	if (memory)
		delete [] memory;
}

void MemoryBlock::write (ofstream &file, ULong padding) {
	ULong padSize = padWithZeros (padding);

	if ((unsigned) file.rdbuf()->sputn ((char *) memory, padSize) != padSize)
		throw Exception ("Could not write file");
}


void MemoryBlock::setCapacity (ULong newSize) {
	if (newSize) {
		capacity = (newSize + 7) & ~(ULong (7));
		memory = new Byte [capacity];
	} else {
		capacity = 0;
		memory = NULL;
	}
}

void MemoryBlock::resizeCapacity (ULong newSize) {
	if (newSize > capacity) {
		if (capacity < 8) capacity = 8;
		while (newSize > capacity) {
			capacity *= 2;
		}
		Byte * newMemory = new Byte [capacity];
		memcpy (newMemory, memory, size);
		delete [] memory;
		memory = newMemory;
	}
}

void MemoryBlock::resize (ULong newSize) {
	resizeCapacity (newSize);
	size = newSize;
}

ULong MemoryBlock::padWithZeros (ULong padding) {
	// Calculate padding size
	ULong padSize = getSize(padding);
	if (padSize) {
		resizeCapacity (padSize);

		// pad with zeroes
		ULong i = padSize-1;
		while (i >= size) {
			memory [i] = '\0';
			i --;
		}
	}

	return padSize;
}

ULong MemoryBlock::getChecksum() {
	ULong checksum = 0;
	ULong *cur = (ULong *) (memory + padWithZeros (4));
	while ((Byte *)cur != memory) {
		cur --;
		ULong value = *cur;
		checksum += ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |
			((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24);
	}
	return checksum;
}

/*** MemoryPen ***/

MemoryPen::MemoryPen (MemoryBlockPtr aBlock)
: block (aBlock), position (0) {}

MemoryPen::MemoryPen (MemoryBlockPtr aBlock, ULong aPosition)
: block (aBlock), position (aPosition) {}

MemoryPen::MemoryPen (const MemoryPen &p)
: block (p.block), position (p.position) {}

MemoryPen::~MemoryPen() {}

void MemoryPen::set (MemoryBlockPtr aBlock) {
	block = aBlock;
	position = 0;
}

MemoryPen & MemoryPen::operator = (const MemoryPen & p) {
	block = p.block;
	position = p.position;
	return *this;
}

MemoryPen & MemoryPen::operator += (Long deltaPosition) {
	position += deltaPosition;
	return *this;
}

MemoryPen & MemoryPen::operator -= (Long deltaPosition) {
	position -= deltaPosition;
	return *this;
}

MemoryPen MemoryPen::operator + (Long deltaPosition) const {
	return MemoryPen (block, position + deltaPosition);
}

MemoryPen MemoryPen::operator - (Long deltaPosition) const {
	return MemoryPen (block, position - deltaPosition);
}

Long MemoryPen::operator - (const MemoryPen &pen) const {
	assert (block == pen.block);
	return position - pen.position;
}

ULong MemoryPen::getPosition() const {
	return position;
}

bool MemoryPen::endOfBlock() const {
	return position == block->size;
}

void MemoryPen::throwReadException (ULong byteNum) const {
	throw Exception ("Bounds checking error while reading " + String (byteNum) +
		" byte" + (byteNum ? "s" : "") + " from position " + String (position));
}

void MemoryPen::throwWriteException (ULong byteNum) const {
	throw Exception ("Bounds checking error while writing " + String (byteNum) +
		" byte" + (byteNum ? "s" : "") + " to position " + String (position));
}

LongLong MemoryPen::readLongLong() {
	if (position + 8 <= block->size) {
		LongLong value = *((ULong *) (block->memory + position));
		position += 8;
		return ((value & 0xFF) << 56) | ((value & 0xFF00) << 40) |
			((value & 0xFF0000) << 24) | ((value & 0xFF000000) << 8) |
#ifdef __GNUC__
			((value & 0xFF00000000LL) >> 8) | ((value & 0xFF0000000000LL) >> 24) |
			((value & 0xFF000000000000LL) >> 40) | ((value & 0xFF00000000000000LL) >> 56);
#else
			((value & 0xFF00000000) >> 8) | ((value & 0xFF0000000000) >> 24) |
			((value & 0xFF000000000000) >> 40) | ((value & 0xFF00000000000000) >> 56);
#endif
	} else {
		throwReadException (8);
		return 0;
	}
}

String MemoryPen::readString (int length) {
	if (position + length <= block->size) {
		String value ((char *) block->memory + position, length);
		position += length;
		return value;
	} else {
		throwReadException (length);
		return String();
	}
}

MemoryBlockPtr MemoryPen::readBlock (int length) {
	if (position + length <= block->size) {
		MemoryBlockPtr value = new MemoryBlock (block->memory + position, length);
		position += length;
		return value;
	} else {
		throwReadException (length);
		return 0;
	}
}

/*** MemoryWritePen ***/

MemoryWritePen::MemoryWritePen (MemoryBlockPtr aBlock) : MemoryPen (aBlock) {}

MemoryWritePen::MemoryWritePen (MemoryBlockPtr aBlock, ULong aPosition) : MemoryPen (aBlock, aPosition) {}

MemoryWritePen::MemoryWritePen (const MemoryWritePen &p) : MemoryPen (p) {}

MemoryWritePen::~MemoryWritePen() {}

MemoryWritePen & MemoryWritePen::operator = (const MemoryWritePen & p) {
	MemoryPen::operator = (p);
	return *this;
}

MemoryWritePen & MemoryWritePen::operator += (Long deltaPosition) {
	position += deltaPosition;
	return *this;
}

MemoryWritePen & MemoryWritePen::operator -= (Long deltaPosition) {
	position -= deltaPosition;
	return *this;
}

MemoryWritePen MemoryWritePen::operator + (Long deltaPosition) const {
	return MemoryWritePen (block, position + deltaPosition);
}

MemoryWritePen MemoryWritePen::operator - (Long deltaPosition) const {
	return MemoryWritePen (block, position - deltaPosition);
}

Long MemoryWritePen::operator - (const MemoryPen &pen) const {
	return MemoryPen::operator - (pen);
}

void MemoryWritePen::writeLongLong (LongLong value) {
	if (position + 8 > block->size)
		block->resize (position + 8);
	*((LongLong *) (block->memory + position)) =
		((value & 0xFF) << 56) | ((value & 0xFF00) << 40) |
			((value & 0xFF0000) << 24) | ((value & 0xFF000000) << 8) |
#ifdef __GNUC__
			((value & 0xFF00000000LL) >> 8) | ((value & 0xFF0000000000LL) >> 24) |
			((value & 0xFF000000000000LL) >> 40) | ((value & 0xFF00000000000000LL) >> 56);
#else
			((value & 0xFF00000000) >> 8) | ((value & 0xFF0000000000) >> 24) |
			((value & 0xFF000000000000) >> 40) | ((value & 0xFF00000000000000) >> 56);
#endif
	position += 8;
}

void MemoryWritePen::writeString (const String & s) {
	int length = s.length();
	if (length) {
		if (position + length > block->size)
			block->resize (position + length);
		memcpy (block->memory + position, s.getChars(), length);
		position += length;
	}
}

void MemoryWritePen::writeBlock (MemoryBlockPtr b) {
	if (b->size) {
		if (position + b->size > block->size)
			block->resize (position + b->size);
		memcpy (block->memory + position, b->memory, b->size);
		position += b->size;
	}
}

void MemoryWritePen::padWithZeros (ULong padding) {
	ULong paddingLength = OT_PAD_TO (position, padding) - position;
	if (paddingLength) {
		if (position + paddingLength > block->size)
			block->resize (position + paddingLength);
		memset (block->memory + position, 0, paddingLength);
		position += paddingLength;
	}
}

} // end namespace OpenType
