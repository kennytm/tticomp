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
	\file Contains a preprocessor for code files.
*/

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <fstream>
#include <deque>

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include "../Util/String.h"

#define isNumberChar(a) ((a)>='0' && (a) <='9')
#define isLetterChar(a) (((a)>='A' && (a) <='Z') || ((a)>='a' && (a) <='z'))
#define isWhiteSpace(a) ((a)==' ' || (a)=='\t' || (a)=='\n' || (a)=='\r')

class Preprocessor;

/**
	\brief Contains a position in an input file.

	May be used to indicate a position where an error occurred to the user.
*/
struct PreprocessorPosition {
	Preprocessor &prep;
	util::String fileName;
	int lineNumber;
	int colNumber;
	PreprocessorPosition (Preprocessor &aPrep, util::String aFileName, int aLineNumber, int aColNumber)
		: prep (aPrep), fileName (aFileName), lineNumber (aLineNumber), colNumber (aColNumber) {}
	PreprocessorPosition (const PreprocessorPosition & p) : prep (p.prep), fileName (p.fileName),
		lineNumber (p.lineNumber), colNumber (p.colNumber) {}
	PreprocessorPosition & operator = (const PreprocessorPosition & p)
	{
		assert (&prep == &p.prep);
		fileName = p.fileName;
		lineNumber = p.lineNumber;
		colNumber = p.colNumber;
		return *this;
	}
};

class Token {
public:
	util::String token;
	PreprocessorPosition pos;
	Token (util::String aToken, PreprocessorPosition aPos) :
	token (aToken), pos (aPos) {}
};

/**
	\brief Is thrown when too many errors (usually 100) were encountered.

	Applications using the preprocessor should catch this exception and notify
	the user that the compilation process will be aborted.
*/
class TooManyErrorsException {
public:
	TooManyErrorsException() {};
};

/**
	\brief Read text files in tokens.

	Returns the tokens one at a time. It is possible to read tokens after
	the first one to look forward.
	It also provides a mechanism to write error messages to a stream in
	a structured manner.

	\see PreprocessorPosition
 */

class Preprocessor {
	std::ifstream *curFile;
	util::String curFileName;
	int lineNumber, colNumber;

	int maxErrorNum;
	int errorNum;
	int warningNum;

	std::deque <Token> tokens;

	void eatWhite();
	void eatComments();
	// Returns true if a new token could be loaded
	bool getToken();

	unsigned int backtracked;
	char getChar();
	char peekChar();
	void putBackChar (char c);

protected:
	bool firstIdentifierChars[256];
	bool nextIdentifierChars[256];
	bool firstNumberChars[256];
	bool nextNumberChars[256];
	bool specialChars[256];
	virtual bool isSpecialToken(char firstChar, char secondChar);

	virtual void addOption(util::String option, util::String param, PreprocessorPosition &pos);

	/// Return the error stream that is to be used. If you do not want
	/// to use cout, override this method.
	virtual std::ostream &getErrorStream() { return std::cout; }

public:
	Preprocessor (util::String aFileName, int aMaxErrorNum = 200);
	virtual ~Preprocessor();

	/// \brief Get the first token from the stream and delete it.
	util::String get();
	/// \brief Get the first token from the stream but do not delete it.
	/// \param position The position of the token to be read in the stream.
	/// 0 means the first token.
	util::String peek (unsigned int position = 0);
	/// \brief Delete the token from the stream.
	///
	/// An assert is used to make sure the token has actually been seen
	/// (i.e. peek has been called).
	void deleteToken (unsigned int num = 1);

	/// Get the position for the first token in the token stream
	PreprocessorPosition getCurrentPosition();

	/// \brief Write an error header to the error stream and returns the 
	/// stream to write a more extended error message.
	///
	/// \see getErrorStream()
	std::ostream &startError (const PreprocessorPosition &pos, bool warning = false);

	/// \brief Write an error on the current position.
	/// \see getErrorStream()
	inline std::ostream &startError (bool warning = false) { return startError(getCurrentPosition(), warning); }

	/// \brief Writes a "see file ... line ... column ..." to the error stream.
	///
	/// This is useful if an error needs an explanation that may be found
	/// elsewhere.
	void see (const PreprocessorPosition &pos);

	/// \brief Return the number of errors found.
	int getErrorNum() const;
	/// \brief Return the number of warnings found.
	int getWarningNum() const;

	bool eof();
};

#endif	// PREPROCESSOR_H
