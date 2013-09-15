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

#include <algorithm>
#include "OTException.h"
#include "OTpostTable.h"
#include "OTGlyph.h"
#include "OTTags.h"

using std::vector;
using std::find;

using util::String;

namespace OpenType {

#define MACINTOSH_SET_SIZE 258

const char macGlyphs[][MACINTOSH_SET_SIZE] = {
	/* 0 */ ".notdef",
	/* 1 */ ".null",
	/* 2 */ "nonmarkingreturn",
	/* 3 */ "space",
	/* 4 */ "exclam",
	/* 5 */ "quotedbl",
	/* 6 */ "numbersign",
	/* 7 */ "dollar",
	/* 8 */ "percent",
	/* 9 */ "ampersand",
	/* 10 */ "quotesingle",
	/* 11 */ "parenleft",
	/* 12 */ "parenright",
	/* 13 */ "asterisk",
	/* 14 */ "plus",
	/* 15 */ "comma",
	/* 16 */ "hyphen",
	/* 17 */ "period",
	/* 18 */ "slash",
	/* 19 */ "zero",
	/* 20 */ "one",
	/* 21 */ "two",
	/* 22 */ "three",
	/* 23 */ "four",
	/* 24 */ "five",
	/* 25 */ "six",
	/* 26 */ "seven",
	/* 27 */ "eight",
	/* 28 */ "nine",
	/* 29 */ "colon",
	/* 30 */ "semicolon",
	/* 31 */ "less",
	/* 32 */ "equal",
	/* 33 */ "greater",
	/* 34 */ "question",
	/* 35 */ "at",
	/* 36 */ "A",
	/* 37 */ "B",
	/* 38 */ "C",
	/* 39 */ "D",
	/* 40 */ "E",
	/* 41 */ "F",
	/* 42 */ "G",
	/* 43 */ "H",
	/* 44 */ "I",
	/* 45 */ "J",
	/* 46 */ "K",
	/* 47 */ "L",
	/* 48 */ "M",
	/* 49 */ "N",
	/* 50 */ "O",
	/* 51 */ "P",
	/* 52 */ "Q",
	/* 53 */ "R",
	/* 54 */ "S",
	/* 55 */ "T",
	/* 56 */ "U",
	/* 57 */ "V",
	/* 58 */ "W",
	/* 59 */ "X",
	/* 60 */ "Y",
	/* 61 */ "Z",
	/* 62 */ "bracketleft",
	/* 63 */ "backslash",
	/* 64 */ "bracketright",
	/* 65 */ "asciicircum",
	/* 66 */ "underscore",
	/* 67 */ "grave",
	/* 68 */ "a",
	/* 69 */ "b",
	/* 70 */ "c",
	/* 71 */ "d",
	/* 72 */ "e",
	/* 73 */ "f",
	/* 74 */ "g",
	/* 75 */ "h",
	/* 76 */ "i",
	/* 77 */ "j",
	/* 78 */ "k",
	/* 79 */ "l",
	/* 80 */ "m",
	/* 81 */ "n",
	/* 82 */ "o",
	/* 83 */ "p",
	/* 84 */ "q",
	/* 85 */ "r",
	/* 86 */ "s",
	/* 87 */ "t",
	/* 88 */ "u",
	/* 89 */ "v",
	/* 90 */ "w",
	/* 91 */ "x",
	/* 92 */ "y",
	/* 93 */ "z",
	/* 94 */ "braceleft",
	/* 95 */ "bar",
	/* 96 */ "braceright",
	/* 97 */ "asciitilde",
	/* 98 */ "Adieresis",
	/* 99 */ "Aring",
	/* 100 */ "Ccedilla",
	/* 101 */ "Eacute",
	/* 102 */ "Ntilde",
	/* 103 */ "Odieresis",
	/* 104 */ "Udieresis",
	/* 105 */ "aacute",
	/* 106 */ "agrave",
	/* 107 */ "acircumflex",
	/* 108 */ "adieresis",
	/* 109 */ "atilde",
	/* 110 */ "aring",
	/* 111 */ "ccedilla",
	/* 112 */ "eacute",
	/* 113 */ "egrave",
	/* 114 */ "ecircumflex",
	/* 115 */ "edieresis",
	/* 116 */ "iacute",
	/* 117 */ "igrave",
	/* 118 */ "icircumflex",
	/* 119 */ "idieresis",
	/* 120 */ "ntilde",
	/* 121 */ "oacute",
	/* 122 */ "ograve",
	/* 123 */ "ocircumflex",
	/* 124 */ "odieresis",
	/* 125 */ "otilde",
	/* 126 */ "uacute",
	/* 127 */ "ugrave",
	/* 128 */ "ucircumflex",
	/* 129 */ "udieresis",
	/* 130 */ "dagger",
	/* 131 */ "degree",
	/* 132 */ "cent",
	/* 133 */ "sterling",
	/* 134 */ "section",
	/* 135 */ "bullet",
	/* 136 */ "paragraph",
	/* 137 */ "germandbls",
	/* 138 */ "registered",
	/* 139 */ "copyright",
	/* 140 */ "trademark",
	/* 141 */ "acute",
	/* 142 */ "dieresis",
	/* 143 */ "notequal",
	/* 144 */ "AE",
	/* 145 */ "Oslash",
	/* 146 */ "infinity",
	/* 147 */ "plusminus",
	/* 148 */ "lessequal",
	/* 149 */ "greaterequal",
	/* 150 */ "yen",
	/* 151 */ "mu",
	/* 152 */ "partialdiff",
	/* 153 */ "summation",
	/* 154 */ "product",
	/* 155 */ "pi",
	/* 156 */ "integral",
	/* 157 */ "ordfeminine",
	/* 158 */ "ordmasculine",
	/* 159 */ "Omega",
	/* 160 */ "ae",
	/* 161 */ "oslash",
	/* 162 */ "questiondown",
	/* 163 */ "exclamdown",
	/* 164 */ "logicalnot",
	/* 165 */ "radical",
	/* 166 */ "florin",
	/* 167 */ "approxequal",
	/* 168 */ "Delta",
	/* 169 */ "guillemotleft",
	/* 170 */ "guillemotright",
	/* 171 */ "ellipsis",
	/* 172 */ "nonbreakingspace",
	/* 173 */ "Agrave",
	/* 174 */ "Atilde",
	/* 175 */ "Otilde",
	/* 176 */ "OE",
	/* 177 */ "oe",
	/* 178 */ "endash",
	/* 179 */ "emdash",
	/* 180 */ "quotedblleft",
	/* 181 */ "quotedblright",
	/* 182 */ "quoteleft",
	/* 183 */ "quoteright",
	/* 184 */ "divide",
	/* 185 */ "lozenge",
	/* 186 */ "ydieresis",
	/* 187 */ "Ydieresis",
	/* 188 */ "fraction",
	/* 189 */ "currency",
	/* 190 */ "guilsinglleft",
	/* 191 */ "guilsinglright",
	/* 192 */ "fi",
	/* 193 */ "fl",
	/* 194 */ "daggerdbl",
	/* 195 */ "periodcentered",
	/* 196 */ "quotesinglbase",
	/* 197 */ "quotedblbase",
	/* 198 */ "perthousand",
	/* 199 */ "Acircumflex",
	/* 200 */ "Ecircumflex",
	/* 201 */ "Aacute",
	/* 202 */ "Edieresis",
	/* 203 */ "Egrave",
	/* 204 */ "Iacute",
	/* 205 */ "Icircumflex",
	/* 206 */ "Idieresis",
	/* 207 */ "Igrave",
	/* 208 */ "Oacute",
	/* 209 */ "Ocircumflex",
	/* 210 */ "apple",
	/* 211 */ "Ograve",
	/* 212 */ "Uacute",
	/* 213 */ "Ucircumflex",
	/* 214 */ "Ugrave",
	/* 215 */ "dotlessi",
	/* 216 */ "circumflex",
	/* 217 */ "tilde",
	/* 218 */ "macron",
	/* 219 */ "breve",
	/* 220 */ "dotaccent",
	/* 221 */ "ring",
	/* 222 */ "cedilla",
	/* 223 */ "hungarumlaut",
	/* 224 */ "ogonek",
	/* 225 */ "caron",
	/* 226 */ "Lslash",
	/* 227 */ "lslash",
	/* 228 */ "Scaron",
	/* 229 */ "scaron",
	/* 230 */ "Zcaron",
	/* 231 */ "zcaron",
	/* 232 */ "brokenbar",
	/* 233 */ "Eth",
	/* 234 */ "eth",
	/* 235 */ "Yacute",
	/* 236 */ "yacute",
	/* 237 */ "Thorn",
	/* 238 */ "thorn",
	/* 239 */ "minus",
	/* 240 */ "multiply",
	/* 241 */ "onesuperior",
	/* 242 */ "twosuperior",
	/* 243 */ "threesuperior",
	/* 244 */ "onehalf",
	/* 245 */ "onequarter",
	/* 246 */ "threequarters",
	/* 247 */ "franc",
	/* 248 */ "Gbreve",
	/* 249 */ "gbreve",
	/* 250 */ "Idotaccent",
	/* 251 */ "Scedilla",
	/* 252 */ "scedilla",
	/* 253 */ "Cacute",
	/* 254 */ "cacute",
	/* 255 */ "Ccaron",
	/* 256 */ "ccaron",
	/* 257 */ "dcroat" };

postTable::postTable (OpenTypeFile &aFont, MemoryBlockPtr memory) : Table (aFont)
{
	MemoryPen pen (memory);
	version = pen.readFixed();
	if (version != 0x00020000 && version != 0x00030000)
		throw Exception ("Unsupported table version: " + String (version, 16));
	italicAngle = pen.readFixed();
	underlinePosition = pen.readFWord();
	underlineThickness = pen.readFWord();
	isFixedPitch = pen.readULong();
	minMemType42 = pen.readULong();
	maxMemType42 = pen.readULong();
	minMemType1 = pen.readULong();
	maxMemType1 = pen.readULong();

	if (version == 0x00020000) {
		// Read glyph names
		UShort glyphNum = pen.readUShort();
		typedef vector<UShort> IndexVector;
		IndexVector glyphNameIndices;
		glyphNameIndices.reserve (glyphNum);
		UShort i;
		for (i = 0; i < glyphNum; i++)
			glyphNameIndices.push_back (pen.readUShort());

		vector<String> extraNames;
		IndexVector::iterator index;
		for (index = glyphNameIndices.begin(); index != glyphNameIndices.end(); index ++) {
			if (*index < MACINTOSH_SET_SIZE)
				postNames.push_back (macGlyphs [*index]);
			else {
				UShort extraIndex = *index - MACINTOSH_SET_SIZE;
				while (extraNames.size() <= extraIndex) {
					Byte nameLength = pen.readByte();
					extraNames.push_back (pen.readString (nameLength));
				}
				postNames.push_back (extraNames [extraIndex]);
			}
		}
	}
}

postTable::~postTable() {}

ULong postTable::getTag() const {
	return postTag;
}

Short getMacIndex (String name) {
	Short i;
	for (i = 0; i < MACINTOSH_SET_SIZE; i++) {
		if (name == macGlyphs [i])
			return i;
	}
	return -1;
}

MemoryBlockPtr postTable::getMemory () const {
	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);
	pen.writeFixed (version);
	pen.writeFixed (italicAngle);
	pen.writeFWord (underlinePosition);
	pen.writeFWord (underlineThickness);
	pen.writeULong (isFixedPitch);
	pen.writeULong (minMemType42);
	pen.writeULong (maxMemType42);
	pen.writeULong (minMemType1);
	pen.writeULong (maxMemType1);

	// Write glyph names

	if (version == 0x00020000) {
		pen.writeUShort (postNames.size());

		StringVector extraNames;

		StringVector::const_iterator i;
		for (i = postNames.begin(); i != postNames.end(); i ++) {
			Short index = getMacIndex (*i);
			if (index != -1)
				pen.writeShort (index);
			else {
				StringVector::iterator extraName = find (extraNames.begin(), extraNames.end(), *i);
				if (extraName != extraNames.end())
					pen.writeUShort (extraName - extraNames.begin());
				else {
					extraNames.push_back (*i);
					pen.writeUShort (extraNames.size() - 1 + MACINTOSH_SET_SIZE);
				}
			}
		}

		// Write extra names

		for (i = extraNames.begin(); i != extraNames.end(); i++) {
			pen.writeByte ((*i).length());
			pen.writeString (*i);
		}
	}

	return memory;
}

String postTable::getPostName (UShort glyphIndex) {
	if (glyphIndex >= postNames.size()) {
		if (version != 0x00030000)
			font.addWarning (new Exception (
				"No postscript name found for glyph " + String (glyphIndex)));
		return String();
	} else {
		return postNames [glyphIndex];
	}
}

void postTable::clear() {
	postNames.clear();
}

void postTable::set (const Glyphs &glyphs) {
	if (version != 0x00030000) {
		Glyphs::const_iterator g;
		for (g = glyphs.begin(); g != glyphs.end(); g ++)
			postNames.push_back ((*g)->getName());
	}
}

} // end namespace OpenType
