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

#ifndef SCOPE_H
#define SCOPE_H

#include <vector>
#include "Statement.h"

using std::vector;
using std::pair;

class Scope {
protected:
	typedef vector <StatementPtr> StatementList;
	typedef vector <DefinitionStatementPtr> DefinitionStatementList;
	typedef vector <FunctionDefinitionStatementPtr> FunctionDefinitionStatementList;
	typedef vector <FunctionDeclarationStatementPtr> FunctionDeclarationStatementList;
	typedef vector <VariableDefinitionStatementPtr> VariableDefinitionStatementList;
	typedef pair <FunctionDefinitionStatementList::const_iterator,
		FunctionDefinitionStatementList::const_iterator> FunctionDefinitionRange;
	FunctionDefinitionStatementList functionDecls;
	FunctionDefinitionStatementList functionDefs;
	VariableDefinitionStatementList variableDefs;
	StatementList executableDefs;

	Preprocessor &prep;

	// Get the range of functions with this name from the list
	FunctionDefinitionRange
		getFunctionsFromList (String name, const FunctionDefinitionStatementList &list) const;

	// Get the function with this signature from the list
	FunctionDefinitionStatementPtr getFunctionFromList(
		FunctionDefinitionStatementPtr statement, const FunctionDefinitionStatementList &list) const;

	// Get function from list by assuming any type for "tyUnknown"
	FunctionDefinitionStatementPtr getImpliedFunctionFromList(String name,
		TypeVector &types, const FunctionDefinitionStatementList &list) const;

public:
	Scope(Preprocessor &aPrep);
	virtual ~Scope();

	virtual void writeToStream(ostream &o) const;

	virtual void addFunctionDecl (FunctionDeclarationStatementPtr statement);
	virtual void addFunctionDef (FunctionDefinitionStatementPtr statement);
	virtual void addVariableDef (VariableDefinitionStatementPtr statement);
	virtual void addExecutableDef (StatementPtr statement);

	virtual VariableDefinitionStatementPtr getLocalVariable (String name) const;
	virtual VariableDefinitionStatementPtr getVariable (String name) const;
	virtual FunctionDeclarationStatementPtr getFunctionDeclaration (String name, TypeVector &types) const;
	virtual FunctionDefinitionStatementPtr getFunctionDefinition (String name, TypeVector &types) const;

	virtual void deleteUncalledDefinitions();
	virtual void assignStoragePlaces(UShort &storageId);
};

ostream& operator<< (ostream& o, const Scope &scope);

class MainScope : public Scope {
public:
	MainScope(Preprocessor &prep);
	virtual ~MainScope() {}

	void readFile();

	virtual void addPredefinedDefinitions();
		// MainScope::addExecutableDef produces an error because executable
		// statements can only exist within a function body.
	virtual void addExecutableDef(StatementPtr statement);

	void compileFont (smart_ptr <OpenTypeFont> font);
};

class BodyScope : public Scope {
	Scope &parent;
public:
	BodyScope(Preprocessor &prep, Scope &aParent);
	virtual ~BodyScope() {}

		// BodyScope::addFunctionDef produces an error because function definitions
		// statements can only exist within a function body.
	virtual void addFunctionDef (FunctionDefinitionStatementPtr statement);

	virtual VariableDefinitionStatementPtr getVariable (String name) const;
	virtual FunctionDeclarationStatementPtr getFunctionDeclaration (String name, TypeVector &types) const;
	virtual FunctionDefinitionStatementPtr getFunctionDefinition (String name, TypeVector &types) const;

	virtual void callExecutableInstructions();
	virtual void addToInstructionSequence (InstructionSequence *seq) const;
};


#endif // SCOPE_H
