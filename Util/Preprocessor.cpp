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

/**
	\file Preprocessor
*/

#include <vector>
#include <cassert>
#include "Preprocessor.h"
using std::ios;
using std::vector;
using std::endl;
using std::ifstream;
using std::ostream;

using util::String;

Preprocessor::Preprocessor(String aFileName, int aMaxErrorNum) {
	curFileName = aFileName;
	curFile = new ifstream(curFileName.getCString(), ios::in);
	lineNumber = colNumber = 1;
	errorNum = warningNum = 0;
	maxErrorNum = aMaxErrorNum;
	backtracked = 0;
	
	if (!curFile->is_open()) {
		throw String("Error: couldn't open file.");
		delete curFile;
		curFile = NULL;
		return;
	}

	unsigned char i;
	for (i=0; i<255; i++) {
		firstIdentifierChars[i] = isLetterChar(i) || i=='_';
		nextIdentifierChars[i] = isLetterChar(i) || isNumberChar(i) || i=='_';

		firstNumberChars[i] = isNumberChar(i) || i=='.';
		nextNumberChars[i] = isLetterChar(i) || isNumberChar(i) || i=='_' || i=='.';

		specialChars[i] = false;
	}
}


Preprocessor::~Preprocessor() {
	if (curFile) {
		curFile->close();
		delete curFile;
	}
}

// Private methods

char Preprocessor::getChar() {
	if (curFile->eof())
		return '\0';
	else {
		int c = curFile->get();
		if (c == EOF)
			return '\0';
		if (backtracked)
			backtracked --;
		else {
			if (c=='\n') {
				lineNumber ++;
				colNumber = 1;
			} else {
				if (c=='\t')
					colNumber += 4;
				else
					colNumber ++;
			}
		}
		return c;
	}
}

char Preprocessor::peekChar() {
	return curFile->peek();
}

void Preprocessor::putBackChar (char c) {
	if (c) {
		backtracked ++;
		curFile->putback (c);
	}
}

bool Preprocessor::isSpecialToken (char firstChar, char secondChar) {
	return false;
	return (firstChar == '=' && secondChar == '=') ||
		(firstChar == '-' && secondChar == '>');
}

void Preprocessor::eatWhite() {
	char firstChar = getChar();
	while (isWhiteSpace(firstChar))
		firstChar = getChar();
	putBackChar(firstChar);
}

void Preprocessor::eatComments() {
	do {
		eatWhite();
		char firstChar = getChar();

		if (firstChar == '/') {
			// This may be a comment
			char secondChar = peekChar();
			if (secondChar == '/') {
				// Comment
				getChar();
				do {
					firstChar = getChar();
				} while (firstChar != '\n' && firstChar != '\0');
			} else {
				if (secondChar == '*') {
					/* Comment */
					getChar();
					do {
						firstChar = secondChar;
						secondChar = getChar();
					} while (secondChar != '\0' && (firstChar != '*' || secondChar != '/'));
				} else
				{	// No comment after all
					putBackChar (firstChar);
					return;
				}
			}
		} else {
			putBackChar (firstChar);
			return;
		}
	} while (true);
}

void Preprocessor::addOption (String option, String param, PreprocessorPosition &pos) {
	// Default handles
	startError (pos) << "Unknown option \"#" << option << "\"." << endl;
}

bool Preprocessor::getToken() {
	eatComments();

	// Save current position
	PreprocessorPosition pos = PreprocessorPosition (*this, curFileName, lineNumber, colNumber);

	char firstChar = getChar();
	char secondChar;
	while (true) {
		if (!firstChar)
			return false;

		// Options
		if (firstChar == '#') {
			String option;
			firstChar = getChar();
			while (nextIdentifierChars [firstChar]) {
				option += firstChar;
				firstChar = getChar();
			}
			while (firstChar == ' ')
				firstChar = getChar();
			String param;
			while (true) {
				if (firstChar == '\n' || firstChar == '\r')
					break;
				if (firstChar == '\\') {
					firstChar = getChar();
					while (firstChar != '\n') {
						if (!isWhiteSpace (firstChar))
							startError() << "\"" << firstChar << "\" found after \"\\\"." << endl;
						firstChar = getChar();
					}
					while (isWhiteSpace (firstChar)) {
						firstChar = getChar();
					}
					param += ' ';
				} else {
					param += firstChar;
					firstChar = getChar();
				}
			}
			param += ' ';
			String::const_iterator i = param.end();
			do {
				i --;
			} while (i != param.begin() && isWhiteSpace (*i));
			addOption (option, String (param.begin(), i + 1), pos);
		} else
		{
			// Identifiers
			if (firstIdentifierChars [firstChar]) {
				// Get identifier
				String identifier;
				do {
					identifier += firstChar;
					firstChar = getChar();
				} while (nextIdentifierChars [firstChar]);

				if (firstChar)
					putBackChar (firstChar);
				tokens.push_back (Token (identifier, pos));
				return true;
			} else
			{
				// Numbers
				if (firstNumberChars [firstChar]) {
					// Get number
					String number;
					do {
						number += firstChar;
						firstChar = getChar();
					} while (nextNumberChars [firstChar]);

					if (firstChar)
						putBackChar (firstChar);
					tokens.push_back (Token (number, pos));
					return true;
				} else
				{
					// Special sequences like "==" or "->"
					if (specialChars [firstChar]) {
						secondChar = peekChar();
						if (isSpecialToken(firstChar, secondChar)) {
							tokens.push_back (Token (String (firstChar) + String (secondChar), pos));
							getChar();
							return true;
						}
					}
					tokens.push_back (Token (String (firstChar), pos));
					return true;
				}
			}
		}

		eatComments();
		pos.fileName = curFileName;
		pos.lineNumber = lineNumber;
		pos.colNumber = colNumber;
		firstChar = getChar();
	}
}

String Preprocessor::get() {
	if (tokens.empty()) {
		if (!getToken())
			return String();
	}
	String token =  tokens.front().token;
	tokens.pop_front();
	return token;
}

String Preprocessor::peek (unsigned int position) {
	while (tokens.size() <= position) {
		if (!getToken())
			return String();
	}
	return tokens [position].token;
}

void Preprocessor::deleteToken (unsigned int num) {
	// You should have seen all tokens
	assert (tokens.size() >= num);
	unsigned int i;
	for (i=0; i<num; i++) {
		tokens.pop_front();
	}
}

bool Preprocessor::eof() {
	return tokens.empty() && !getToken();
}

PreprocessorPosition Preprocessor::getCurrentPosition() {
	if (tokens.empty()) {
		return  PreprocessorPosition (*this, curFileName, lineNumber, colNumber);
	} else
		return tokens.front().pos;
}


ostream & operator << (ostream &o, const PreprocessorPosition &pos) {
	return o << pos.fileName << "(l." << pos.lineNumber << " c." << pos.colNumber << ")";
}

ostream & Preprocessor::startError (const PreprocessorPosition &pos, bool warning) {
	if (warning)
		warningNum++;
	else {
		errorNum++;
		if (errorNum > maxErrorNum)
			throw TooManyErrorsException();
	}
	ostream &o = getErrorStream();
	o << pos << (warning ? ": warning: " : ": error: ");
	return o;
}

void Preprocessor::see (const PreprocessorPosition &pos) {
	ostream &o = getErrorStream();
	o << "    (see " << pos << ")" << endl;
}

int Preprocessor::getErrorNum() const {
	return errorNum;
}

int Preprocessor::getWarningNum() const {
	return warningNum;
}
