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

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <cassert>
#include "../OTFont/OTMemoryBlock.h"
#include "../InstructionProcessor/Instructions.h"
#include "InstructionSequence.h"
#include "Expression.h"


class MultiplePushInstruction : public Instruction {
	PushValues values;
public:
	MultiplePushInstruction(PushValuePtr aValue);
	MultiplePushInstruction();
	virtual ~MultiplePushInstruction();

	void insertValue (PushValuePtr aValue, unsigned int position = 0);
	virtual void execute (InstructionProcessor & proc) const;
	virtual String getName() const;

	virtual ULong getByteSize() const;
	virtual void optimiseByteSize();
	virtual void write (MemoryWritePen &pen) const;
};

class SequenceInstruction : public Instruction {
	MemoryBlockPtr memory;
public:
	SequenceInstruction (smart_ptr <InstructionSequence> aSequence);
	virtual ~SequenceInstruction();
	virtual void execute (InstructionProcessor & proc) const;
	virtual String getName() const;

	virtual void write (MemoryWritePen &pen) const;
	virtual ULong getByteSize();
};

/*** InstructionSequence ***/

InstructionSequence::InstructionSequence (bool isGlyphProgram)
: curStackSize (0) {
	smart_ptr <MultiplePushInstruction> pushInstruction = new MultiplePushInstruction();
	StackInstruction s;
	s.instr = pushInstruction;
	s.stackSize = 0;
	s.generated = 1;
	pushInstructionStack.push (s);
	addInstruction (pushInstruction);

	//addInstruction(lastPushInstruction = new MultiplePushInstruction());

	if (isGlyphProgram) {
		pointerKnown[0] = pointerKnown[1] = pointerKnown[2] = 
			pointerKnown[3] = pointerKnown[4] = pointerKnown[5] = true;
		pointerValues[0] = pointerValues[1] = pointerValues[2] = 1;
		pointerValues[3] = pointerValues[4] = pointerValues[5] = 0;
	} else {
		pointerKnown[0] = pointerKnown[1] = pointerKnown[2] = 
			pointerKnown[3] = pointerKnown[4] = pointerKnown[5] = false;
	}

	automaticReference0Setter = false;
	automaticReference0Value = 0;
	increaseInstructionWith = 0;

	optimised = false;
}

InstructionSequence::~InstructionSequence() {}

void InstructionSequence::addInstruction (InstructionPtr instruction) {
	instructions.push_back (instruction);

	automaticReference0Setter = false;
}

void InstructionSequence::addSequence(smart_ptr <InstructionSequence> seq) {
	if (!seq->empty()) {
		addInstruction(new SequenceInstruction (seq));
	}
}

void InstructionSequence::push (PushValuePtr value) {
	if (!optimise)
		addInstruction(new MultiplePushInstruction(value));
	else
		pushInstructionStack.top().instr->insertValue (value, curStackSize -
			pushInstructionStack.top().stackSize);
	curStackSize ++;
}

void InstructionSequence::push (Long value) {
	// Cases like these should be handled gracefully
	if (value < 0x10000 && value >= -0x10000)
		push (new ConstantPushValue ((Short) value));
	else {
		if (value > 0) {
			ULong masked = value & 0xFF;
			if (masked)
				push (new ConstantPushValue ((Short) masked));

			push (value >> 8);
			push (new ConstantPushValue ((Short)0x4000));
			addInstruction (new MultiplyInstruction());
			notifyStackChange (2, 1);

			if (masked) {
				addInstruction (new AddInstruction());
				notifyStackChange (2, 1);
			}
		} else {
			value = -value;
			ULong masked = value & 0xFF;
			if (masked)
				push (new ConstantPushValue ((Short) -masked));

			push (value >> 8);
			push (new ConstantPushValue ((Short)-0x4000));
			addInstruction (new MultiplyInstruction());
			notifyStackChange (2, 1);

			if (masked) {
				addInstruction (new AddInstruction());
				notifyStackChange (2, 1);
			}
		}
	}
}

void InstructionSequence::notifyStackChange(int deleted, int added) {
	assert(deleted <= curStackSize);
	curStackSize -= deleted;
	while (pushInstructionStack.top().stackSize - pushInstructionStack.top().generated >= curStackSize)
		pushInstructionStack.pop();
	
	if (pushInstructionStack.top().stackSize > curStackSize) {
		smart_ptr <MultiplePushInstruction> i = new MultiplePushInstruction();
		pushInstructionStack.top().generated -= pushInstructionStack.top().stackSize - curStackSize;
		pushInstructionStack.top().stackSize = curStackSize;
		pushInstructionStack.top().instr = i;
		addInstruction (i);
	}

	if (added) {
		curStackSize += added;
		StackInstruction newStack;
		smart_ptr <MultiplePushInstruction> i = new MultiplePushInstruction();
		newStack.instr = i;
		addInstruction(i);
		newStack.stackSize = curStackSize;
		newStack.generated = added;
		pushInstructionStack.push (newStack);
	}
}

bool InstructionSequence::emptyStack() {
	return curStackSize==0;
}

void InstructionSequence::setZonePointer(Byte zp, Byte zone) {
	assert(zp<=2);
	if (!pointerKnown[zp] || zone != pointerValues[zp]) {
		push(zone);
		addInstruction(new SetZonePointerInstruction(zp));
		notifyStackChange(1);
		pointerKnown[zp] = true;
		pointerValues[zp] = zone;
	}
}

bool InstructionSequence::shouldBeSet (int pointer, const ExpressionPtr expr) {
	if (!expr)
		return false;
	if (!optimise) return true;
	if (pointerKnown[pointer] && expr->isConstant() && expr->getConstantValue() == pointerValues[pointer]) {
		return false;
	}
	pointerKnown[pointer] = expr->isConstant();
	if (pointerKnown[pointer]) {
		UShort newValue = expr->getConstantValue();
		pointerValues[pointer] = expr->getConstantValue();
		if (pointer==3 && automaticReference0Setter && automaticReference0Value==newValue) {
			(*(instructions.end() - 1))->increaseInstruction (increaseInstructionWith);
			return false;
		}
	}
	return true;
}

void InstructionSequence::setPointers (const ExpressionPtr zp0, const ExpressionPtr zp1, const ExpressionPtr zp2,
									  const ExpressionPtr rp0, const ExpressionPtr rp1, const ExpressionPtr rp2) {
	// The particular order (first expressions, then set instructions) is used
	// to make sure the expressions don't interfere by setting pointers themselves.
	bool setZp0, setZp1, setZp2, setRp0, setRp1, setRp2;

	setZp0 = shouldBeSet(0, zp0);
	setZp1 = shouldBeSet(1, zp1);
	setZp2 = shouldBeSet(2, zp2);
	setRp0 = shouldBeSet(3, rp0);
	setRp1 = shouldBeSet(4, rp1);
	setRp2 = shouldBeSet(5, rp2);

	if (setZp0)
		zp0->addToInstructionSequence(this, true);
	if (setZp1)
		zp1->addToInstructionSequence(this, true);
	if (setZp2)
		zp2->addToInstructionSequence(this, true);
	if (setRp0)
		rp0->addToInstructionSequence(this, true);
	if (setRp1)
		rp1->addToInstructionSequence(this, true);
	if (setRp2)
		rp2->addToInstructionSequence(this, true);

	if (setRp2) {
		addInstruction(new SetReferencePointInstruction(2));
		notifyStackChange(1);
	}
	if (setRp1) {
		addInstruction(new SetReferencePointInstruction(1));
		notifyStackChange(1);
	}
	if (setRp0) {
		addInstruction(new SetReferencePointInstruction(0));
		notifyStackChange(1);
	}
	if (setZp2) {
		addInstruction(new SetZonePointerInstruction(2));
		notifyStackChange(1);
	}
	if (setZp1) {
		addInstruction(new SetZonePointerInstruction(1));
		notifyStackChange(1);
	}
	if (setZp0) {
		addInstruction(new SetZonePointerInstruction(0));
		notifyStackChange(1);
	}
}

void InstructionSequence::setPointer(int pointer, const ExpressionPtr expr) {
	if (expr) {
		if (expr->isConstant()) {
			pointerKnown[pointer] = true;
			pointerValues[pointer] = expr->getConstantValue();
		} else
			pointerKnown[pointer] = false;
	}
}

void InstructionSequence::notifyPointers (const ExpressionPtr rp0, const ExpressionPtr rp1, const ExpressionPtr rp2) {
	setPointer(3, rp0);
	setPointer(4, rp1);
	setPointer(5, rp2);
}

void InstructionSequence::notifyAutomaticRef0 (ExpressionPtr ref0Expr, Byte with) {
	if (!ref0Expr->isConstant())
		automaticReference0Setter = false;
	else {
		automaticReference0Setter = true;
		automaticReference0Value = ref0Expr->getConstantValue();
		increaseInstructionWith = with;
	}

}

void InstructionSequence::notifyUnknownPointers() {
	pointerKnown[0] = pointerKnown[1] = pointerKnown[2] = 
		pointerKnown[3] = pointerKnown[4] = pointerKnown[5] = false;
}

bool InstructionSequence::empty() {
	// An empty sequence contains only an empty push instruction
	return instructions.size()==1;
}

InstructionPositionPtr InstructionSequence::setOptimisationBoundaryHere() {
	assert(emptyStack());
	assert(pushInstructionStack.size() == 1);
	notifyUnknownPointers();
	automaticReference0Setter = false;

	InstructionPositionPtr newOptimisationBoundary = new InstructionPosition();
	smart_ptr <MultiplePushInstruction> i = new MultiplePushInstruction();
	pushInstructionStack.top().instr = i;
	newOptimisationBoundary->setInstruction (&instructions, instructions.size());
	addInstruction (i);

	optimisationBoundaries.push_back (newOptimisationBoundary);
	return newOptimisationBoundary;
}

ULong InstructionSequence::getCurrentByteLength() {
	ULong length = 0;
	Instructions::iterator cur;

	for (cur = instructions.begin(); cur != instructions.end(); cur ++)
		length += (*cur)->getByteSize();
	return length;
}

MemoryBlockPtr InstructionSequence::getMemory() {
	// Optimise
	if (optimise && !optimised) {
		ULong oldLength;
		ULong newLength = getCurrentByteLength();
		do {
			oldLength = newLength;
			Instructions::iterator cur;
			for (cur = instructions.begin(); cur != instructions.end(); cur ++)
				(*cur)->optimiseByteSize();
			newLength = getCurrentByteLength();
		} while (newLength < oldLength);
		optimised = true;
	}

	assert(curStackSize == 0);

	MemoryBlockPtr memory (new MemoryBlock);
	MemoryWritePen pen (memory);
	Instructions::iterator cur;
	for (cur = instructions.begin(); cur != instructions.end(); cur ++)
		(*cur)->write (pen);

	return memory;
}


/*** SequenceInstruction ***/

SequenceInstruction::SequenceInstruction (smart_ptr <InstructionSequence> sequence) : Instruction(0) {
	memory = sequence->getMemory();
}

SequenceInstruction::~SequenceInstruction() {}

void SequenceInstruction::execute (InstructionProcessor & proc) const {
	assert (false);
}

String SequenceInstruction::getName() const {
	return "<Multiple instructions>";
}

ULong SequenceInstruction::getByteSize() {
	return memory->getSize();
}

void SequenceInstruction::write (MemoryWritePen &pen) const {
	pen.writeBlock (memory);
}

/*** InstructionPosition ***/

InstructionPosition::InstructionPosition() : instructions (NULL), position (0) {}

InstructionPosition::~InstructionPosition() {}

bool InstructionPosition::attachedToInstruction() {
	return instructions != NULL;
}

void InstructionPosition::setInstruction (const Instructions *aInstructions,
										  Instructions::size_type aPosition) {
	instructions = aInstructions;
	position = aPosition;
}

ULong InstructionPosition::getDistance (InstructionPositionPtr second) const {
	if (instructions == NULL)
		return 0;
	ULong distance = 0;
	Instructions::const_iterator cur, end;

	if (second->instructions)
		end = second->instructions->begin() + second->position;
	else
		end = instructions->end();

	for (cur = instructions->begin() + position; cur != end; cur ++)
		distance += (*cur)->getByteSize();

	return distance;
}

/*** MultiplePushInstruction ***/

MultiplePushInstruction::MultiplePushInstruction(PushValuePtr aValue) : Instruction(oiPUSHW) {
	values.push_back (aValue);
}

MultiplePushInstruction::MultiplePushInstruction() : Instruction(oiPUSHW) {}

MultiplePushInstruction::~MultiplePushInstruction() {}

void MultiplePushInstruction::execute (InstructionProcessor & proc) const {
	assert (false);
}

String MultiplePushInstruction::getName() const {
	return "<Push instruction>";
}

void MultiplePushInstruction::insertValue (PushValuePtr aValue, unsigned int position) {
	assert(position >= 0 && position <= values.size());

	values.insert (values.begin() + position, aValue);
}

ULong MultiplePushInstruction::getByteSize() const {
	if (values.empty())
		return 0;
	else {
		ULong size = 0;
		//vector <bool> isByte;
		PushValues::const_iterator i;
		/*for (i = values.begin(); i != values.end(); i ++)
			isByte.push_back((*i)->isByte());*/

		i = values.begin();
		while (i != values.end()) {
			PushValues::const_iterator j = i;
			PushValues::const_iterator k;
			// code bytes
			while (j != values.end() && (*j)->isByte())
				j++;
			if (j != i) {
				if (j-i <= 8) {
					size++;
				} else {
					if (j-i >= 256)
						j = i + 256;
					size ++;
					size ++;
				}
				for (k = i; k < j; k ++) {
					size ++;
				}
				i = j;
			}

			// code words
			j = i;
			while (j != values.end() && !(*j)->isByte())
				j++;
			if (j != i) {
				if (j-i <= 8) {
					size++;
				} else {
					if (j-i>=256)
						j = i+256;
					size++;
					size++;
				}
				for (k=i; k<j; k++) {
					size += sizeof(UShort);
				}
				i = j;
			}
		}
		return size;
	}
}

void MultiplePushInstruction::write (MemoryWritePen &pen) const {
	if (!values.empty()) {
		PushValues::const_iterator i;

		i = values.begin();
		while (i != values.end()) {
			PushValues::const_iterator j = i;
			PushValues::const_iterator k;
			// code bytes
			while (j != values.end() && (*j)->isByte())
				j++;
			if (j != i) {
				if (j-i <= 8)
					pen.writeByte (oiPUSHB + (j-i-1));
				else {
					if (j-i >= 256)
						j = i+255;
					pen.writeByte (oiNPUSHB);
					pen.writeByte ((Byte)(j-i));
				}
				for (k=i; k<j; k++)
					pen.writeByte ((Byte) (*k)->getValue());
				i = j;
			}

			// code words
			j = i;
			while (j != values.end() && !(*j)->isByte())
				j++;
			if (j != i) {
				if (j-i <= 8)
					pen.writeByte (oiPUSHW + (j-i-1));
				else {
					if (j-i >= 256)
						j = i + 256;
					pen.writeByte (oiNPUSHW);
					pen.writeByte ((Byte)(j-i));
				}
				for (k = i; k < j; k++)
					pen.writeUShort ((*k)->getValue());
				i = j;
			}
		}
	}
}

void MultiplePushInstruction::optimiseByteSize() {
	PushValues::iterator i;
	for (i = values.begin(); i != values.end(); i++)
		(*i)->optimiseByteSize();
}

/*** PushValue ***/

PushValue::PushValue() {}

Short PushValue::getValue() const {
	assert(false);
	return 0;
}

bool PushValue::isByte() const {
	assert(false);
	return false;
}

void PushValue::optimiseByteSize() {}

/*** ConstantPushValue ***/

ConstantPushValue::ConstantPushValue(Short aValue) {
	value = aValue;
}

Short ConstantPushValue::getValue() const {
	return value;
}

bool ConstantPushValue::isByte() const {
	return value >= 0 && value < 256;
}

/*** DistancePushValue ***/

DistancePushValue::DistancePushValue() {
	pos1 = NULL;
	pos2 = NULL;
	byte = false;
}


void DistancePushValue::setDistance(InstructionPositionPtr aPos1, InstructionPositionPtr aPos2,
									bool aForward, Short aAdd) {
	pos1 = aPos1;
	pos2 = aPos2;
	forward = aForward;
	add = aAdd;
}

Short DistancePushValue::getValue() const {
	assert(pos1);
	assert(pos2);
	Short value = pos1->getDistance(pos2);
	if (forward)
		return value + add;
	else
		return -value + add;
}

bool DistancePushValue::isByte() const {
	return byte;
}

void DistancePushValue::optimiseByteSize() {
	Short val = getValue();
	if (val>=0 && val<256)
		byte = true;
	else
		assert(byte==false);
}
