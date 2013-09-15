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
	\file InstructionSequence is a sequence of instructions that is produced
	from a .TTI-file.
*/

#ifndef INSTRUCTIONSEQUENCE_H
#define INSTRUCTIONSEQUENCE_H

#include <vector>
#include <stack>
#include "../InstructionProcessor/InstructionProcessor.h"

using std::vector;
using std::stack;
using util::smart_ptr;
using namespace OpenType;

extern bool optimise;

class Expression;
class InstructionPosition;
class PushValue;
class MultiplePushInstruction;
class InstructionSequence;

typedef struct {
	smart_ptr <MultiplePushInstruction> instr;
	int stackSize;
	int generated;
} StackInstruction;

typedef smart_ptr <Expression> ExpressionPtr;
typedef smart_ptr <InstructionSequence> InstructionSequencePtr;

typedef smart_ptr <Instruction> InstructionPtr;
typedef std::vector <InstructionPtr> Instructions;

typedef smart_ptr <InstructionPosition> InstructionPositionPtr;
typedef vector <InstructionPositionPtr> InstructionPositions;

class InstructionSequence {
	Instructions instructions;

	InstructionPositions optimisationBoundaries;
	int curStackSize;
	stack <StackInstruction> pushInstructionStack;

	bool pointerKnown[6];
	UShort pointerValues[6];
	bool automaticReference0Setter;
	UShort automaticReference0Value;
	Byte increaseInstructionWith;
	bool shouldBeSet(int pointer, const ExpressionPtr expr);
	void setPointer(int pointer, const ExpressionPtr expr);

	ULong getCurrentByteLength();

	bool optimised;

public:
	InstructionSequence(bool isGlyphProgram);
	virtual ~InstructionSequence();

	void addInstruction(InstructionPtr instruction);
	void addSequence(smart_ptr <InstructionSequence> seq);

	void push (smart_ptr <PushValue> value);
	void push (Long value);
	void notifyStackChange(int deleted, int added = 0);
	bool emptyStack();

	void setZonePointer(Byte zp, Byte zone);
	void setPointers (const ExpressionPtr zp0, const ExpressionPtr zp1 = NULL, const ExpressionPtr zp2 = NULL,
	                 const ExpressionPtr rp0 = NULL, const ExpressionPtr rp1 = NULL, const ExpressionPtr rp2 = NULL);
	void notifyPointers (const ExpressionPtr rp0, const ExpressionPtr rp1 = NULL, const ExpressionPtr rp2 = NULL);
	void notifyAutomaticRef0 (const ExpressionPtr expr, Byte with);
	void notifyUnknownPointers();

	InstructionPositionPtr setOptimisationBoundaryHere();

	bool empty();
	MemoryBlockPtr getMemory();
};

class InstructionPosition {
	const Instructions *instructions;
	Instructions::size_type position;
protected:
	bool attachedToInstruction();
	friend class InstructionSequence;
	void setInstruction (const Instructions *aInstructions, Instructions::size_type aPosition);
public:
	InstructionPosition();
	virtual ~InstructionPosition();

	ULong getDistance(InstructionPositionPtr second) const;
};

class PushValue {
public:
	PushValue();
	virtual ~PushValue() {}

	virtual Short getValue() const = 0;
	virtual bool isByte() const = 0;
	virtual void optimiseByteSize() = 0;
};

typedef smart_ptr <PushValue> PushValuePtr;
typedef vector <PushValuePtr> PushValues;

class ConstantPushValue : public PushValue {
	Short value;
public:
	ConstantPushValue(Short aValue);
	virtual ~ConstantPushValue() {}

	virtual Short getValue() const;
	virtual bool isByte() const;
	virtual void optimiseByteSize() {}
};

class DistancePushValue : public PushValue {
	InstructionPositionPtr pos1, pos2;
	bool forward;
	Short add;
	bool byte;
public:
	DistancePushValue();
	virtual ~DistancePushValue() {}
	void setDistance(InstructionPositionPtr aPos1, InstructionPositionPtr aPos2,
		bool aForward, Short aAdd);

	virtual Short getValue() const;
	virtual bool isByte() const;
	virtual void optimiseByteSize();
};

#endif // INSTRUCTIONSEQUENCE_H
