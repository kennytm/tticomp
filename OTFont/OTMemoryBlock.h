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

// Some comments in this file may be parsed by doygen to yield nice
// documentation files for the source.

/**
	\file OTMemoryBlock provides a block of memory that may be used to write
	OpenType tables with.
	OTMemoryPen is used to traverse the memory block and to read and write
	using short-endian/long-endian conversion.
*/

#ifndef OTMEMORYBLOCK_H
#define OTMEMORYBLOCK_H

#include <fstream>

#include "../Util/String.h"
#include "../Util/smart_ptr.h"
#include "OpenType.h"

namespace OpenType {

	/*** MemoryBlock ***/

	class MemoryBlock {
		void setCapacity (ULong newSize);
		void resizeCapacity (ULong newSize);
		ULong padWithZeros (ULong padding);

	protected:
		Byte *memory;
		ULong capacity;
		ULong size;

		void resize (ULong newSize);
		friend class MemoryPen;
		friend class MemoryWritePen;

	public:
		MemoryBlock (ULong aSize = 0);
		MemoryBlock (const Byte *aMemory, ULong aSize);
		MemoryBlock (std::ifstream &file, ULong aSize);
		virtual ~MemoryBlock();

		bool empty() const {
			assert (memory == NULL || size != 0);
			return memory == NULL;
		}

		ULong getSize () const {
			return size;
		}

		ULong getSize (ULong padding) const {
			return (size + padding - 1) & ~(padding-1);
		}

		ULong getChecksum();
		void write (std::ofstream &file, ULong padding);
	};

	typedef util::smart_ptr <MemoryBlock> MemoryBlockPtr;

	/*** MemoryPen ***/

	class MemoryPen {
	protected:
		MemoryBlockPtr block;
		ULong position;

		void throwReadException (ULong byteNum) const;
		void throwWriteException (ULong byteNum) const;

		explicit MemoryPen (MemoryBlockPtr aBlock, ULong position);

	public:
		MemoryPen() {}
		explicit MemoryPen (MemoryBlockPtr aBlock);
		MemoryPen (const MemoryPen &p);
		virtual ~MemoryPen();

		MemoryPen & operator = (const MemoryPen & p);
		MemoryPen & operator += (Long deltaPosition);
		MemoryPen & operator -= (Long deltaPosition);
		MemoryPen operator + (Long deltaPosition) const;
		MemoryPen operator - (Long deltaPosition) const;
		Long operator - (const MemoryPen &pen) const;
		
		ULong getPosition() const;
		bool endOfBlock() const;
		void set (MemoryBlockPtr aBlock);
		
		/// Read one LongLong value using short-endian conversion
		LongLong readLongLong();

		/// Read one ULong value using short-endian conversion
		ULong readULong() {
			if (position + 4 <= block->size) {
				ULong value = *((ULong *) (block->memory + position));
				position += 4;
				return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |
					((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24);
			} else {
				throwReadException (4);
				return 0;
			}
		}
		
		/// Read one UShort value using short-endian conversion
		UShort readUShort() {
			if (position + 2 <= block->size) {
				UShort value = *((UShort *) (block->memory + position));
				position += 2;
				return ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
			} else {
				throwReadException (2);
				return 0;
			}
		}

		/// Read one Byte value using short-endian conversion
		Byte readByte() {
			if (position + 1 <= block->getSize()) {
				Byte value = *(block->memory + position);
				position += 1;
				return value;
			} else {
				throwReadException (1);
				return 0;
			}
		}

		/// Read one Tag value, _not_ using short-endian conversion
		Tag readTag() {
			if (position + 4 <= block->size) {
				Tag value = *((Tag *) (block->memory + position));
				position += 4;
				return value;
			} else {
				throwReadException (4);
				return 0;
			}
		}
		
		util::String readString (int length);
		MemoryBlockPtr readBlock (int length);

		LongDateTime readLongDateTime() { return readLongLong(); }
		Fixed readFixed() { return (Fixed) readULong(); }
		Long readLong() { return (Long) readULong(); }
		Short readShort() { return (Short) readUShort(); }
		FWord readFWord() { return (FWord) readUShort(); }
		UFWord readUFWord() { return readUShort(); }
		Offset readOffset() { return readUShort(); }
		GlyphId readGlyphId() { return readUShort(); }
		SignedByte readSignedByte() { return (SignedByte) readByte(); }
		F2Dot14 readF2Dot14() { return F2Dot14 (readShort(), util::fixed_fraction()); }
	};
	
	/*** MemoryWritePen ***/
	
	class MemoryWritePen : public MemoryPen {
	protected:
		explicit MemoryWritePen (MemoryBlockPtr aBlock, ULong position);
	public:
		MemoryWritePen() {}
		explicit MemoryWritePen (MemoryBlockPtr aBlock);
		MemoryWritePen (const MemoryWritePen &p);
		virtual ~MemoryWritePen();
		
		MemoryWritePen & operator = (const MemoryWritePen & p);
		MemoryWritePen & operator += (Long deltaPosition);
		MemoryWritePen & operator -= (Long deltaPosition);
		MemoryWritePen operator + (Long deltaPosition) const;
		MemoryWritePen operator - (Long deltaPosition) const;
		Long operator - (const MemoryPen &pen) const;

		/// Write one LongLong value using short-endian conversion
		void writeLongLong (LongLong value);

		/// Write one ULong value using short-endian conversion
		void writeULong (ULong value) {
			if (position + 4 > block->size)
				block->resize (position + 4);
			*((ULong *) (block->memory + position)) =
				((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |
				((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24);
			position += 4;
		}

		/// Write one UShort value using short-endian conversion
		void writeUShort (UShort value) {
			if (position + 2 > block->size)
				block->resize (position + 2);
			*((UShort *) (block->memory + position)) =
				((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
			position += 2;
		}

		/// Write one Byte value using short-endian conversion
		void writeByte (Byte value) {
			if (position + 1 > block->size)
				block->resize (position + 1);
			*(block->memory + position) = value;
			position += 1;
		}

		/// Write one Tag value, _not_ using short-endian conversion
		void writeTag (Tag value) {
			if (position + 4 > block->size)
				block->resize (position + 4);
			*((Tag *) (block->memory + position)) = value;
			position += 4;
		}

		void writeString (const util::String & s);
		void writeBlock (MemoryBlockPtr b);

		void writeLongDateTime (LongDateTime value) { writeLongLong (value); }
		void writeFixed (Fixed value) { writeULong ((Fixed) value); }
		void writeLong (Long value) { writeULong ((ULong) value); }
		void writeShort (Short value) { writeUShort ((UShort) value); }
		void writeFWord (FWord value) { writeUShort ((UShort) value); }
		void writeUFWord (UFWord value) { writeUShort (value); }
		void writeOffset (Offset value) { writeUShort (value); }
		void writeGlyphId (GlyphId value) { writeUShort (value); }
		void writeSignedByte (SignedByte value) { writeByte ((Byte) value); }
		void writeF2Dot14 (F2Dot14 value) { writeShort (value.get_i()); }
		
		void padWithZeros (ULong padding);
	};
}

#endif	// OTMEMORYBLOCK_H
