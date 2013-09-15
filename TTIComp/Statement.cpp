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

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <cassert>
#include "../InstructionProcessor/Instructions.h"
#include "Statement.h"
#include "Scope.h"

using std::endl;

/*** Statement represents a statement in an TTI file ***/

Statement::Statement(Scope &aScope, const PreprocessorPosition &aPos)
: pos (aPos), scope (aScope), valid (true) {}

void Statement::writeToStream(ostream &o) const {
	o << "<Undefined statement>";
}

PreprocessorPosition Statement::getPos() const {
	return pos;
}

void Statement::invalidate() {
	assert(valid);
	valid = false;
}

bool Statement::isValid() const {
	return valid;
}

void Statement::callExecutable() {}

bool Statement::checkUncalled() {
	return false;
}

void Statement::addToInstructionSequence (InstructionSequence * seq) const {
	assert(false);
}

ostream& operator<< (ostream& o, const Statement &s) {
	if (!s.isValid()) {
		o << "<invalid statement \"";
		s.writeToStream(o);
		o << "\">";
	} else
		s.writeToStream(o);
	return o;
}

void Statement::assignStoragePlaces(UShort &storageId) {}


/*** ExpressionStatement is a statement that only contains an Expression ***/

ExpressionStatement::ExpressionStatement(Scope &aScope, const PreprocessorPosition &aPos, ExpressionPtr aExpr)
 : Statement(aScope, aPos) {
	expr = aExpr;
	if (expr->getType(scope) == tyUnknown) {
		invalidate();
		return;
	}
	if (!expr->hasSideEffect())
		pos.prep.startError(pos, true) << "Expression with side effect expected." << endl;
//	scope->addExecutableDef(this);
}

ExpressionStatement::~ExpressionStatement() {}

void ExpressionStatement::writeToStream (ostream &o) const {
	o << expr << ";";
}

void ExpressionStatement::callExecutable() {
	expr->referenceDefinition();
}

void ExpressionStatement::addToInstructionSequence (InstructionSequence * seq) const {
	expr->addToInstructionSequence (seq, false);
}


/*** ReturnStatement is a return statement, e.g. return; or return getDistance(1u,2u); ***/

ReturnStatement::ReturnStatement(Scope &aScope, const PreprocessorPosition &aPos, ExpressionPtr aExpr)
: Statement(aScope, aPos) {
	expr = aExpr;
	if (expr) {
		// TODO: check this type
		if (expr->getType(scope) == tyUnknown) {
			invalidate();
			return;
		}
		if (expr->getType(scope) == tyVoid) {
			pos.prep.startError() << "Error: cannot return value of type \"void\"." << endl;
			invalidate();
			return;
		}
	}
//	scope->addExecutableDef(this);
}

ReturnStatement::~ReturnStatement() {}

void ReturnStatement::writeToStream (ostream &o) const {
	o << "return " << expr << ";";
}

void ReturnStatement::callExecutable() {
	if (expr)
		expr->referenceDefinition();
}

void ReturnStatement::addToInstructionSequence (InstructionSequence * seq) const {
	if (expr)
		expr->addToInstructionSequence(seq, true);

	// Jump to the end of the InstructionSequence (
	smart_ptr <DistancePushValue> jumpValue = new DistancePushValue();
	seq->push (smart_ptr <PushValue> (jumpValue));
	seq->addInstruction(new JumpInstruction());
	seq->notifyStackChange (1);
	InstructionPositionPtr afterReturnStatement = seq->setOptimisationBoundaryHere();
	jumpValue->setDistance (afterReturnStatement, new InstructionPosition(), true, 1);
}

/*** IfStatement is an if-statement, e.g. if (getDistance(p1, p2)<1.0) {} else {} ***/

IfStatement::IfStatement(Scope &aScope, const PreprocessorPosition &aPos, ExpressionPtr aCondition,
						 StatementPtr aIfTrue, StatementPtr aIfFalse) : Statement(aScope, aPos) {
	assert (aCondition);
	assert (aIfTrue);
	condition = aCondition;
	ifTrue = aIfTrue;
	ifFalse = aIfFalse;

	Type conditionType = condition->getType(scope);
	if (conditionType == tyUnknown) {
		invalidate();
		return;
	}
	if (conditionType == tyVoid) {
		pos.prep.startError(pos) << "Error: \"if\" statement condition cannot have type \"void\"." << endl;
		invalidate();
		return;
	}

//	scope->addExecutableDef(this);
}

IfStatement::~IfStatement() {}

void IfStatement::writeToStream (ostream &o) const {
	o << "if (" << condition << ")" << endl << ifTrue;
	if (ifFalse)
		o  << endl << "else" << endl << ifFalse;
}

void IfStatement::callExecutable() {
	condition->referenceDefinition();
	ifTrue->callExecutable();
	if (ifFalse)
		ifFalse->callExecutable();
}

bool IfStatement::checkUncalled() {
	ifTrue->checkUncalled();
	if (ifFalse)
		ifFalse->checkUncalled();
	return false;
}

void IfStatement::assignStoragePlaces(UShort &storageId) {
	ifTrue->assignStoragePlaces(storageId);
	if (ifFalse)
		ifFalse->assignStoragePlaces(storageId);
}

void IfStatement::addToInstructionSequence (InstructionSequence * seq) const {
	if (ifFalse) {
		DistancePushValue *jump1Value = new DistancePushValue();
		seq->push(jump1Value);
		condition->addToInstructionSequence(seq, true);
		seq->addInstruction(new JumpRelOnFalseInstruction());
		seq->notifyStackChange(2);
		InstructionPositionPtr beforeTrue = seq->setOptimisationBoundaryHere();
		// Execute first block of code
		ifTrue->addToInstructionSequence(seq);
		DistancePushValue *jump2Value = new DistancePushValue();
		seq->push(jump2Value);
		seq->addInstruction(new JumpInstruction());
		seq->notifyStackChange(1);
		InstructionPositionPtr beforeFalse = seq->setOptimisationBoundaryHere();
		// Execute second block of code
		ifFalse->addToInstructionSequence(seq);
		InstructionPositionPtr afterFalse = seq->setOptimisationBoundaryHere();

		// Wire up jump values
		jump1Value->setDistance(beforeTrue, beforeFalse, true, 1);
		jump2Value->setDistance(beforeFalse, afterFalse, true, 1);
	} else {
		DistancePushValue *jumpValue = new DistancePushValue();
		seq->push(jumpValue);
		condition->addToInstructionSequence(seq, true);
		seq->addInstruction(new JumpRelOnFalseInstruction());
		seq->notifyStackChange(2);
		InstructionPositionPtr beforeTrue = seq->setOptimisationBoundaryHere();
		// Execute first block of code
		ifTrue->addToInstructionSequence(seq);
		InstructionPositionPtr endIf = seq->setOptimisationBoundaryHere();

		jumpValue->setDistance(beforeTrue, endIf, true, 1);
	}
}


/*** WhileStatement is an while-statement, e.g. while (getDistance(p1, p2)<1.0) {} ***/

WhileStatement::WhileStatement(Scope &aScope, const PreprocessorPosition &aPos, ExpressionPtr aCondition,
						 StatementPtr aStatement) : Statement(aScope, aPos) {
	assert(aCondition);
	assert(aStatement);
	condition = aCondition;
	statement = aStatement;

	Type conditionType = condition->getType(scope);
	if (conditionType == tyUnknown) {
		invalidate();
		return;
	}
	if (conditionType == tyVoid) {
		pos.prep.startError(pos) << "Error: \"if\" statement condition cannot have type \"void\"." << endl;
		invalidate();
		return;
	}

//	scope->addExecutableDef(this);
}

WhileStatement::~WhileStatement() {}

void WhileStatement::writeToStream (ostream &o) const {
	o << "while (" << condition << ")" << endl << statement;
}

void WhileStatement::callExecutable() {
	condition->referenceDefinition();
	statement->callExecutable();
}

bool WhileStatement::checkUncalled() {
	statement->checkUncalled();
	return false;
}

void WhileStatement::assignStoragePlaces(UShort &storageId) {
	statement->assignStoragePlaces(storageId);
}

void WhileStatement::addToInstructionSequence (InstructionSequence * seq) const {
	InstructionPositionPtr beforeWhile = seq->setOptimisationBoundaryHere();
	DistancePushValue *jump1Value = new DistancePushValue();
	seq->push(jump1Value);
	condition->addToInstructionSequence(seq, true);
	seq->addInstruction(new JumpRelOnFalseInstruction());
	seq->notifyStackChange(2);

	InstructionPositionPtr beforeStatement = seq->setOptimisationBoundaryHere();

	statement->addToInstructionSequence(seq);
	DistancePushValue *jump2Value = new DistancePushValue();
	seq->push(jump2Value);
	seq->addInstruction(new JumpInstruction());
	seq->notifyStackChange(1);
	InstructionPositionPtr afterStatement = seq->setOptimisationBoundaryHere();

	jump1Value->setDistance(beforeStatement, afterStatement, true, 1);
	jump2Value->setDistance(beforeWhile, afterStatement, false, 1);
}


/*** CompoundStatement is a statement that consists of multiple statements
	 enclosed by {}, e.g. a function body. ***/

CompoundStatement::CompoundStatement(Scope &aParentScope, const PreprocessorPosition &aPos,
									 BodyScopePtr aBodyScope, bool functionBody)
									 : Statement(aParentScope, aPos), bodyScope (aBodyScope) {}

CompoundStatement::~CompoundStatement() {}

void CompoundStatement::writeToStream (ostream &o) const {
	o << "{" << endl << bodyScope << "}";
}

void CompoundStatement::callExecutable() {
	bodyScope->callExecutableInstructions();
}

bool CompoundStatement::checkUncalled() {
	bodyScope->deleteUncalledDefinitions();
	return false;
}

void CompoundStatement::assignStoragePlaces (UShort &storageId) {
	bodyScope->assignStoragePlaces(storageId);
}

void CompoundStatement::addToInstructionSequence (InstructionSequence * seq) const {
	bodyScope->addToInstructionSequence (seq);
}

VariableDefinitionStatementPtr CompoundStatement::getLocalVariable (String name) const {
	return bodyScope->getLocalVariable(name);
}

/*** DefinitionStatement defines a function or a variable. ***/

DefinitionStatement::DefinitionStatement(Scope &aScope, const PreprocessorPosition &aPos,
										 Type aType, String aName) :
Statement(aScope, aPos), type (aType), name (aName), called (false) {}

void DefinitionStatement::writeToStream (ostream &o) const {
	o << type << " " << name;
}

String DefinitionStatement::getName() const {
	return name;
}

Type DefinitionStatement::getType() {
	return type;
}

void DefinitionStatement::setCalled(bool aInGlyphProgram, UShort glyphId) {
	// Only functions may be linked to glyph programs
	assert(aInGlyphProgram==false);
	called = true;
}

/*** VariableDefinitionStatement defines a variable ***/

VariableDefinitionStatement::VariableDefinitionStatement(Scope &aScope, const PreprocessorPosition &aPos, Type aType,
														 String aName, const ExpressionPtr aInitialValue, bool aConstant)
: DefinitionStatement(aScope, aPos, aType, aName), initialValue (aInitialValue), constant (aConstant) {
	if (initialValue) {
		Type valueType = initialValue->getType(aScope);
		if (aType != valueType) {
			pos.prep.startError(pos) << "Error: variable of type \"" << aType << 
				"\" cannot be initialised to expression with type \"" << valueType << "\"." << endl;
			invalidate();
			return;
		}
	}

	storageId = 0xFFFF;
}

VariableDefinitionStatement::~VariableDefinitionStatement() {}

void VariableDefinitionStatement::writeToStream (ostream &o) const {
	if (constant)
		o << "const ";
	DefinitionStatement::writeToStream(o);
	if (constant)
		o << " = " << initialValue << ";";
	else
		o << ";";
}

bool VariableDefinitionStatement::checkUncalled() {
	if (!called) {
		pos.prep.startError(pos, true) << "Unreferenced variable \"" << getName() << "\"" << endl;
		return true;
	}
	return false;
}

bool VariableDefinitionStatement::isAssignable() const {
	return !constant;
}

bool VariableDefinitionStatement::isConstant() const {
	return constant;
}

void VariableDefinitionStatement::callExecutable() {
	if (initialValue)
		initialValue->referenceDefinition();

	if (constant && (!initialValue || !initialValue->isConstant())) {
		pos.prep.startError(pos) << "Error: \"const\" variable should have a constant initial value." << endl;
		invalidate();
		return;
	}
}

ULong VariableDefinitionStatement::getConstantValue() const {
	assert(isConstant());
	return initialValue->getConstantValue();
}

UShort VariableDefinitionStatement::getStorageId() const {
	assert(called);
	assert(!constant);
	return storageId;
}

void VariableDefinitionStatement::assignStoragePlaces(UShort &aStorageId) {
	if (!constant) {
		storageId = aStorageId;
		aStorageId ++;
	}
}

void VariableDefinitionStatement::addToInstructionSequence (InstructionSequence * seq) const {
	// Initialise variable if needed
/*	if (!constant && initialValue) {
		seq->push(storageId);
		initialValue->addToInstructionSequence(seq, true);
		seq->addInstruction(new WriteStoreInstruction());
		seq->notifyStackChange(2);
	}*/
}

void VariableDefinitionStatement::callThis(InstructionSequence * seq) const {
	if (constant)
		seq->push(getConstantValue());
	else {
		seq->push(storageId);
		seq->addInstruction(new ReadStoreInstruction());
		seq->notifyStackChange(1,1);
	}
}

void VariableDefinitionStatement::assignToThis(InstructionSequence * seq, ExpressionPtr expr, bool returnValue) const {
	assert(isAssignable());
	if (!returnValue) {
		seq->push(storageId);
		expr->addToInstructionSequence(seq, true);
	} else {
		expr->addToInstructionSequence(seq, true);
		seq->addInstruction(new DuplicateStackElementInstruction());
		seq->notifyStackChange(0,1);
		seq->push(storageId);
		seq->addInstruction(new SwapStackElementsInstruction());
	}
	seq->addInstruction(new WriteStoreInstruction());
	seq->notifyStackChange(2);
}

/*** FunctionDefinitionStatement defines a function ***/

FunctionDefinitionStatement::FunctionDefinitionStatement(Scope &aScope, const PreprocessorPosition &aPos, Type aType,
														 String aName, const FormalParameters &aFormalParameters)
														 : DefinitionStatement(aScope, aPos, aType, aName) {
	formalParameters = aFormalParameters;
}

void FunctionDefinitionStatement::writeToStream (ostream &o) const {
	DefinitionStatement::writeToStream(o);
	o << formalParameters;
}

const FormalParameters &FunctionDefinitionStatement::getFormalParameters() const {
	return formalParameters;
}

InstructionSequencePtr FunctionDefinitionStatement::getInstructionSequence (bool forGlyphProgram) const {
	return InstructionSequencePtr();
}

void FunctionDefinitionStatement::assignFunctionIds(UShort &aFunctionId) {}

UShort FunctionDefinitionStatement::getFunctionId() const {
	assert(false);
	return 0;
}

UShort FunctionDefinitionStatement::getTargetGlyph() const {
	assert(false);
	return 0;
}

void FunctionDefinitionStatement::callThis(InstructionSequence * seq,
										   const FunctionCallParameters &parameters) const {
	assert(false);
}


ostream& operator<< (ostream& o, const FormalParameters &params) {
	o << '(';
	FormalParameters::const_iterator i;
	for (i = params.begin(); i != params.end(); i++) {
		if (!(*i).name.empty())
			o << (*i).type << ' ' << (*i).name;
		else
			o << (*i).type;
		if (i != params.end() - 1)
			o << ", ";
	}
	o << ')';
	return o;
}

bool operator == (const FormalParameter &p1, const FormalParameter &p2) {
	return p1.type == p2.type;
}

/*** FunctionDeclarationStatement ***/

FunctionDeclarationStatement::FunctionDeclarationStatement(Scope &aScope, const PreprocessorPosition &aPos, Type aType,
														   String aName, const FormalParameters &aFormalParameters)
														   : FunctionDefinitionStatement(aScope, aPos, aType, aName, aFormalParameters) {
//	scope->addFunctionDecl(this);
}

void FunctionDeclarationStatement::writeToStream(ostream &o) const {
	FunctionDefinitionStatement::writeToStream(o);
	o << ';';
}


/*** FunctionWithBodyStatement ***/

FunctionWithBodyStatement::FunctionWithBodyStatement(Scope &aScope, const PreprocessorPosition &aPos, bool aIsInline,
													 Type aType, String aName, const FormalParameters &aFormalParameters,
													 CompoundStatementPtr aBody)
: FunctionDefinitionStatement(aScope, aPos, aType, aName, aFormalParameters),
body (aBody), isInline (aIsInline), inGlyphProgram (false), glyphId (0xFFFF), functionId (0xFFFF) {}

FunctionWithBodyStatement::~FunctionWithBodyStatement() {}

void FunctionWithBodyStatement::writeToStream(ostream &o) const {
	FunctionDefinitionStatement::writeToStream(o);
	o << endl <<  body;
}

void FunctionWithBodyStatement::setCalled(bool aInGlyphProgram, UShort aGlyphID) {
	// Only recurse if this function has not been called yet
	bool recurse = !inGlyphProgram && !called;
	if (aInGlyphProgram) {
		inGlyphProgram = true;
		glyphId = aGlyphID;
	} else
		called = true;
	if (recurse) body->callExecutable();
}

void FunctionWithBodyStatement::callExecutable() {
	body->callExecutable();
}

bool FunctionWithBodyStatement::checkUncalled() {
	if (!called && !inGlyphProgram) {
		pos.prep.startError(pos, true) << "Unreferenced function \"" << getName() <<
			formalParameters << "\"" << endl;
		return true;
	}

	body->checkUncalled();
	return false;
}

void FunctionWithBodyStatement::assignStoragePlaces(UShort &storageId) {
	body->assignStoragePlaces(storageId);
}

void FunctionWithBodyStatement::assignFunctionIds (UShort &aFunctionId) {
	// This is only for functions that are called from other functions
	if (called) {
		functionId = aFunctionId;
		aFunctionId ++;
	}
}

UShort FunctionWithBodyStatement::getTargetGlyph() const {
	assert(inGlyphProgram);
	return glyphId;
}

UShort FunctionWithBodyStatement::getFunctionId() const {
	assert(called);
	return functionId;
}

InstructionSequencePtr  FunctionWithBodyStatement::getInstructionSequence(bool forGlyphProgram) const {
	InstructionSequencePtr seq;
	if (forGlyphProgram) {
		if (!inGlyphProgram)
			return NULL;
		else {
			if (called) {
				// This function is both called and linked to a function;
				// it will call _itself_ in the glyph program.
				seq = new InstructionSequence(true);
				callThis(&*seq, FunctionCallParameters());
//				seq->push(functionId);
//				seq->addInstruction(new CallInstruction());
				return seq;
			} else {
				seq = new InstructionSequence(true);
				body->addToInstructionSequence (&*seq);
//				return seq;
			}
		}
	} else {
		if (!called)
			return NULL;
		else {
			seq = new InstructionSequence (false);
			body->addToInstructionSequence (&*seq);
//			return seq;
		}
	}

	// Return a value if needed. TODO: make sure there is a return statement before this
	if (type != tyVoid)
		seq->push((Short)0);
	return seq;
}

void FunctionWithBodyStatement::callThis(InstructionSequence * seq,
										 const FunctionCallParameters &params) const {
	assert(called);
	// parameters need to be filled in for formalParameters
	// So the lengths of both need to be equal
	assert (formalParameters.size() == params.size());

	FormalParameters::const_iterator formal = formalParameters.begin();
	FunctionCallParameters::const_iterator actual = params.begin();
	while (formal != formalParameters.end()) {
		assert(actual != params.end());

		VariableDefinitionStatementPtr var = body->getLocalVariable ((*formal).name);
		if (var) {
			seq->push(var->getStorageId());
			(*actual)->addToInstructionSequence(seq, true);
			seq->addInstruction(new WriteStoreInstruction());
			seq->notifyStackChange(2);
		} else
		{	// Don't use this variable; execute it anyway, because the writer
			// of the program may expect it to. If it has no side effect it
			// will do nothing:
			(*actual)->addToInstructionSequence(seq, false);
		}
		formal ++;
		actual ++;
	}
	assert(actual == params.end());
	seq->push(functionId);
	seq->addInstruction(new CallInstruction());
	seq->notifyStackChange(1);
	if (type != tyVoid)
		seq->notifyStackChange(0,1);
	seq->notifyUnknownPointers();
}


/*** Return a statement from the preprocessor stream ***/

void getFirstCharacter(char c, Preprocessor &prep) {
	PreprocessorPosition pos = prep.getCurrentPosition();
	String buffer = prep.get();
	if (buffer != c) {
		prep.startError(pos) << "Syntax error: \"" << c << "\" expected instead of \""
			<< buffer << "\"." << endl;
		do {
			buffer = prep.get();
		} while (!prep.eof() && buffer != c);
	}
}

smart_ptr <CompoundStatement> getCompoundStatement(Preprocessor &prep, Scope &parentScope,
												  smart_ptr <BodyScope> bodyScope, bool isFunctionBody) {
	PreprocessorPosition pos = prep.getCurrentPosition();
	String buffer = prep.peek();

	if (buffer != "{") {
		prep.startError() << "Syntax error: \"{\" expected; \"" << buffer << "\" found instead." << endl;
		return NULL;
	}
	prep.deleteToken();

	buffer = prep.peek();
	while (buffer != "}" && !prep.eof()) {
		// Find statement and add
		StatementPtr s = getStatement (prep, *bodyScope);

		if (!s) {
			prep.startError() << "Syntax error: statement expected instead of \"" << buffer << "\"." << endl;
			return new CompoundStatement(parentScope, pos, bodyScope, isFunctionBody);
		}
		buffer = prep.peek();
	}
	if (prep.eof()) {
		prep.startError() << "Syntax error: \"}\" expected; end of file found instead." << endl;
		return new CompoundStatement(parentScope, pos, bodyScope, isFunctionBody);
	}
	// "}" found
	prep.deleteToken();
	return new CompoundStatement(parentScope, pos, bodyScope, isFunctionBody);
}

smart_ptr <CompoundStatement> getCompoundStatementAnyway(Preprocessor &prep, Scope &parentScope) {
	PreprocessorPosition pos = prep.getCurrentPosition();
	String buffer = prep.peek();

	BodyScopePtr bodyScope = new BodyScope(prep, parentScope);
	if (buffer == "{")
		return getCompoundStatement(prep, parentScope, bodyScope, true);

	StatementPtr s = getStatement(prep, *bodyScope);
	if (!s)
		return NULL;

	return new CompoundStatement(parentScope, pos, bodyScope, true);
}


smart_ptr <ReturnStatement> getReturnStatement(Preprocessor &prep, Scope &scope) {
	String buffer;
	PreprocessorPosition pos = prep.getCurrentPosition();

	assert(prep.peek() == "return");
	prep.deleteToken();

	buffer = prep.peek();
	if (buffer == ";") {
		prep.deleteToken();
		return new ReturnStatement(scope, pos, NULL);
	}

	ExpressionPtr expr = getExpression(scope, prep);
	if (!expr) {
		prep.startError() << "Syntax error: expression expected after \"return\"." << endl;
		return NULL;
	}

	buffer = prep.get();
	if (buffer != ";") {
		prep.startError() << "Syntax error: \";\" expected after \"return <expression>\"." << endl;
	}
	return new ReturnStatement(scope, pos, expr);
}

smart_ptr <IfStatement> getIfStatement(Preprocessor &prep, Scope &scope) {
	String buffer;
	PreprocessorPosition pos = prep.getCurrentPosition();

	buffer = prep.peek();
	assert(prep.peek() == "if");
	prep.deleteToken();

	buffer = prep.get();
	if (buffer != "(") {
		prep.startError() << "Syntax error: \"(\" expected after \"if\"." << endl;
		return NULL;
	}

	ExpressionPtr condition = getExpression(scope, prep);
	if (!condition) {
		prep.startError() << "Error: condition expected after \"if (\"." << endl;
		return NULL;
	}

	buffer = prep.get();
	if (buffer != ")") {
		prep.startError() << "Syntax error: \")\" expected after \"if (<condition>\"." << endl;
		return NULL;
	}

	StatementPtr ifTrue = getCompoundStatementAnyway(prep, scope);
	if (!ifTrue) {
		prep.startError() << "Error: statement expected after \"if ()\"." << endl;
		return NULL;
	}

	buffer = prep.peek();
	if (buffer != "else")
		// if without else
		return new IfStatement(scope, pos, condition, ifTrue, NULL);

	prep.deleteToken();

	StatementPtr ifFalse = getCompoundStatementAnyway(prep, scope);
	if (!ifFalse) {
		prep.startError() << "Error: statement expected after \"else\"." << endl;
		return NULL;
	}

	return new IfStatement(scope, pos, condition, ifTrue, ifFalse);
}

smart_ptr <WhileStatement> getWhileStatement(Preprocessor &prep, Scope &scope) {
	String buffer;
	PreprocessorPosition pos = prep.getCurrentPosition();

	assert(prep.peek() == "while");
	prep.deleteToken();

	buffer = prep.get();
	if (buffer != "(") {
		prep.startError() << "Syntax error: \"(\" expected after \"while\"." << endl;
		return NULL;
	}

	ExpressionPtr condition = getExpression(scope, prep);
	if (!condition) {
		prep.startError() << "Error: condition expected after \"while (\"." << endl;
		return NULL;
	}

	buffer = prep.get();
	if (buffer != ")") {
		prep.startError() << "Syntax error: \")\" expected after \"while (<condition>\"." << endl;
		return NULL;
	}

	StatementPtr statement = getCompoundStatementAnyway(prep, scope);
	if (!statement) {
		prep.startError() << "Error: statement expected after \"while ()\"." << endl;
		return NULL;
	}

	return new WhileStatement(scope, pos, condition, statement);
}


smart_ptr <DefinitionStatement> getDefinitionStatement(Preprocessor &prep, Scope &scope) {
	String buffer;
	PreprocessorPosition pos = prep.getCurrentPosition();
	buffer = prep.peek();
	bool isConstant = false, isInline = false;
	if (buffer == "const")
	{	// Constant variable found
		prep.deleteToken();
		buffer = prep.peek();
		isConstant = true;
	} else {
		if (buffer == "inline") {
			// inline function found
			prep.deleteToken();
			buffer = prep.peek();
			isInline = true;
		}
	}
	Type type = getType(buffer);
	if (type != tyUnknown) {
		buffer = prep.peek(1);
		// Just in case it is a cast or something...
		if (isLetterChar(buffer[0]) || buffer[0] == '_') {
			String name = buffer;
			prep.deleteToken();
			prep.deleteToken();
			buffer = prep.peek();
			if (buffer == ";") {
				if (isInline)
					prep.startError (pos) << "Variable cannot be declared \"inline\"." << endl;
				prep.deleteToken();
				// Variable definition without initial value
				VariableDefinitionStatementPtr statement =
					new VariableDefinitionStatement(scope, pos, type, name, NULL, isConstant);
				if (statement->isValid())
					scope.addVariableDef (statement);
				return statement;
			}

			if (buffer == "=")
			{	// Variable definition with initial value
				if (isInline)
					prep.startError (pos) << "Variable cannot be declared \"inline\"." << endl;
				prep.deleteToken();
				ExpressionPtr initialValue = getExpression(scope, prep);
				getFirstCharacter(';', prep);
				VariableDefinitionStatementPtr statement =
					new VariableDefinitionStatement(scope, pos, type, name, initialValue, isConstant);
				if (statement->isValid()) {
					scope.addVariableDef (statement);
					if (!isConstant) {
						smart_ptr <ExpressionStatement> expressionStatement = 
							new ExpressionStatement(scope, pos,
								new BinaryExpression(new VariableExpression(name, scope, pos),
									boAssign, initialValue, scope, pos));
						if (statement->isValid())
							scope.addExecutableDef (expressionStatement);
					}
				}
				return statement;
			}

			if (isConstant)
			{	// Error: constant function definition or something else
				prep.startError(pos) << "Syntax error: variable with initial value expected after \"const\"." << endl;
				getFirstCharacter(';', prep);
				return NULL;
			}

			if  (buffer == "(")
			{	// Function definition
				prep.deleteToken();
				FormalParameters params;

				//	Prepare scope for local variables
				BodyScopePtr bodyScope = new BodyScope(prep, scope);
				PreprocessorPosition varPos = prep.getCurrentPosition();

				buffer = prep.get();
				while (buffer != ")") {
					Type paramType = getType(buffer);
					if (paramType == tyUnknown) {
						prep.startError() << "Syntax error: formal parameter type expected, \"" << buffer << "\" found instead." << endl;
						getFirstCharacter(')', prep);
						break;
					}

					buffer = prep.get();
					if (buffer != "," && buffer != ")") {
						if (!isLetterChar(buffer[0]) && buffer[0] != '_') {
							prep.startError() << "Syntax error: formal parameter identifier expected, \"" << buffer << "\" found instead." << endl;
							getFirstCharacter(')', prep);
							break;
						}
					}

					FormalParameter param;
					param.type = paramType;
					if (buffer != "," && buffer != ")"){
						param.name = buffer;
						String varName = buffer;
						VariableDefinitionStatementPtr def = 
							new VariableDefinitionStatement (*bodyScope, varPos, param.type, varName, NULL, false);
						if (def->isValid())
							bodyScope->addVariableDef (def);
						buffer = prep.get();
					}
					params.push_back (param);

					if (buffer != "," && buffer != ")") {
						prep.startError() << "Syntax error: \",\" or \")\" expected; \"" << buffer << "\" found instead." <<endl;
						do {
							buffer = prep.get();
						} while (!prep.eof() && buffer != ")");
					}

					if (buffer == ",")
						buffer = prep.get();
				}
				buffer = prep.peek();
				if (buffer == ";") {
					prep.deleteToken();
					FunctionDeclarationStatementPtr statement =
						new FunctionDeclarationStatement(scope, pos, type, name, params);
					if (statement->isValid())
						scope.addFunctionDecl (statement);
					return statement;
				} else {
					FunctionDefinitionStatementPtr statement =
						new FunctionWithBodyStatement(scope, pos, isInline, type, name, params,
							getCompoundStatement(prep, scope, bodyScope, true));
					if (statement->isValid())
						scope.addFunctionDef (statement);
					return statement;
				}
			}

			// Unknown construction

			prep.startError() << "Syntax error: \";\", \"=\" or \"(\" expected in variable definition or function definition; found \"" << buffer << "\" instead." << endl;
			getFirstCharacter(';', prep);
			return NULL;
		}
	}

	if (isConstant || isInline)
		prep.startError() << "Syntax error: type expected instead of \"" << buffer << "\"." << endl;

	return NULL;
}



smart_ptr <Statement> getStatement(Preprocessor &prep, Scope &scope) {
	String buffer = prep.peek();

	// Get compound statement from stream
	if (buffer == "{") {
		CompoundStatementPtr statement = getCompoundStatement(prep, scope, new BodyScope(prep, scope), false);
		if (statement && statement->isValid())
			scope.addExecutableDef (statement);
		return statement;
	}

	// Get return statement from stream
	if (buffer == "return") {
		smart_ptr <ReturnStatement> statement = getReturnStatement(prep, scope);
		if (statement && statement->isValid())
			scope.addExecutableDef (statement);
		return statement;
	}

	// Get if statement from stream
	if (buffer == "if") {
		smart_ptr <IfStatement> statement = getIfStatement(prep, scope);
		if (statement && statement->isValid())
			scope.addExecutableDef (statement);
		return statement;
	}

	// Get while statement from stream
	if (buffer == "while") {
		smart_ptr <WhileStatement> statement = getWhileStatement(prep, scope);
		if (statement && statement->isValid())
			scope.addExecutableDef (statement);
		return statement;
	}

	{
		// Get definition statement from stream
		DefinitionStatementPtr statement = getDefinitionStatement(prep, scope);
		if (statement) return statement;
	}

	// Get expression statement from stream
	{
		// Get expression followed by ; from the stream
		ExpressionPtr expr = getExpression(scope, prep);
		if (!expr) {
			prep.startError();
			prep.startError() << "Syntax error: expression expected instead of \"" << buffer << "\"." << endl;
			buffer = prep.get();
			return NULL;
		}

		getFirstCharacter(';', prep);

		smart_ptr <ExpressionStatement> statement =
			new ExpressionStatement(scope, prep.getCurrentPosition(), expr);
		if (statement && statement->isValid())
			scope.addExecutableDef (statement);
		return statement;
	}
}
