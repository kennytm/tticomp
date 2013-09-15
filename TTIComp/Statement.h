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
	\file Statements found in TTI files. May be executed.
*/

#ifndef STATEMENT_H
#define STATEMENT_H

#include "Expression.h"
#include "InstructionSequence.h"

class Statement;
class DefinitionStatement;
class VariableDefinitionStatement;
class FunctionDefinitionStatement;
class FunctionDeclarationStatement;
class CompoundStatement;

typedef smart_ptr <Statement> StatementPtr;
typedef smart_ptr <DefinitionStatement> DefinitionStatementPtr;
typedef smart_ptr <VariableDefinitionStatement> VariableDefinitionStatementPtr;
typedef smart_ptr <FunctionDefinitionStatement> FunctionDefinitionStatementPtr;
typedef smart_ptr <FunctionDeclarationStatement> FunctionDeclarationStatementPtr;
typedef smart_ptr <CompoundStatement> CompoundStatementPtr;

class Statement {
	bool valid;
protected:
	Scope &scope;
	PreprocessorPosition pos;
public:
	Statement(Scope &aScope, const PreprocessorPosition &aPos);
//	Statement(Scope &aScope);
	virtual ~Statement() {}
	virtual void writeToStream(ostream& o) const;
	PreprocessorPosition getPos() const;

	void invalidate();
	bool isValid() const;

	virtual void assignStoragePlaces(UShort &storageId);
	virtual void callExecutable();
	virtual bool checkUncalled();
	virtual void addToInstructionSequence(InstructionSequence * seq) const;
};

ostream& operator<< (ostream& o, const Statement &s);

class ExpressionStatement : public Statement {
	ExpressionPtr expr;
public:
	ExpressionStatement(Scope &aScope, const PreprocessorPosition &aPos, ExpressionPtr aExpr);
	virtual ~ExpressionStatement();

	virtual void writeToStream(ostream& o) const;
	virtual void callExecutable();
	virtual void addToInstructionSequence(InstructionSequence * seq) const;
};

class ReturnStatement : public Statement {
	ExpressionPtr expr;
public:
	ReturnStatement(Scope &aScope, const PreprocessorPosition &aPos, ExpressionPtr aExpr);
	virtual ~ReturnStatement();

	virtual void writeToStream(ostream& o) const;
	virtual void callExecutable();
	virtual void addToInstructionSequence(InstructionSequence * seq) const;
};

class IfStatement : public Statement {
	ExpressionPtr condition;
	StatementPtr ifTrue, ifFalse;
public:
	IfStatement(Scope &aScope, const PreprocessorPosition &aPos, ExpressionPtr aCondition, 
		StatementPtr aIfTrue, StatementPtr aIfFalse);
	virtual ~IfStatement();

	virtual void writeToStream(ostream &o) const;
	virtual void callExecutable();
	virtual bool checkUncalled();
	virtual void assignStoragePlaces(UShort &storageId);
	virtual void addToInstructionSequence(InstructionSequence * seq) const;
};

class WhileStatement : public Statement {
	ExpressionPtr condition;
	StatementPtr statement;
public:
	WhileStatement(Scope &aScope, const PreprocessorPosition &aPos, ExpressionPtr aCondition, StatementPtr aStatement);
	virtual ~WhileStatement();

	virtual void writeToStream(ostream &o) const;
	virtual void callExecutable();
	virtual bool checkUncalled();
	virtual void assignStoragePlaces(UShort &storageId);
	virtual void addToInstructionSequence(InstructionSequence * seq) const;
};

typedef smart_ptr <BodyScope> BodyScopePtr;

class CompoundStatement : public Statement {
	BodyScopePtr bodyScope;
public:
		// if functionBody==true then this statement will not be added to the parent scope
	CompoundStatement(Scope &aParentScope, const PreprocessorPosition &aPos,
		BodyScopePtr aBodyScope, bool functionBody);
	virtual ~CompoundStatement();

	virtual void writeToStream(ostream &o) const;
	virtual void callExecutable();
	virtual bool checkUncalled();
	virtual void assignStoragePlaces (UShort &storageId);
	virtual void addToInstructionSequence (InstructionSequence * seq) const;

	virtual VariableDefinitionStatementPtr getLocalVariable(String name) const;
};


class DefinitionStatement : public Statement {
	String name;
protected:
	bool called;
	Type type;
public:
	DefinitionStatement(Scope &aScope, const PreprocessorPosition &aPos, Type aType, String aName);
	virtual ~DefinitionStatement() {}

	virtual void writeToStream(ostream &o) const;
	String getName() const;
	Type getType();

	virtual void setCalled(bool aInGlyphProgram = false, UShort glyphId=0);
};

class VariableDefinitionStatement : public DefinitionStatement {
	ExpressionPtr initialValue;
	bool constant;
	UShort storageId;
public:
	VariableDefinitionStatement(Scope &aScope, const PreprocessorPosition &aPos,
		Type aType, String aName, ExpressionPtr initialValue, bool constant);
	virtual ~VariableDefinitionStatement();

	virtual void writeToStream(ostream &o) const;
	virtual bool checkUncalled();
	virtual bool isAssignable() const;
	virtual bool isConstant() const;
	virtual void callExecutable();
	virtual ULong getConstantValue() const;
	virtual UShort getStorageId() const;

	virtual void assignStoragePlaces (UShort &aStorageId);
	virtual void addToInstructionSequence (InstructionSequence * seq) const;
	virtual void callThis (InstructionSequence * seq) const;
	virtual void assignToThis (InstructionSequence * seq, ExpressionPtr expr, bool returnValue) const;
};

typedef struct {
	Type type;
	String name;
} FormalParameter;

typedef vector<FormalParameter> FormalParameters;

ostream& operator<< (ostream& o, const FormalParameters &params);
// Returns true iff both types are the same (names don't matter here)
bool operator == (const FormalParameter &p1, const FormalParameter &p2);

class FunctionDefinitionStatement : public DefinitionStatement {
protected:
	FormalParameters formalParameters;
public:
	FunctionDefinitionStatement(Scope &aScope, const PreprocessorPosition &aPos, Type aType,
		String aName, const FormalParameters &aFormalParameters);
	virtual ~FunctionDefinitionStatement() {}

	virtual void writeToStream(ostream &o) const;
	const FormalParameters &getFormalParameters() const;

	virtual void assignFunctionIds(UShort &aFunctionId);
	virtual UShort getFunctionId() const;
	virtual InstructionSequencePtr getInstructionSequence (bool forGlyphProgram) const;
	virtual UShort getTargetGlyph() const;

	virtual void callThis (InstructionSequence * seq,
		const FunctionCallParameters &parameters) const;
};

class FunctionDeclarationStatement : public FunctionDefinitionStatement {
public:
	FunctionDeclarationStatement(Scope &aScope, const PreprocessorPosition &aPos,
		Type aType, String aName, const FormalParameters &aFormalParameters);
	virtual ~FunctionDeclarationStatement() {}

	virtual void writeToStream(ostream &o) const;
};

class FunctionWithBodyStatement : public FunctionDefinitionStatement {
	bool isInline;
	CompoundStatementPtr body;
	bool inGlyphProgram;
	UShort glyphId;
	UShort functionId;
public:
	FunctionWithBodyStatement(Scope &aScope, const PreprocessorPosition &aPos, bool aIsInline, Type aType,
		String aName, const FormalParameters &aFormalParameters, CompoundStatementPtr aBody);
	~FunctionWithBodyStatement();

	virtual void setCalled(bool aInGlyphProgram = false, UShort aGlyphID=0);

	virtual void writeToStream(ostream &o) const;
	virtual void callExecutable();
	virtual bool checkUncalled();
	virtual void assignStoragePlaces(UShort &storageId);
	virtual void assignFunctionIds(UShort &aFunctionId);
	virtual UShort getTargetGlyph() const;
	virtual UShort getFunctionId() const;
	virtual InstructionSequencePtr getInstructionSequence(bool forGlyphProgram) const;
	virtual void callThis(InstructionSequence *seq, const FunctionCallParameters &parameters) const;
};

DefinitionStatementPtr getDefinitionStatement(Preprocessor &prep, Scope &scope);
StatementPtr getStatement(Preprocessor &prep, Scope &scope);

#endif	// STATEMENT_H
