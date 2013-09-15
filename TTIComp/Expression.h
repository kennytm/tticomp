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
	\file Expressions used to read .TTI files
*/

#ifndef EXPRESSION_H
#define EXPRESSION_H

extern bool optimise;

#include "../Util/Preprocessor.h"
#include "../Util/smart_ptr.h"
#include "../OTFont/OpenType.h"

using std::vector;
using std::ostream;
using util::smart_ptr;
using util::String;
using OpenType::ULong;
using OpenType::Long;

enum MaximumNumbers {
	maxULONG	= ((ULong)0xFFFFFFFF),
	minULONG	= ((ULong)0x0),
	maxLONG		= ((Long) 0x7FFFFFFF),
	minLONG		= ((Long)-0x80000000)
};

class Scope;
class MainScope;
class BodyScope;
class InstructionSequence;
class VariableDefinitionStatement;
class FunctionDeclarationStatement;
class FunctionDefinitionStatement;
class Expression;

typedef smart_ptr <VariableDefinitionStatement> VariableDefinitionStatementPtr;
typedef smart_ptr <FunctionDeclarationStatement> FunctionDeclarationStatementPtr;
typedef smart_ptr <FunctionDefinitionStatement> FunctionDefinitionStatementPtr;
typedef smart_ptr <Expression> ExpressionPtr;

typedef enum {
	tyUnknown, tyVoid, tyInt, tyUint, tyFixed, tyBool,
} Type;

typedef enum {
	boMult, boDiv, boAdd, boSub, boAssign,
	boEqual, boNotEqual, boGreater, boGreaterEqual, boLess, boLessEqual,
	boOr, boAnd
} BinaryOperator;

class Expression {
protected:
	Scope &scope;
	PreprocessorPosition pos;
public:
	Expression(Scope &aScope, PreprocessorPosition aPos);
	virtual ~Expression() {}
	virtual void writeToStream(ostream& o) const;
	
	virtual bool hasSideEffect() const;
	virtual bool isAssignable() const;
	virtual Type getType(Scope &scope);
	virtual bool isConstant() const; // That is to say, is the value known now?
	virtual ULong getConstantValue() const;

	virtual void referenceDefinition();
	virtual void addToInstructionSequence(InstructionSequence * seq, bool returnValue) const;
	virtual void addAssignToInstructionSequence(InstructionSequence * seq,
		ExpressionPtr aExpr, bool returnValue) const;
};

ostream& operator<< (ostream& o, smart_ptr <Expression> e);
ostream& operator<< (ostream& o, Type t);
ostream& operator<< (ostream& o, BinaryOperator b);

class ConstantExpression : public Expression {
	Type type;
	ULong value;
public:
	ConstantExpression(Type aType, ULong aValue, Scope &aScope, PreprocessorPosition aPos);
	virtual ~ConstantExpression() {}
	virtual void writeToStream(ostream& o) const;
	
	virtual Type getType(Scope &scope);
	virtual bool isConstant() const;
	virtual ULong getConstantValue() const;

	virtual void addToInstructionSequence(InstructionSequence * seq, bool returnValue) const;
};

class VariableExpression : public Expression {
	String variableName;
	VariableDefinitionStatementPtr reference;
public:
	VariableExpression(String aVariableName, Scope &aScope, PreprocessorPosition aPos);
	virtual ~VariableExpression();
	virtual void writeToStream(ostream& o) const;

	virtual Type getType(Scope &scope);
	virtual bool isAssignable() const;
	virtual bool isConstant() const;
	virtual ULong getConstantValue() const;
	
	virtual void referenceDefinition();
	virtual void addToInstructionSequence (InstructionSequence * seq, bool returnValue) const;
	virtual void addAssignToInstructionSequence (InstructionSequence * seq,
		ExpressionPtr aExpr, bool returnValue) const;
};

typedef vector < smart_ptr <Expression> > FunctionCallParameters;
typedef vector <Type> TypeVector;
ostream & operator << (ostream &o, const FunctionCallParameters &params);
ostream & operator << (ostream &o, const TypeVector &types);

class FunctionCallExpression : public Expression {
	String functionName;
	FunctionCallParameters parameters;
	TypeVector paramTypes;
	Type returnType;
	FunctionDeclarationStatementPtr declarationReference;
	FunctionDefinitionStatementPtr reference;
public:
	FunctionCallExpression(String aFunctionName, const FunctionCallParameters &aParameters,
		Scope &aScope, PreprocessorPosition aPos);
	virtual ~FunctionCallExpression();
	virtual void writeToStream(ostream& o) const;

	virtual Type getType(Scope &scope);
	virtual bool hasSideEffect() const;
	virtual bool isConstant() const;

	virtual void referenceDefinition();
	virtual void addToInstructionSequence(InstructionSequence * seq, bool returnValue) const;
};

class TypeCastExpression : public Expression {
	Type type;
	ExpressionPtr expr;
public:
	TypeCastExpression(Type aType, ExpressionPtr aExpr, Scope &aScope, PreprocessorPosition aPos);
	virtual ~TypeCastExpression();
	virtual void writeToStream(ostream &o) const;

	virtual Type getType(Scope &scope);
	virtual bool isAssignable() const;
	virtual bool isConstant() const;
	virtual ULong getConstantValue() const;

	virtual void referenceDefinition();
	virtual void addToInstructionSequence(InstructionSequence * seq, bool returnValue) const;
	virtual void addAssignToInstructionSequence(InstructionSequence * seq,
		ExpressionPtr aExpr, bool returnValue) const;
};

class NegateExpression : public Expression {
	ExpressionPtr expr;
public:
	NegateExpression(ExpressionPtr aExpr, Scope &aScope, PreprocessorPosition aPos);
	virtual ~NegateExpression();
	virtual void writeToStream(ostream &o) const;

	virtual Type getType(Scope &scope);
	virtual bool isConstant() const;
	virtual ULong getConstantValue() const;

	virtual void referenceDefinition();
	virtual void addToInstructionSequence(InstructionSequence * seq, bool returnValue) const;
};

class NotExpression : public Expression {
	ExpressionPtr expr;
public:
	NotExpression(ExpressionPtr aExpr, Scope &aScope, PreprocessorPosition aPos);
	virtual ~NotExpression();
	virtual void writeToStream(ostream &o) const;

	virtual Type getType(Scope &scope);
	virtual bool isConstant() const;
	virtual ULong getConstantValue() const;

	virtual void referenceDefinition();
	virtual void addToInstructionSequence(InstructionSequence * seq, bool returnValue) const;
};

class BinaryExpression : public Expression {
	ExpressionPtr left, right;
	BinaryOperator op;

	void reorderLeftRight();
public:
	BinaryExpression(ExpressionPtr aLeft, BinaryOperator aOp, ExpressionPtr aRight, Scope &aScope, PreprocessorPosition aPos);
	virtual ~BinaryExpression();
	virtual void writeToStream(ostream& o) const;

	virtual bool hasSideEffect() const;
	virtual Type getType(Scope &scope);
	virtual bool isConstant() const;
	virtual ULong getConstantValue() const;

	virtual void referenceDefinition();
	virtual void addToInstructionSequence(InstructionSequence * seq, bool returnValue) const;
};

ExpressionPtr getExpression(Scope &scope, Preprocessor &prep);

Type getType(String buffer);

#endif	//	EXPRESSION_H
