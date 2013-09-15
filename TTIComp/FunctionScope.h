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
	\file A Scope that includes all the function definitions necessary for 
	Truetype instructions.
*/

#ifndef FUNCTIONSCOPE_H
#define FUNCTIONSCOPE_H

#include "Scope.h"
#include "Statement.h"
#include "TTICompPreprocessor.h"

class FunctionScope : public MainScope {
public:
	FunctionScope(TTICompPreprocessor &aPrep);
	virtual void addPredefinedDefinitions();
};

class PredefinedFunctionDefinition : public FunctionDefinitionStatement {
public:
	PredefinedFunctionDefinition(Scope &aScope, TTICompPreprocessor &prep, Type aType,
		String aName, const FormalParameters &aFormalParameters);
		// Virtual functions redefined to do nothing
	virtual void writeToStream(ostream &o) const;
	virtual void setCalled(bool aInGlyphProgram = false);
};

/*** Predefined functions ***/

// CVT

class SetCVT : public PredefinedFunctionDefinition {
	bool pixels;
public:
	SetCVT(Scope &aScope, TTICompPreprocessor &prep, bool aPixels);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class GetCVT : public PredefinedFunctionDefinition {
public:
	GetCVT(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

// SetVector

class SetVectorToAxis : public PredefinedFunctionDefinition {
	bool projection;
	bool freedom;
	bool x;
public:
	SetVectorToAxis(Scope &aScope, TTICompPreprocessor &prep, bool aProjection, bool aFreedom, bool aX);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class SetVectorLine : public PredefinedFunctionDefinition {
	bool projection;
	bool dual;
	bool perp;
public:
	SetVectorLine(Scope &aScope, TTICompPreprocessor &prep, bool aProjection, bool aDual, bool perp);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class SetFreedomProjection : public PredefinedFunctionDefinition {
public:
	SetFreedomProjection(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class SetVector : public PredefinedFunctionDefinition {
	bool projection;
public:
	SetVector(Scope &aScope, TTICompPreprocessor &prep, bool aProjection);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

/*  GetProjectionVector() and GetFreedomVector() are not provided, as they would need a
	return type "Vector" or "fixed fixed" or something, which does not fit with the TTI
	syntax. It would be possible to introduce a type "Vector", or to provide functions
	fixed getProjectionVectorX()
	fixed getProjectionVectorY()
	fixed getFreedomVectorX()
	fixed getFreedomVectorY()
	This may be done in the future; I'm not sure however whether it is ever used in
	instructions (never seen it yet, I believe).
*/

/*** Graphics State functions ***/

// Round state

typedef enum _RoundType {
	roHalf, roGrid, roDouble, roDown, roUp, roOff, roSuper, ro45
} RoundType;

class SetRound : public PredefinedFunctionDefinition {
	RoundType roundType;
public:
	SetRound(Scope &aScope, TTICompPreprocessor &prep, RoundType aType);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class SuperRound : public PredefinedFunctionDefinition {
	bool round45;
public:
	SuperRound(Scope &aScope, TTICompPreprocessor &prep, bool a45);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

// Other Graphics State items

class SetMinimumDistance : public PredefinedFunctionDefinition {
public:
	SetMinimumDistance(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class InstructionExecutionControl : public PredefinedFunctionDefinition {
public:
	InstructionExecutionControl(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class ScanConversionControl : public PredefinedFunctionDefinition {
public:
	ScanConversionControl(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class ScanType : public PredefinedFunctionDefinition {
public:
	ScanType(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class SetCVTCutIn : public PredefinedFunctionDefinition {
public:
	SetCVTCutIn(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class SetSingleWidth : public PredefinedFunctionDefinition {
public:
	SetSingleWidth(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class SetSingleWidthCutIn : public PredefinedFunctionDefinition {
public:
	SetSingleWidthCutIn(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class SetAutoFlip : public PredefinedFunctionDefinition {
	bool on;
public:
	SetAutoFlip(Scope &aScope, TTICompPreprocessor &prep, bool aOn);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class SetDeltaBase : public PredefinedFunctionDefinition {
public:
	SetDeltaBase(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class SetDeltaShift : public PredefinedFunctionDefinition {
public:
	SetDeltaShift(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

//

class GetPPEM : public PredefinedFunctionDefinition {
public:
	GetPPEM(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class GetPointSize : public PredefinedFunctionDefinition {
public:
	GetPointSize(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

/*** Manipulate Point functions ***/

class GetCoordinate : public PredefinedFunctionDefinition {
	bool original;
public:
	GetCoordinate(Scope &aScope, TTICompPreprocessor &prep, bool aOriginal);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class SetCoordinate : public PredefinedFunctionDefinition {
public:
	SetCoordinate(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class GetDistance : public PredefinedFunctionDefinition {
	bool original;
public:
	GetDistance(Scope &aScope, TTICompPreprocessor &prep, bool aOriginal);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class FlipPoint : public PredefinedFunctionDefinition {
public:
	FlipPoint(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class FlipRange : public PredefinedFunctionDefinition {
	bool on;
public:
	FlipRange(Scope &aScope, TTICompPreprocessor &prep, bool aOn);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class ShiftPoint : public PredefinedFunctionDefinition {
public:
	ShiftPoint(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
}; 

class ShiftContour : public PredefinedFunctionDefinition {
public:
	ShiftContour(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
}; 

class ShiftZone : public PredefinedFunctionDefinition {
public:
	ShiftZone(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
}; 

class ShiftPointByPixels : public PredefinedFunctionDefinition {
public:
	ShiftPointByPixels(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
}; 


class MoveDirectAbsolutePoint : public PredefinedFunctionDefinition {
	bool round;
public:
	MoveDirectAbsolutePoint(Scope &aScope, TTICompPreprocessor &prep, bool aRound);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class MoveStackIndirectRelativePoint : public PredefinedFunctionDefinition {
public:
	MoveStackIndirectRelativePoint(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class MoveIndirectAbsolutePoint : public PredefinedFunctionDefinition {
	bool round;
public:
	MoveIndirectAbsolutePoint(Scope &aScope, TTICompPreprocessor &prep, bool aRound);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class MoveDirectRelativePoint : public PredefinedFunctionDefinition {
	bool minDist;
	bool round;
	Byte colour;
public:
	MoveDirectRelativePoint(Scope &aScope, TTICompPreprocessor &prep, bool aMinDist, bool aRound,
		Byte aColour);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class MoveIndirectRelativePoint : public PredefinedFunctionDefinition {
	bool minDist;
	bool round;
	Byte colour;
public:
	MoveIndirectRelativePoint(Scope &aScope, TTICompPreprocessor &prep, bool aMinDist, bool aRound,
		Byte aColour);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class AlignRelativePoint : public PredefinedFunctionDefinition {
public:
	AlignRelativePoint(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class Intersection : public PredefinedFunctionDefinition {
public:
	Intersection(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class AlignPoints : public PredefinedFunctionDefinition {
public:
	AlignPoints(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class InterpolatePoint : public PredefinedFunctionDefinition {
public:
	InterpolatePoint(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class UnTouchPoint : public PredefinedFunctionDefinition {
public:
	UnTouchPoint(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class InterpolateUntouchedPoints : public PredefinedFunctionDefinition {
	bool x;
public:
	InterpolateUntouchedPoints(Scope &aScope, TTICompPreprocessor &prep, bool aX);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class DeltaP : public PredefinedFunctionDefinition {
	Byte type;
public:
	DeltaP(Scope &aScope, TTICompPreprocessor &prep, Byte aType);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class DeltaC : public PredefinedFunctionDefinition {
	Byte type;
public:
	DeltaC(Scope &aScope, TTICompPreprocessor &prep, Byte aType);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

// Numerical methods

class Odd : public PredefinedFunctionDefinition {
public:
	Odd(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class Even : public PredefinedFunctionDefinition {
public:
	Even(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class Absolute : public PredefinedFunctionDefinition {
	bool forFixed;
public:
	Absolute(Scope &aScope, TTICompPreprocessor &prep, bool aForFixed);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class Floor : public PredefinedFunctionDefinition {
	bool forFixed;
public:
	Floor(Scope &aScope, TTICompPreprocessor &prep, bool aForFixed);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class Ceiling : public PredefinedFunctionDefinition {
	bool forFixed;
public:
	Ceiling(Scope &aScope, TTICompPreprocessor &prep, bool aForFixed);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class Minimum : public PredefinedFunctionDefinition {
public:
	Minimum(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class Maximum : public PredefinedFunctionDefinition {
public:
	Maximum(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

// Helper functions

class Round : public PredefinedFunctionDefinition {
	Byte colour;
public:
	Round(Scope &aScope, TTICompPreprocessor &prep, Byte aColour);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

class NoRound : public PredefinedFunctionDefinition {
	Byte colour;
public:
	NoRound(Scope &aScope, TTICompPreprocessor &prep, Byte aColour);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};


class GetInformation : public PredefinedFunctionDefinition {
public:
	GetInformation(Scope &aScope, TTICompPreprocessor &prep);
	virtual void callThis(InstructionSequence * seq, const FunctionCallParameters &parameters) const;
};

#endif // FUNCTIONSCOPE_H
