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
	\file Scope.h defines a scope for function and variable definitions.
*/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <cassert>
#include <algorithm>
#include <functional>
#include "../Util/smart_ptr.h"
#include "../InstructionProcessor/Instructions.h"
#include "../OTFont/OpenTypeFont.h"
#include "../OTFont/OTGlyph.h"
#include "Scope.h"

using std::endl;
using std::pair;
using std::lower_bound;
using std::upper_bound;
using std::equal_range;
using std::unary_function;
using std::for_each;

ostream& operator<< (ostream& o, const Scope &scope) {
	scope.writeToStream(o);
	return o;
}


Scope::Scope (Preprocessor &aPrep) : prep (aPrep) {}

Scope::~Scope() {}

template <class S> struct writeStatement : public unary_function<S, void>
{
	ostream &o;
	writeStatement(ostream &aO) : o(aO) {};
	void operator() (S s) { o << s << endl; }
};

void Scope::writeToStream(ostream &o) const {
	if (!variableDefs.empty()) {
		o << "// Variable definitions" << endl;
		for_each (variableDefs.begin(), variableDefs.end(), writeStatement<StatementPtr >(o));
		o << endl;
	}

	if (!functionDecls.empty()) {
		o << "// Function declarations" << endl;
		for_each (functionDecls.begin(), functionDecls.end(), writeStatement<StatementPtr >(o));
		o << endl;
	}

	for_each (functionDefs.begin(), functionDefs.end(), writeStatement<StatementPtr >(o));
	for_each (executableDefs.begin(), executableDefs.end(), writeStatement<StatementPtr >(o));
}

bool definitionStatementCompare (const DefinitionStatementPtr s1, const DefinitionStatementPtr s2) {
	return s1->getName() < s2->getName();
}

Scope::FunctionDefinitionRange
	Scope::getFunctionsFromList (String name, const FunctionDefinitionStatementList &list) const
{
	FunctionDefinitionStatementList::const_iterator firstLower, firstUpper,
		secondLower, secondUpper, guess;

	firstLower = list.begin();
	secondUpper = list.end();
	while (firstLower != secondUpper) {
		guess = firstLower + (secondUpper - firstLower)/2;
		if (name < (*guess)->getName())
			secondUpper = guess;
		else {
			if ((*guess)->getName() < name)
				firstLower = guess + 1;
			else {
				// Found right guess, so split here
				// Guess is in [firstLower, secondUpper)
				firstUpper = guess;
				secondLower = guess;

				// Find start of range first
				while (firstLower != firstUpper) {
					guess = firstLower + (firstUpper - firstLower)/2;
					if (name == (*guess)->getName())
						firstUpper = guess;
					else {
						assert ((*guess)->getName() < name);
						firstLower = guess + 1;
					}
				}
				
				// Find end of range
				while (secondLower != secondUpper) {
					guess = secondLower + (secondUpper - secondLower)/2;
					if (name == (*guess)->getName())
						secondLower = guess + 1;
					else {
						assert (name < (*guess)->getName());
						secondUpper = guess;
					}
				}

				return FunctionDefinitionRange (firstLower, secondUpper);
			}
		}
	}
	return FunctionDefinitionRange (firstLower, secondUpper);
}

// See whether a function with this signature exists
FunctionDefinitionStatementPtr Scope::getFunctionFromList (FunctionDefinitionStatementPtr statement,
														   const FunctionDefinitionStatementList &list) const
{
	/*pair<FunctionDefinitionStatementList::const_iterator,
		FunctionDefinitionStatementList::const_iterator> bounds =
		equal_range(list.begin(), list.end(), statement, definitionStatementCompare);*/
	FunctionDefinitionRange bounds = getFunctionsFromList (statement->getName(), list);

	FunctionDefinitionStatementList::const_iterator i;
	for (i = bounds.first; i < bounds.second; i++) {
		assert (statement->getName() == (*i)->getName());
		if (statement->getFormalParameters() == (*i)->getFormalParameters())
			return *i;
	}
	return NULL;
}

FunctionDefinitionStatementPtr Scope::getImpliedFunctionFromList(String name, TypeVector &paramTypes,
																 const FunctionDefinitionStatementList &list) const
{
/*	smart_ptr<DefinitionStatement> statementSearchedFor(new DefinitionStatement(name));
	pair<FunctionDefinitionStatementList::const_iterator,
		FunctionDefinitionStatementList::const_iterator> bounds =
		equal_range(list.begin(), list.end(), statementSearchedFor, definitionStatementCompare);*/
	FunctionDefinitionRange bounds = getFunctionsFromList (name, list);

	FunctionDefinitionStatementList::const_iterator i;
	for (i = bounds.first; i < bounds.second; i++) {
		assert ((*i)->getName() == name);
		if (paramTypes.size() == (*i)->getFormalParameters().size()) {
			TypeVector::iterator actual = paramTypes.begin();
			FormalParameters::const_iterator formal = (*i)->getFormalParameters().begin();
			while (true) {
				if (actual == paramTypes.end()) {
					// All types equal
					assert (formal == (*i)->getFormalParameters().end());
					return *i;
				}
				// Skip unknown types
				if (*actual != tyUnknown) {
					if (*actual != (*formal).type)
						// Not equal
						break;
				}

				actual ++;
				formal ++;
			}
		}
	}
	return FunctionDefinitionStatementPtr();
}


VariableDefinitionStatementPtr Scope::getLocalVariable (String name) const {
	VariableDefinitionStatementList::const_iterator lower, upper, guess;
	lower = variableDefs.begin();
	upper = variableDefs.end();

	while (lower != upper) {
		guess = lower + (upper - lower) / 2;
		if (name < (*guess)->getName())
			upper = guess;
		else {
			if (name > (*guess)->getName())
				lower = guess + 1;
			else
				return *guess;
		}
	}

	return NULL;
}

VariableDefinitionStatementPtr Scope::getVariable (String name) const {
	return getLocalVariable (name);
}

FunctionDeclarationStatementPtr Scope::getFunctionDeclaration (String name, TypeVector &types) const {
	return util::smart_ptr_cast <FunctionDeclarationStatement>
		(getImpliedFunctionFromList (name, types, functionDecls));
}

FunctionDefinitionStatementPtr Scope::getFunctionDefinition (String name, TypeVector &types) const {
	return getImpliedFunctionFromList (name, types, functionDefs);
}

void Scope::addFunctionDecl(FunctionDeclarationStatementPtr statement) {
	// First check whether this header does not exist yet
	FunctionDefinitionStatementPtr similar = getFunctionFromList(statement, functionDecls);
	if (!similar)
		getFunctionFromList(statement, functionDefs);

	if (similar) {
		ostream &o = prep.startError(statement->getPos());
		o << "Error: function \"" << statement->getName() << '(' << statement->getFormalParameters() << 
			")\" has already been defined." << endl;
		prep.see(similar->getPos());
		// delete statement
		statement->invalidate();
		return;
	}

	// Add the new declaration to the declaration list
	functionDecls.insert(
		upper_bound(functionDecls.begin(), functionDecls.end(), statement, definitionStatementCompare),
		statement);
}

void Scope::addFunctionDef(FunctionDefinitionStatementPtr statement) {
	// First check whether this very header does not exist yet
	FunctionDefinitionStatementPtr similar = getFunctionFromList(statement, functionDecls);
	if (similar) {
		if (similar->getType() != statement->getType()) {
			prep.startError(statement->getPos()) << "Error: function \"" << statement->getName() <<
				statement->getFormalParameters() << "\" has already been declared with return type " << 
				similar->getType() << ", not " << statement->getType() << "." << endl;
			prep.see(similar->getPos());

			statement->invalidate();
			return;
		}
	}

	similar = getFunctionFromList(statement, functionDefs);
	if (similar) {
		prep.startError(statement->getPos()) << "Error: function \"" << statement->getName() <<
				statement->getFormalParameters() << "\" has already been defined." << endl;
		prep.see(similar->getPos());

		statement->invalidate();
		return;
	}

	// No similar function found; add statement to list

	functionDefs.insert(
		upper_bound(functionDefs.begin(), functionDefs.end(), statement, definitionStatementCompare),
		statement);
}

void Scope::addVariableDef(VariableDefinitionStatementPtr statement) {
	// Check whether a variable with this name already exists

	VariableDefinitionStatementList::iterator var =
		lower_bound(variableDefs.begin(),variableDefs.end(), statement, definitionStatementCompare);

	if (var != variableDefs.end() && (*var)->getName() == statement->getName())
	{	// Variable already exists
		(prep.startError(statement->getPos())) << "Error: a variable named \"" <<
			statement->getName() << "\" has already been defined." << endl;
		prep.see((*var)->getPos());
		statement->invalidate();
		return;
	}
	
	if (statement->getType() == tyVoid) {
		(prep.startError(statement->getPos())) << "Error: no variable can have type \"void\"." << endl;
		statement->invalidate();
		return;
	}

	variableDefs.insert(var, statement);
}

void Scope::addExecutableDef(StatementPtr statement) {
	executableDefs.push_back(statement);
}


void Scope::deleteUncalledDefinitions() {
	VariableDefinitionStatementList::iterator var;
	for (var = variableDefs.begin(); var < variableDefs.end();) {
		if ((*var)->checkUncalled())
			variableDefs.erase(var);
		else
			var++;
	}

	FunctionDefinitionStatementList::iterator def;
	for (def=functionDefs.begin(); def<functionDefs.end();) {
		if ((*def)->checkUncalled())
			functionDefs.erase(def);
		else
			def++;
	}

	for (def=functionDecls.begin(); def<functionDecls.end();) {
		if ((*def)->checkUncalled())
			functionDecls.erase(def);
		else
			def++;
	}

}

template<class T> struct assignStorage : public unary_function<T, void>
{
	UShort &storageId;
	assignStorage(UShort &aStorageId) : storageId(aStorageId) {}
	void operator() (T x) { x->assignStoragePlaces(storageId); }
};

void Scope::assignStoragePlaces(UShort &storageId) {
	for_each(variableDefs.begin(), variableDefs.end(), assignStorage <StatementPtr>(storageId));
	for_each(functionDefs.begin(), functionDefs.end(), assignStorage <StatementPtr>(storageId));
	for_each(executableDefs.begin(), executableDefs.end(), assignStorage <StatementPtr>(storageId));
}


/*** MainScope ***/

MainScope::MainScope(Preprocessor &prep) : Scope(prep) {}

void MainScope::readFile() {
	addPredefinedDefinitions();
	while (!prep.eof()) {
		StatementPtr statement = getDefinitionStatement(prep, *this);
		if (!statement) {
			String buffer = prep.get();
			prep.startError() << "Syntax error: statement expected instead of \"" << buffer <<"\"." << endl;
		}
	}
}

void MainScope::addPredefinedDefinitions() {
	assert(false);
}


void MainScope::addExecutableDef(StatementPtr statement) {
	prep.startError() << "Error: executable statement found outside function body. Maybe you put in too many \"}\"s or assign a value to a non-constant global variable?" << endl;
	statement->invalidate();
}


void MainScope::compileFont (smart_ptr <OpenTypeFont> font) {
	UShort glyphNum = font->getGlyphNum();
	for (UShort i=0; i<glyphNum; i++)
		font->getGlyph (i)->setInstructions(MemoryBlockPtr());

	UShort storageSize = 0;
	assignStoragePlaces (storageSize);
	font->setMaxStorage (storageSize);

	UShort functionId = 0;
	FunctionDefinitionStatementList::iterator fdef;
	for (fdef = functionDefs.begin(); fdef < functionDefs.end(); fdef ++)
		(*fdef)->assignFunctionIds (functionId);

	font->setMaxFunctionDefs (functionId);
	font->setMaxInstructionDefs (0);

	/*** Get the font program ***/
	InstructionSequencePtr fontProgramSeq = new InstructionSequence(false);

	// Initialise global variables
	VariableDefinitionStatementList::iterator var;
	for (var = variableDefs.begin(); var < variableDefs.end(); var ++)
		(*var)->addToInstructionSequence (&*fontProgramSeq);

	// Add functions to fpgm table
	for (fdef = functionDefs.begin(); fdef < functionDefs.end(); fdef++) {
		InstructionSequencePtr seq = (*fdef)->getInstructionSequence(false);
		if (seq) {
			fontProgramSeq->push((*fdef)->getFunctionId());
			fontProgramSeq->addInstruction(new FunctionDefInstruction());
			fontProgramSeq->notifyStackChange(1);
			fontProgramSeq->addSequence(seq);
			fontProgramSeq->addInstruction(new EndFunctionDefInstruction());
		}
	}

	if (!fontProgramSeq->empty())
		font->setfpgm (fontProgramSeq->getMemory());
		//font->addTable("fpgm", new Table(fontProgramSeq->getBytes(), fontProgramSeq->getByteLength(), font, false));

	/*** Get glyph programs ***/

//	cur = functionDefs;
	ULong maxGlyphInstructionSize = 0;

	for (fdef = functionDefs.begin(); fdef < functionDefs.end(); fdef++) {
		InstructionSequencePtr seq = (*fdef)->getInstructionSequence(true);
		if (seq) {
			if (!seq->empty()) {
				if ((*fdef)->getName() == "prep")
					font->setprep (seq->getMemory());
					//font->addTable("prep", new Table(seq->getBytes(), seq->getByteLength(), font, false));
				else {
					MemoryBlockPtr memory = seq->getMemory();
					if (memory->getSize() > maxGlyphInstructionSize)
						maxGlyphInstructionSize = memory->getSize();
					font->getGlyph((*fdef)->getTargetGlyph())->setInstructions(seq->getMemory());
				}
			}
		}
	}

	font->setMaxSizeOfInstructions (maxGlyphInstructionSize);
}


/*** BodyScope ***/

BodyScope::BodyScope(Preprocessor &prep, Scope &aParent) : Scope(prep), parent (aParent) {}

void BodyScope::addFunctionDef(FunctionDefinitionStatementPtr statement) {
	prep.startError() << "Error: function definition found inside function body. Maybe you put in too few \"}\"s?" << endl;
	statement->invalidate();
}

VariableDefinitionStatementPtr BodyScope::getVariable (String name) const {
	VariableDefinitionStatementPtr v = getLocalVariable (name);
	if (!v)
		v = parent.getVariable (name);
	return v;
}

FunctionDeclarationStatementPtr BodyScope::getFunctionDeclaration (String name, TypeVector &types) const {
	return parent.getFunctionDeclaration (name, types);
}

FunctionDefinitionStatementPtr BodyScope::getFunctionDefinition (String name, TypeVector &types) const {
	return parent.getFunctionDefinition (name, types);
}

void BodyScope::callExecutableInstructions() {
	VariableDefinitionStatementList::iterator var;
	for (var = variableDefs.begin(); var < variableDefs.end(); var++)
		(*var)->callExecutable();

	StatementList::iterator exec;
	for (exec = executableDefs.begin(); exec < executableDefs.end(); exec++)
		(*exec)->callExecutable();

}

void BodyScope::addToInstructionSequence (InstructionSequence *seq) const {
	VariableDefinitionStatementList::const_iterator var;
	for (var = variableDefs.begin(); var < variableDefs.end(); var++)
		(*var)->addToInstructionSequence (seq);

	StatementList::const_iterator exec;
	for (exec = executableDefs.begin(); exec < executableDefs.end(); exec++)
		(*exec)->addToInstructionSequence (seq);
}
