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

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <cassert>
#include "Expression.h"
#include "Scope.h"
#include "../InstructionProcessor/Instructions.h"

using std::endl;

/*** ExpressionPtr **/

Expression::Expression(Scope &aScope, PreprocessorPosition aPos) : scope (aScope), pos (aPos) {}

void Expression::writeToStream(ostream &o) const {
	o << tyUnknown;
}

bool Expression::hasSideEffect() const {
	return false;
}

bool Expression::isAssignable() const {
	return false;
}

Type Expression::getType(Scope &scope) {
	return tyUnknown;
}

bool Expression::isConstant() const {
	assert(false);
	return false;
}

ULong Expression::getConstantValue() const {
	assert(false);
	return 0;
}

void Expression::referenceDefinition() {}

void Expression::addToInstructionSequence(InstructionSequence * seq, bool returnValue) const {
	assert(false);
}

void Expression::addAssignToInstructionSequence(InstructionSequence * seq, ExpressionPtr aExpr,
												bool returnValue) const {
	assert(false);
}

ostream& operator<< (ostream& o, smart_ptr <Expression> e) {
	if (e)
		e->writeToStream(o);
	else
		o << "????";
	return o;
}

ostream& operator<< (ostream& o, Type type) {
	switch (type) {
	case tyVoid:
		o << "void";
		break;
	case tyInt:
		o << "int";
		break;
	case tyUint:
		o << "uint";
		break;
	case tyFixed:
		o << "fixed";
		break;
	case tyBool:
		o << "bool";
		break;
	default:
		o << "<Unknown type>";
	}
	return o;
}

ostream& operator<< (ostream& o, BinaryOperator op) {
	switch (op) {
	case boMult:
		o << "*";
		break;
	case boDiv:
		o << "/";
		break;
	case boAdd:
		o << "+";
		break;
	case boSub:
		o << "-";
		break;

	case boAssign:
		o << "=";
		break;

	case boEqual:
		o << "==";
		break;
	case boNotEqual:
		o << "!=";
		break;
	case boGreater:
		o << ">";
		break;
	case boGreaterEqual:
		o << ">=";
		break;
	case boLess:
		o << "<";
		break;
	case boLessEqual:
		o << "<=";
		break;

	case boOr:
		o << "||";
		break;
	case boAnd:
		o << "&&";
		break;
	default:
		assert(false);
	}
	return o;
}


/*** ConstantExpression, e.g. 0, false, 0x5fau ***/

ConstantExpression::ConstantExpression(Type aType, ULong aValue, Scope &aScope,
									   PreprocessorPosition aPos)
									   : Expression(aScope, aPos), type (aType), value (aValue) {}


void ConstantExpression::writeToStream(ostream& o) const {
	if (type==tyBool) {
		if (value==0)
			o << "false";
		else {
			assert(value==1);
			o << "true";
		}
	} else {
		if (type==tyFixed)
			o << ((double) value/64) << 'p';
		else {
			if (type==tyInt)
				o << ((Long)value);
			else
				o << ((Long)value) << 'u';
		}
	}
}

Type ConstantExpression::getType(Scope &scope) {
	return type;
}

bool ConstantExpression::isConstant() const {
	return true;
}

ULong ConstantExpression::getConstantValue() const {
	return value;
}

void ConstantExpression::addToInstructionSequence (InstructionSequence * seq, bool returnValue) const {
	if (returnValue) {
		seq->push(value);
	}
}

/*** VariableExpression, e.g., a, previousStemWidth ***/

VariableExpression::VariableExpression(String aVariableName, Scope &aScope, PreprocessorPosition aPos) : Expression(aScope, aPos) {
	variableName =  aVariableName;
	reference = NULL;
}

VariableExpression::~VariableExpression() {}

void VariableExpression::writeToStream(ostream &o) const {
	o << variableName;
}

Type VariableExpression::getType(Scope &scope) {
	reference = scope.getVariable (variableName);
	//Type type = scope->getVariableType(variableName);
	if (!reference) {
		pos.prep.startError(pos) << "Error: undeclared variable name \"" << variableName << "\"." << endl;
		return tyUnknown;
	}
	return reference->getType();
}

bool VariableExpression::isAssignable() const {
	if (reference)
		return reference->isAssignable();
	else
		return true;
}

bool VariableExpression::isConstant() const {
	if (reference)
		return reference->isConstant();
	else
		return false;
}

ULong VariableExpression::getConstantValue() const {
	assert (isConstant());
	return reference->getConstantValue();
}

void VariableExpression::referenceDefinition() {
//	reference = scope->setCalledVariable(variableName);
	// The variable should exist because the getType() has been called, so it has been declared.
	assert (reference);
	reference->setCalled();
}

void VariableExpression::addToInstructionSequence(InstructionSequence * seq, bool returnValue) const {
	assert (reference);
	if (returnValue)
		reference->callThis(seq);
}

void VariableExpression::addAssignToInstructionSequence(InstructionSequence * seq,
														ExpressionPtr aExpr, bool returnValue) const {
	assert (isAssignable());
	assert (reference);
	reference->assignToThis(seq, aExpr, returnValue);
}


ostream & operator << (ostream &o, const FunctionCallParameters &params) {
	o << "(";
	FunctionCallParameters::const_iterator i;
	for (i = params.begin(); i != params.end(); i++) {
		o << *i;
		if (i != params.end() - 1)
			o << ", ";
	}
	o << ")";
	return o;
}

ostream & operator << (ostream &o, const TypeVector &types) {
	o << "(";
	TypeVector::const_iterator i;
	for (i = types.begin(); i != types.end(); i++) {
		o << *i;
		if (i != types.end() - 1)
			o << ", ";
	}
	o << ")";
	return o;
}


/*** FunctionCallExpression, e.g., movePointFarAway(2, 5-a, getPointPosition(7)) ***/

FunctionCallExpression::FunctionCallExpression(String aFunctionName, const FunctionCallParameters &aParameters, Scope &aScope, PreprocessorPosition aPos) : Expression(aScope, aPos) {
	functionName = aFunctionName;
	parameters = aParameters;
	declarationReference = NULL;
	returnType = tyUnknown;
	reference = NULL;
}

FunctionCallExpression::~FunctionCallExpression() {}

void FunctionCallExpression::writeToStream (ostream &o) const {
	o << functionName << parameters;
}

Type FunctionCallExpression::getType(Scope &scope) {
	if (paramTypes.empty()) {
		FunctionCallParameters::iterator i;
		for (i = parameters.begin(); i != parameters.end(); i++) {
			paramTypes.push_back ((*i)->getType (scope));
		}
	}

	FunctionDefinitionStatementPtr statement;
	reference = scope.getFunctionDefinition (functionName, paramTypes);
	if (reference)
		statement = reference;
	else
		statement = declarationReference =
			scope.getFunctionDeclaration (functionName, paramTypes);

	if (!statement) {
		pos.prep.startError(pos) << "Error: undeclared function \""
			<< functionName << paramTypes << "\"." << endl;
		return tyUnknown;
	}

	returnType = statement->getType();

	return statement->getType();

	/*Type *curType;

	if (paramNum == 0) {
		paramNum = 0;
		FunctionCallParameter *p = parameters;
		while (p) {
			p = p->next;
			paramNum++;
		}

		paramTypes = NULL;
		curType = paramTypes;
		if (paramNum) {
			paramTypes = new Type[paramNum];
			curType = paramTypes;
			p = parameters;
			while (p) {
				*curType = p->expr->getType(scope);
				curType++;
				p = p->next;
			}
		}
	}

	returnType = scope->getFunctionType(functionName, paramNum, paramTypes);;
	if (returnType==tyUnknown) {
		ostream &o = pos.prep.startError(pos);
		o << "Error: undeclared function \"" << functionName << '(';
		curType = paramTypes;
		for(int i=0; i<paramNum; i++) {
			o << *curType;
			curType++;
			if (i<paramNum-1)
				o << ", ";
		}
		o << ")\"." << endl;
	}

	return returnType;*/
}

bool FunctionCallExpression::hasSideEffect() const {
	return true;
}

bool FunctionCallExpression::isConstant() const {
	return false;
}

void FunctionCallExpression::referenceDefinition() {
	assert (paramTypes.size() == parameters.size());
	FunctionCallParameters::iterator i;
	for (i = parameters.begin(); i != parameters.end(); i++)
		(*i)->referenceDefinition();

//	reference = scope->setCalledFunction(functionName, paramNum, paramTypes);
	if (!reference)
		reference = scope.getFunctionDefinition (functionName, paramTypes);
	if (!reference)
	{	// Function definition not found
		pos.prep.startError(pos) << "Error: undefined function \""
			<< functionName << paramTypes << "\"." << endl;
	} else
		reference->setCalled();
}

void FunctionCallExpression::addToInstructionSequence(InstructionSequence * seq, bool returnValue) const {
	assert(reference);
	assert(returnType != tyUnknown);
	assert(!(returnValue && returnType==tyVoid));
	reference->callThis(seq, parameters);
	if (!returnValue && returnType!=tyVoid) {
		seq->addInstruction(new PopInstruction());
		seq->notifyStackChange(1, 0);
	}
}

/*** TypeCastExpression, e.g. int(6.3) ***/

TypeCastExpression::TypeCastExpression(Type aType, ExpressionPtr aExpr, Scope &aScope, PreprocessorPosition aPos) : Expression(aScope, aPos) {
	type = aType;
	expr = aExpr;
}

TypeCastExpression::~TypeCastExpression() {}

void TypeCastExpression::writeToStream(ostream &o) const {
	o << type << '(' << expr << ')';
}

Type TypeCastExpression::getType(Scope &scope) {
	Type exprType = expr->getType(scope);
	if (exprType != tyInt && exprType != tyUint && exprType != tyBool && exprType != tyFixed && exprType != tyUnknown) {
		pos.prep.startError(pos) << "Error: expressions of type \"" << exprType << "\" cannot be cast to another type." << endl;
	}
	if (type != tyInt && type != tyUint && type != tyBool && type != tyFixed) {
		pos.prep.startError(pos) << "Error: no expression can be cast to type \"" << exprType << "\"." << endl;
		return tyUnknown;
	}
	return type;
}

bool TypeCastExpression::isAssignable() const {
	return expr && expr->isAssignable();
}

bool TypeCastExpression::isConstant() const {
	return expr->isConstant();
}

ULong TypeCastExpression::getConstantValue() const {
	assert(isConstant());
	return expr->getConstantValue();
}

void TypeCastExpression::referenceDefinition() {
	expr->referenceDefinition();
}

void TypeCastExpression::addToInstructionSequence(InstructionSequence * seq, bool returnValue) const {
	if (isConstant()) {
		if (returnValue)
			seq->push(getConstantValue());
	}
	else
		expr->addToInstructionSequence(seq, returnValue);
}

void TypeCastExpression::addAssignToInstructionSequence(InstructionSequence * seq,
														ExpressionPtr aExpr, bool returnValue) const {
	assert(isAssignable());
	expr->addAssignToInstructionSequence(seq, aExpr, returnValue);
}

/*** NegateExpression, e.g. -6, -(345*0xfa) ***/

NegateExpression::NegateExpression(ExpressionPtr aExpr, Scope &aScope, PreprocessorPosition aPos) : Expression(aScope, aPos) {
	expr = aExpr;
}

NegateExpression::~NegateExpression() {}

void NegateExpression::writeToStream(ostream &o) const {
	o << '-' << expr;
}

Type NegateExpression::getType(Scope &scope) {
	Type childType = expr->getType(scope);
	if (childType == tyFixed)
		return tyFixed;
	if (childType == tyInt || childType == tyUint) 
		return tyInt;

	// Otherwise (i.e., childType is void, bool or Unknown), we have an error
	pos.prep.startError(pos) << "Error: type \"" << childType << "\" cannot be negated, so \"" << this << "\" is an invalid expression." << endl;
	return tyUnknown;
}

bool NegateExpression::isConstant() const {
	return expr->isConstant();
}

ULong NegateExpression::getConstantValue() const {
	assert(isConstant());
	return -(Long)expr->getConstantValue();
}

void NegateExpression::referenceDefinition() {
	expr->referenceDefinition();
}

void NegateExpression::addToInstructionSequence(InstructionSequence * seq, bool returnValue) const {
	if (isConstant()) {
		if (returnValue)
			seq->push(getConstantValue());
	} else {
		expr->addToInstructionSequence(seq, returnValue);
		if (returnValue) {
			seq->addInstruction(new NegateInstruction());
			seq->notifyStackChange(1,1);
		}
	}
}

/*** NotExpression, e.g. !(GetInformation(0x20)==5), !true ***/

NotExpression::NotExpression(ExpressionPtr aExpr, Scope &aScope, PreprocessorPosition aPos) : Expression(aScope, aPos) {
	expr = aExpr;
}

NotExpression::~NotExpression() {}

void NotExpression::writeToStream(ostream &o) const {
	o << '!' << expr;
}

Type NotExpression::getType(Scope &scope) {
	Type childType = expr->getType(scope);
	if (childType == tyBool || childType == tyUint || childType == tyInt || childType == tyFixed)
		return tyBool;

	// Otherwise (i.e., childType is void or Unknown), we have an error
	pos.prep.startError(pos) << "Error: invalid type for \"!\" expression \"" << childType << "\"." << endl;
	return tyUnknown;
}

bool NotExpression::isConstant() const {
	return expr->isConstant();
}

ULong NotExpression::getConstantValue() const {
	assert(isConstant());
	return !expr->getConstantValue();
}

void NotExpression::referenceDefinition() {
	expr->referenceDefinition();
}

void NotExpression::addToInstructionSequence(InstructionSequence * seq, bool returnValue) const {
	if (isConstant()) {
		if (returnValue)
			seq->push(getConstantValue());
	} else {
		expr->addToInstructionSequence(seq, returnValue);
		if (returnValue) {
			seq->addInstruction(new NotInstruction());
			seq->notifyStackChange(1,1);
		}
	}
}

/*** BinaryExpression, e.g. 4+6, false * -0xfa9 ***/

BinaryExpression::BinaryExpression(ExpressionPtr aLeft, BinaryOperator aOp, ExpressionPtr aRight, Scope &aScope, PreprocessorPosition aPos) : Expression(aScope, aPos) {
	left = aLeft;
	right = aRight;
	op = aOp;
}

BinaryExpression::~BinaryExpression() {}

void BinaryExpression::writeToStream(ostream &o) const {
	o << '(' << left << ' ' << op << ' ' << right << ')';
}

/*
	This function basically handles all error conditions due to wrong types 
	in binary expressions. Because of this, care has to be taken to prevent one
	typo from causing loads of errors from other expressions. Checks for
	tyUnknown do exactly this. Expressions with children that have type
	tyUnknown (i.e. have errors in them) will generally not generate any
	errors.
	The presumedType variable is used to try to emit a sensible type in any
	case, for example, "4 < bool" will have type tyBool because that's what
	"less than" operations emit, though they are not valid on these parameters.
*/
Type BinaryExpression::getType(Scope &scope) {
	assert (left);
	assert (right);
	Type leftType, rightType, presumedType;

	leftType = left->getType(scope);
	rightType = right->getType(scope);
	presumedType = tyUnknown;

	switch (op) {
	case boMult:
	case boDiv:
		if (leftType == tyFixed && rightType == tyFixed)
			return tyFixed;
		break;

	case boAdd:
	case boSub:
		if (leftType==tyUint && rightType==tyUint)
			return tyUint;
		if ((leftType==tyInt || leftType==tyUint) && (rightType==tyInt || rightType==tyUint))
			return tyInt;
		if (leftType==tyFixed && rightType==tyFixed)
			return tyFixed;
		break;

	case boAssign:
		if (leftType == tyVoid) {
			pos.prep.startError(pos) << "Error: no variable can be assigned a \"void\" value." << endl;
			return leftType;
		}

		if (!left->isAssignable()) {
			pos.prep.startError(pos) << "Error: left of \"=\" should be an l-value." << endl;
			return rightType;
		}

		if (leftType != rightType && leftType != tyUnknown && rightType != tyUnknown) {
			pos.prep.startError(pos) << "Error: incompatiable types: left of \"=\" has \"" << leftType << "\" type, right has \"" << rightType << "\"." << endl;
			return rightType;
		}

		if (leftType == rightType)
			return leftType;

		if (leftType!=tyUnknown)
			presumedType = leftType;
		else
			presumedType = rightType;
		break;

	case boEqual:
	case boNotEqual:
	case boGreater:
	case boGreaterEqual:
	case boLess:
	case boLessEqual:
		if (leftType == rightType)
			return tyBool;
		if ((leftType == tyInt || leftType == tyUint) && (rightType == tyInt || rightType == tyUint)) {
			pos.prep.startError(pos) << "Error: left of \"" << op << "\" has type \"" << leftType <<"\", right has type \"" << rightType << "\". Try using a type cast to int or uint." << endl;
			return tyBool;
		}
		presumedType = tyBool;
		break;

	case boOr:
	case boAnd:
		if (leftType == tyBool && rightType == tyBool)
			return tyBool;
		presumedType = tyBool;
		break;
	default:
		assert(false);
	}
	
	// There has been an error

	if (leftType!=tyUnknown && rightType!=tyUnknown)
		pos.prep.startError(pos) << "Error: operator \"" << op << "\" does not take a left side of type \"" <<
			leftType << "\" and a right side of type \"" << rightType << "\"." << endl;
		// Otherwise, errors in sub-expressions, that probably cause this error;
		// Don't emit an error.

	return presumedType;
}

bool BinaryExpression::hasSideEffect() const {
	return (op==boAssign);
}

bool BinaryExpression::isConstant() const {
	if (op == boAssign)
		return false;
	return left->isConstant() && right->isConstant();
}

ULong BinaryExpression::getConstantValue() const {
	assert(isConstant());
	switch (op) {
	case boMult:
		return (left->getConstantValue() * right->getConstantValue())>>6;
	case boDiv:
		return (left->getConstantValue()<<6) / right->getConstantValue();
	case boAdd:
		return left->getConstantValue() + right->getConstantValue();
	case boSub:
		return left->getConstantValue() - right->getConstantValue();

	case boEqual:
		return left->getConstantValue() == right->getConstantValue();
	case boNotEqual:
		return left->getConstantValue() != right->getConstantValue();
	case boGreater:
		return left->getConstantValue() > right->getConstantValue();
	case boGreaterEqual:
		return left->getConstantValue() >= right->getConstantValue();
	case boLess:
		return left->getConstantValue() < right->getConstantValue();
	case boLessEqual:
		return left->getConstantValue() <= right->getConstantValue();

	case boOr:
		return left->getConstantValue() || right->getConstantValue();
	case boAnd:
		return left->getConstantValue() && right->getConstantValue();
	default:
		assert(false);
	}
	return 0;
}


void BinaryExpression::referenceDefinition() {
	left->referenceDefinition();
	right->referenceDefinition();
}

void BinaryExpression::reorderLeftRight() {
	// returns true if optimising is on and left and right may be interchanged
	// and the current right value is is a constant and the current left value
	// is not. This is because the constant may then be coded more efficiently
	// through stack optimisation.
	if (!optimise)
		return;
	if (op == boMult || op == boAdd || op == boEqual || op == boNotEqual ||
		op == boOr || op == boAnd) {
		if (!left->isConstant() && right->isConstant()) {
			ExpressionPtr swap = left;
			left = right;
			right = swap;
		}
	}
}

void BinaryExpression::addToInstructionSequence (InstructionSequence * seq, bool returnValue) const {
	if (op == boAssign) {
		left->addAssignToInstructionSequence(seq, right, returnValue);
	} else {
		if (isConstant()) {
			if (returnValue)
				seq->push(getConstantValue());
		}
		else {
			// Optimise operand order if one is constant (this will often
			// reduce the number of push instructions)
			if (optimise &&
				(op == boMult || op == boAdd || op == boEqual || op == boNotEqual ||
				op == boOr || op == boAnd) &&
				!left->isConstant() && right->isConstant())
			{
				right->addToInstructionSequence (seq, true);
				left->addToInstructionSequence (seq, true);
			} else {
				left->addToInstructionSequence(seq, true);
				right->addToInstructionSequence(seq, true);
			}
			switch (op) {
			case boMult:
				seq->addInstruction(new MultiplyInstruction());
				seq->notifyStackChange(2,1);
				break;
			case boDiv:
				seq->addInstruction(new DivideInstruction());
				seq->notifyStackChange(2,1);
				break;
			case boAdd:
				seq->addInstruction(new AddInstruction());
				seq->notifyStackChange(2,1);
				break;
			case boSub:
				seq->addInstruction(new SubtractInstruction());
				seq->notifyStackChange(2,1);
				break;

			case boEqual:
				seq->addInstruction(new EqualInstruction());
				seq->notifyStackChange(2,1);
				break;
			case boNotEqual:
				seq->addInstruction(new NotEqualInstruction());
				seq->notifyStackChange(2,1);
				break;
			case boGreater:
				seq->addInstruction(new GreaterThanInstruction());
				seq->notifyStackChange(2,1);
				break;
			case boGreaterEqual:
				seq->addInstruction(new GreaterThanOrEqualInstruction());
				seq->notifyStackChange(2,1);
				break;
			case boLess:
				seq->addInstruction(new LessThanInstruction());
				seq->notifyStackChange(2,1);
				break;
			case boLessEqual:
				seq->addInstruction(new LessThanOrEqualInstruction());
				seq->notifyStackChange(2,1);
				break;

			case boOr:
				seq->addInstruction(new OrInstruction());
				seq->notifyStackChange(2,1);
				break;
			case boAnd:
				seq->addInstruction(new AndInstruction());
				seq->notifyStackChange(2,1);
				break;
			default:
				assert(false);
			}
			if (!returnValue) {
				seq->addInstruction(new PopInstruction());
				seq->notifyStackChange(1);
			}
		}
	}
}


/*** Functions that extract the expressions from the stream ***/
/*	These are called so that the operators with the highest precedence
	are checked first. All operators default to left-side precedence:
	3+4+5 is equivalent to (3+4)+5 and not 3+(4+5)
	the one exception is the = operator:
	x = y = 0 is equivalent to x = (y = 0).
*/

// getConstantExpression gets one constant from the stream

bool numericalOverflow(Preprocessor &prep, ULong value, ULong multiplier) {
	if (value > (maxULONG/multiplier)) {
		prep.startError() << "Syntax error: numerical overflow in constant." << endl;
		return true;
	}
	return false;
}

smart_ptr<ConstantExpression> getNumericalExpression (Scope &scope, Preprocessor &prep) {
	PreprocessorPosition pos = prep.getCurrentPosition();
	String buffer = prep.get();
	assert((buffer[0]>='0' && buffer[0]<='9') || buffer[0]=='.');
	ULong value = 0;
	ULong fractionSize = 1;
	ULong value2 = 0;

	Type type = tyInt;
	bool hex = 0;

	int i;
	for (i=0; i<buffer.length(); i++) {
		switch (buffer[i]) {
		case '0': case '1': case '2': case '3': case '4': case '5': 
		case '6': case '7': case '8': case '9':
			if (!hex) {
				if (type==tyInt) {
					if (numericalOverflow(prep, value, 10))
						return new ConstantExpression(type, value, scope, pos);
					value *= 10;
					value += buffer[i] - '0';
				} else {
					if (numericalOverflow(prep, value2, 10<<6))
						return new ConstantExpression(type, value, scope, pos);
					value2 *= 10;
					value2 += buffer[i] - '0';
					fractionSize *=10;
				}
			} else {
				assert(type==tyInt);
				if (numericalOverflow(prep, value, 16))
					return new ConstantExpression(type, value, scope, pos);
				value *= 16;
				value += buffer[i] - '0';
			}
			break;

		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'r':
			if (!hex) {
				prep.startError() << "Syntax error: unexpected \"" << buffer[i]
					<< "\" in constant. N.B.: hexadecimal number are typed with \"0x\", e.g. 0xf0a." << endl;
				return new ConstantExpression(type, value, scope, pos);
			} else {
				assert(type==tyInt);
				if (numericalOverflow(prep, value, 16))
					return new ConstantExpression(type, value, scope, pos);
				value *=16;
				if (buffer[i] < 'Z')
					value += buffer[i] - 'A' + 10;
				else
					value += buffer[i] - 'a' + 10;
			}
			break;

		case 'p': case 'P':
			if (i != buffer.length()-1) {
				prep.startError() << "Syntax error: unexpected \"" << buffer[i]
					<< "\" in constant." << endl;
				return new ConstantExpression(type, value, scope, pos);
			}
			if (type==tyFixed)
				break;
			// Else set type to tyFixed:
		case '.':
			if (type==tyInt && !hex) {
				type = tyFixed;
				value = value << 6;
			} else {
				prep.startError() << "Syntax error: unexpected \".\" in constant." << endl;
				return new ConstantExpression(type, value, scope, pos);
			}
			break;

		case 'u': case 'U':
			if (i == buffer.length()-1) {
				if (type==tyInt)
					type = tyUint;
				else {
					assert(type==tyFixed);
					prep.startError() << "Syntax error: \"u\" after constant: fixed numbers are always signed." << endl;
					return new ConstantExpression(type, value, scope, pos);
				}
			} else {
				prep.startError() << "Syntax error: unexpected \"" << buffer[i] << "\" in constant." << endl;
				return new ConstantExpression(type, value, scope, pos);
			}
			break;

		case 'x':
			if (type==tyInt && value==0 && i == 1 && !hex) {
				hex = true;
				break;
			}
			// Else error

		default:
			prep.startError() << "Syntax error: unexpected \"" << buffer[i] << "\" after constant." << endl;
			return new ConstantExpression(type, value, scope, pos);
			break;
		}
	}

	if (type==tyFixed)
		value += ((value2 << 6) + (fractionSize/2))/fractionSize;
	if (type==tyInt)
		numericalOverflow(prep, value, 2);
	return new ConstantExpression(type, value, scope, pos);	
}

smart_ptr <ConstantExpression> getConstantExpression(Scope &scope, Preprocessor &prep) {
	String buffer;
	PreprocessorPosition pos = prep.getCurrentPosition();
	buffer = prep.peek();
	if (buffer == "false") {
		prep.deleteToken();
		return new ConstantExpression(tyBool, 0, scope, pos);
	}
	if (buffer == "true") {
		prep.deleteToken();
		return new ConstantExpression(tyBool, 1, scope, pos);
	}

	if (isNumberChar(buffer[0]) || (buffer[0]=='.' && isNumberChar(buffer[1])))
		return getNumericalExpression(scope, prep);
	return NULL;
}

// getFunctionCallParameters extracts a number of items of the form (param1, param2, param3)

FunctionCallParameters getFunctionCallParameters(Scope &scope, Preprocessor &prep) {
	String buffer;
	assert(prep.peek() == "(");
	prep.deleteToken();

	buffer = prep.peek();
	if (buffer == ")") {
		prep.deleteToken();
		return FunctionCallParameters ();
	}

	FunctionCallParameters params;

	while (true) {
		ExpressionPtr expr = getExpression(scope, prep);
		if (!expr) {
			prep.startError() << "Syntax error: expression expected instead of \"" << buffer << "\"." << endl;
			return params;
		}

		params.push_back (expr);

		buffer = prep.get();
		if (buffer != ",") {
			if (buffer == ")")
				return params;
			// Else, an error
			prep.startError() << "Syntax error: \",\" or \")\" expected instead of \"" << buffer << "\"." << endl;
			return params;
		}
		// Otherwise go on by reading the next expression.
	}
	// This is to prevent a compiler warning
	return FunctionCallParameters();
}

// getSimpleExpression extracts bracketed expressions, negated expressions (-x),
// constant expressions, variable calls, function calls and type casts.

ExpressionPtr getSimpleExpression(Scope &scope, Preprocessor &prep) {
	ExpressionPtr expr;
	String buffer;
	PreprocessorPosition pos = prep.getCurrentPosition();
	buffer = prep.peek();

	// Bracketed expression
	if (buffer == "(") {
		prep.deleteToken();
		expr = getExpression(scope, prep);
		buffer = prep.peek();
		if (buffer != ")") {
			prep.startError() << "Syntax error: \")\" expected instead of \"" << buffer << "\"." << endl;
		} else
			prep.deleteToken();
		return expr;
	}

	// Negated expression
	if (buffer == "-") {
		prep.deleteToken();
		ExpressionPtr expr = getSimpleExpression(scope, prep);
		if (!expr) {
			buffer = prep.peek();
			prep.startError() << "Syntax error: expression expected instead of \"" << buffer << "\" after \"-\"." << endl;
			return NULL;
		}
		return new NegateExpression(expr, scope, pos);
	}

	if (buffer == "!") {
		prep.deleteToken();
		ExpressionPtr expr = getSimpleExpression(scope, prep);
		if (!expr) {
			buffer = prep.peek();
			prep.startError() << "Syntax error: expression expected instead of \"" << buffer << "\" after \"!\"." << endl;
			return NULL;
		}
		return new NotExpression(expr, scope, pos);
	}

	// Constant expression
	expr = getConstantExpression(scope, prep);
	if (expr) return expr;

	// Variable or function call
	if (isLetterChar(buffer[0]) || buffer[0]=='_') {
		String name = buffer;
		prep.deleteToken();
		buffer = prep.peek();
		if (buffer != "(")
			return new VariableExpression(name, scope, pos);
		else {
			Type castType = getType(name);
			if (castType==tyUnknown)
				return new FunctionCallExpression(name, getFunctionCallParameters(scope, prep), scope, pos);
			else {
				prep.deleteToken();	// Delete "("
				expr = new TypeCastExpression(castType, getExpression(scope, prep), scope, pos);
				buffer = prep.peek();
				if (buffer != ")") {
					prep.startError() << "Syntax Error: \")\" expected after type cast expression." << endl;
				} else
					prep.deleteToken();
				return expr;
			}
		}
	}

	// Error
	return NULL;
}

// getMultiplyExpression extracts * and / expressions

ExpressionPtr getMultiplyExpression(Scope &scope, Preprocessor &prep) {
	ExpressionPtr expr = getSimpleExpression(scope, prep);
	PreprocessorPosition pos = prep.getCurrentPosition();
	if (!expr)
		return NULL;
	do {
		String buffer;
		buffer = prep.peek();
		if (buffer == "*") {
			prep.deleteToken();
			expr = new BinaryExpression(expr, boMult, getSimpleExpression(scope, prep), scope, pos);
		} else {
			if (buffer == "/") {
				prep.deleteToken();
				expr = new BinaryExpression(expr, boDiv, getSimpleExpression(scope, prep), scope, pos);
			} else
				return expr;
		}
	} while (true);
}

// getAddExpression extracts + and - expressions

ExpressionPtr getAddExpression(Scope &scope, Preprocessor &prep) {
	ExpressionPtr expr = getMultiplyExpression(scope, prep);
	PreprocessorPosition pos = prep.getCurrentPosition();
	if (!expr)
		return NULL;
	do {
		String buffer;
		buffer = prep.peek();
		if (buffer == "+") {
			prep.deleteToken();
			expr = new BinaryExpression(expr, boAdd, getMultiplyExpression(scope, prep), scope, pos);
		} else {
			if (buffer == "-") {
				prep.deleteToken();
				expr = new BinaryExpression(expr, boSub, getMultiplyExpression(scope, prep), scope, pos);
			} else
				return expr;
		}
	} while (true);
}

// getComparisonExpression extracts comparison expressions (e.g. ==, >)

ExpressionPtr forcedGetAddExpression (Scope &scope, Preprocessor &prep) {
	ExpressionPtr expr = getAddExpression (scope, prep);
	if (expr)
		return expr;
	prep.startError() << "Missing right element in binary expression; found \"" <<
		prep.peek() << "\" instead." << endl;
	return NULL;
}

ExpressionPtr getComparisonExpression(Scope &scope, Preprocessor &prep) {
	ExpressionPtr expr = getAddExpression(scope, prep);
	PreprocessorPosition pos = prep.getCurrentPosition();
	if (!expr)
		return NULL;
	do {
		String buffer;
		buffer = prep.peek();
		if (buffer == "==") {
			prep.deleteToken();
			expr = new BinaryExpression(expr, boEqual, forcedGetAddExpression(scope, prep), scope, pos);
		} else {
			if (buffer == "!=") {
				prep.deleteToken();
				expr = new BinaryExpression(expr, boNotEqual, forcedGetAddExpression(scope, prep), scope, pos);
			} else {
				if (buffer == ">") {
					prep.deleteToken();
					expr = new BinaryExpression(expr, boGreater, forcedGetAddExpression(scope, prep), scope, pos);
				} else {
					if (buffer == ">=") {
						prep.deleteToken();
						expr = new BinaryExpression(expr, boGreaterEqual, forcedGetAddExpression(scope, prep), scope, pos);
					} else {
						if (buffer == "<") {
							prep.deleteToken();
							expr = new BinaryExpression(expr, boLess, forcedGetAddExpression(scope, prep), scope, pos);
						} else {
							if (buffer == "<=") {
								prep.deleteToken();
								expr = new BinaryExpression(expr, boLessEqual, forcedGetAddExpression(scope, prep), scope, pos);
							} else
								return expr;
						}
					}
				}
			}
		}
	} while (true);
}

// getLogicalExpression extracts || and && expressions.

ExpressionPtr getLogicalExpression(Scope &scope, Preprocessor &prep) {
	ExpressionPtr expr = getComparisonExpression(scope, prep);
	PreprocessorPosition pos = prep.getCurrentPosition();
	if (!expr)
		return NULL;
	do {
		String buffer;
		buffer = prep.peek();
		if (buffer == "||") {
			prep.deleteToken();
			expr = new BinaryExpression(expr, boOr, getComparisonExpression(scope, prep), scope, pos);
		} else {
			if (buffer == "&&") {
				prep.deleteToken();
				expr = new BinaryExpression(expr, boAnd, getComparisonExpression(scope, prep), scope, pos);
			} else
				return expr;
		}
	} while (true);
}

// getAssignExpression extracts the = expression.
// The precedence order is different from the other expressions.

ExpressionPtr getAssignExpression(Scope &scope, Preprocessor &prep) {
	ExpressionPtr expr = getLogicalExpression(scope, prep);
	PreprocessorPosition pos = prep.getCurrentPosition();
	if (!expr)
		return NULL;
	String buffer;
	buffer = prep.peek();
	if (buffer == "=") {
		prep.deleteToken();
		return new BinaryExpression(expr, boAssign, getAssignExpression(scope, prep), scope, pos);
	} else
		return expr;
}

// getExpression extracts an expression from the stream

ExpressionPtr getExpression(Scope &scope, Preprocessor &prep) {
	ExpressionPtr expr = getAssignExpression(scope, prep);
	if (!expr) {
		String buffer;
		buffer = prep.get();
		prep.startError() << "Syntax error: expression expected instead of \"" << buffer << "\"." << endl;
	}
	return expr;
}

// getType returns the type in buffer

Type getType(String buffer) {
	if (buffer == "void")
		return tyVoid;
	if (buffer == "int")
		return tyInt;
	if (buffer == "uint")
		return tyUint;
	if (buffer == "fixed")
		return tyFixed;
	if (buffer == "bool")
		return tyBool;

	return tyUnknown;
}
