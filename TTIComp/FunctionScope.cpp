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

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <cassert>
#include "FunctionScope.h"
#include "../InstructionProcessor/Instructions.h"

FunctionScope::FunctionScope(TTICompPreprocessor &aPrep) : MainScope(aPrep) {
}

void FunctionScope::addPredefinedDefinitions() {
	TTICompPreprocessor &prep = (TTICompPreprocessor &) this->prep;
	addFunctionDef (new SetCVT(*this, prep, false));
	addFunctionDef (new SetCVT(*this, prep, true));
	addFunctionDef (new GetCVT(*this, prep));

	addFunctionDef (new SetVectorToAxis(*this, prep, true, true, false));
	addFunctionDef (new SetVectorToAxis(*this, prep, true, true, true));
	addFunctionDef (new SetVectorToAxis(*this, prep, true, false, false));
	addFunctionDef (new SetVectorToAxis(*this, prep, true, false, true));
	addFunctionDef (new SetVectorToAxis(*this, prep, false, true, false));
	addFunctionDef (new SetVectorToAxis(*this, prep, false, true, true));

	addFunctionDef (new SetVectorLine(*this, prep, false, false, false));
	addFunctionDef (new SetVectorLine(*this, prep, false, false, true));
	addFunctionDef (new SetVectorLine(*this, prep, true, false, false));
	addFunctionDef (new SetVectorLine(*this, prep, true, false, true));
	addFunctionDef (new SetVectorLine(*this, prep, true, true, false));
	addFunctionDef (new SetVectorLine(*this, prep, true, true, true));
	addFunctionDef (new SetFreedomProjection(*this, prep));
	addFunctionDef (new SetVector(*this, prep, false));
	addFunctionDef (new SetVector(*this, prep, true));

	addFunctionDef (new SetRound(*this, prep, roHalf));
	addFunctionDef (new SetRound(*this, prep, roGrid));
	addFunctionDef (new SetRound(*this, prep, roDouble));
	addFunctionDef (new SetRound(*this, prep, roDown));
	addFunctionDef (new SetRound(*this, prep, roUp));
	addFunctionDef (new SetRound(*this, prep, roOff));
	addFunctionDef (new SuperRound(*this, prep, false));
	addFunctionDef (new SuperRound(*this, prep, true));

	addFunctionDef (new SetMinimumDistance(*this, prep));
	addFunctionDef (new ScanConversionControl(*this, prep));
	addFunctionDef (new ScanType(*this, prep));
	addFunctionDef (new SetCVTCutIn(*this, prep));
	addFunctionDef (new SetSingleWidth(*this, prep));
	addFunctionDef (new SetSingleWidthCutIn(*this, prep));
	addFunctionDef (new SetAutoFlip(*this, prep, false));
	addFunctionDef (new SetAutoFlip(*this, prep, true));
	addFunctionDef (new SetDeltaBase(*this, prep));
	addFunctionDef (new SetDeltaShift(*this, prep));

	addFunctionDef (new GetPPEM(*this, prep));
	addFunctionDef (new GetPointSize(*this, prep));

	addFunctionDef (new GetCoordinate(*this, prep, false));
	addFunctionDef (new GetCoordinate(*this, prep, true));
	addFunctionDef (new SetCoordinate(*this, prep));
	addFunctionDef (new GetDistance(*this, prep, false));
	addFunctionDef (new GetDistance(*this, prep, true));
	addFunctionDef (new FlipPoint(*this, prep));
	addFunctionDef (new FlipRange(*this, prep, false));
	addFunctionDef (new FlipRange(*this, prep, true));
	addFunctionDef (new ShiftPoint(*this, prep));
	addFunctionDef (new ShiftContour(*this, prep));
	addFunctionDef (new ShiftZone(*this, prep));
	addFunctionDef (new ShiftPointByPixels(*this, prep));

	addFunctionDef (new MoveDirectAbsolutePoint(*this, prep, false));
	addFunctionDef (new MoveDirectAbsolutePoint(*this, prep, true));
	addFunctionDef (new MoveStackIndirectRelativePoint(*this, prep));
	addFunctionDef (new MoveIndirectAbsolutePoint(*this, prep, false));
	addFunctionDef (new MoveIndirectAbsolutePoint(*this, prep, true));

	addFunctionDef (new MoveDirectRelativePoint(*this, prep, false, false, oiColourWhite));
	addFunctionDef (new MoveDirectRelativePoint(*this, prep, false, false, oiColourGrey));
	addFunctionDef (new MoveDirectRelativePoint(*this, prep, false, false, oiColourBlack));
	addFunctionDef (new MoveDirectRelativePoint(*this, prep, false, true, oiColourWhite));
	addFunctionDef (new MoveDirectRelativePoint(*this, prep, false, true, oiColourGrey));
	addFunctionDef (new MoveDirectRelativePoint(*this, prep, false, true, oiColourBlack));
	addFunctionDef (new MoveDirectRelativePoint(*this, prep, true, false, oiColourWhite));
	addFunctionDef (new MoveDirectRelativePoint(*this, prep, true, false, oiColourGrey));
	addFunctionDef (new MoveDirectRelativePoint(*this, prep, true, false, oiColourBlack));
	addFunctionDef (new MoveDirectRelativePoint(*this, prep, true, true, oiColourWhite));
	addFunctionDef (new MoveDirectRelativePoint(*this, prep, true, true, oiColourGrey));
	addFunctionDef (new MoveDirectRelativePoint(*this, prep, true, true, oiColourBlack));

	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, false, false, oiColourWhite));
	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, false, false, oiColourGrey));
	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, false, false, oiColourBlack));
	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, false, true, oiColourWhite));
	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, false, true, oiColourGrey));
	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, false, true, oiColourBlack));
	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, true, false, oiColourWhite));
	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, true, false, oiColourGrey));
	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, true, false, oiColourBlack));
	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, true, true, oiColourWhite));
	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, true, true, oiColourGrey));
	addFunctionDef (new MoveIndirectRelativePoint(*this, prep, true, true, oiColourBlack));

	addFunctionDef (new AlignRelativePoint(*this, prep));
	addFunctionDef (new Intersection(*this, prep));
	addFunctionDef (new AlignPoints(*this, prep));
	addFunctionDef (new InterpolatePoint(*this, prep));
	addFunctionDef (new UnTouchPoint(*this, prep));
	addFunctionDef (new InterpolateUntouchedPoints(*this, prep, false));
	addFunctionDef (new InterpolateUntouchedPoints(*this, prep, true));
	addFunctionDef (new DeltaP(*this, prep, 1));
	addFunctionDef (new DeltaP(*this, prep, 2));
	addFunctionDef (new DeltaP(*this, prep, 3));
	addFunctionDef (new DeltaC(*this, prep, 1));
	addFunctionDef (new DeltaC(*this, prep, 2));
	addFunctionDef (new DeltaC(*this, prep, 3));

	addFunctionDef (new Odd(*this, prep));
	addFunctionDef (new Even(*this, prep));
	addFunctionDef (new Absolute(*this, prep, false));
	addFunctionDef (new Absolute(*this, prep, true));
	addFunctionDef (new Floor(*this, prep, false));
	addFunctionDef (new Floor(*this, prep, true));
	addFunctionDef (new Ceiling(*this, prep, false));
	addFunctionDef (new Ceiling(*this, prep, true));
	addFunctionDef (new Minimum(*this, prep));
	addFunctionDef (new Maximum(*this, prep));

	addFunctionDef (new Round(*this, prep, oiColourWhite));
	addFunctionDef (new Round(*this, prep, oiColourGrey));
	addFunctionDef (new Round(*this, prep, oiColourBlack));
	addFunctionDef (new NoRound(*this, prep, oiColourWhite));
	addFunctionDef (new NoRound(*this, prep, oiColourGrey));
	addFunctionDef (new NoRound(*this, prep, oiColourBlack));
	addFunctionDef (new GetInformation(*this, prep));
}

/*** PredefinedFunctionDefinition ***/

PredefinedFunctionDefinition::PredefinedFunctionDefinition(Scope &aScope, TTICompPreprocessor &prep, Type aType,
														   String aName, const FormalParameters &aFormalParameters) : 
FunctionDefinitionStatement(aScope, prep.getInternalPosition(), aType, aName, aFormalParameters) {
//	aScope->addFunctionDef(this);
}

void PredefinedFunctionDefinition::writeToStream(ostream &o) const {
	// Uncomment this to get a header
	FunctionDefinitionStatement::writeToStream(o);
	o << ';';
}


void PredefinedFunctionDefinition::setCalled(bool aInGlyphProgram) {}

/*** Predefined Functions ***/

FormalParameters getOneParameter(Type type1) {
	/*
	FormalParameter *par = new FormalParameter;
	par->type = type1;
	par->next = NULL;*/
	FormalParameters params;
	FormalParameter p;
	p.type = type1;
	params.push_back (p);
	return params;
}

FormalParameters getTwoParameters(Type type1, Type type2) {
	FormalParameters params;
	FormalParameter p;
	p.type = type1;
	params.push_back (p);
	p.type = type2;
	params.push_back (p);
	return params;
}

FormalParameters getThreeParameters(Type type1, Type type2, Type type3) {
	FormalParameters params;
	FormalParameter p;
	p.type = type1;
	params.push_back (p);
	p.type = type2;
	params.push_back (p);
	p.type = type3;
	params.push_back (p);
	return params;
}

FormalParameters getFourParameters(Type type1, Type type2, Type type3, Type type4) {
	FormalParameters params;
	FormalParameter p;
	p.type = type1;
	params.push_back (p);
	p.type = type2;
	params.push_back (p);
	p.type = type3;
	params.push_back (p);
	p.type = type4;
	params.push_back (p);
	return params;
}

FormalParameters getFiveParameters(Type type1, Type type2, Type type3, Type type4, Type type5) {
	FormalParameters params;
	FormalParameter p;
	p.type = type1;
	params.push_back (p);
	p.type = type2;
	params.push_back (p);
	p.type = type3;
	params.push_back (p);
	p.type = type4;
	params.push_back (p);
	p.type = type5;
	params.push_back (p);
	return params;
}

FormalParameters getSixParameters(Type type1, Type type2, Type type3, Type type4, Type type5, Type type6) {
	FormalParameters params;
	FormalParameter p;
	p.type = type1;
	params.push_back (p);
	p.type = type2;
	params.push_back (p);
	p.type = type3;
	params.push_back (p);
	p.type = type4;
	params.push_back (p);
	p.type = type5;
	params.push_back (p);
	p.type = type6;
	params.push_back (p);
	return params;
}

FormalParameters getEightParameters(Type type1, Type type2, Type type3, Type type4,
									Type type5, Type type6, Type type7, Type type8) {
	FormalParameters params;
	FormalParameter p;
	p.type = type1;
	params.push_back (p);
	p.type = type2;
	params.push_back (p);
	p.type = type3;
	params.push_back (p);
	p.type = type4;
	params.push_back (p);
	p.type = type5;
	params.push_back (p);
	p.type = type6;
	params.push_back (p);
	p.type = type7;
	params.push_back (p);
	p.type = type8;
	params.push_back (p);
	return params;
}


// setCVT

SetCVT::SetCVT(Scope &aScope, TTICompPreprocessor &prep, bool aPixels) : 
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setCVT",
							 getTwoParameters(tyUint, aPixels ? tyFixed : tyInt)) {
	pixels = aPixels;
}

void SetCVT::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 2);
	params[0]->addToInstructionSequence(seq, true);
	params[1]->addToInstructionSequence(seq, true);
	if (pixels)
		seq->addInstruction(new WriteCVTPixelsInstruction());
	else
		seq->addInstruction(new WriteCVTFUnitsInstruction());
	seq->notifyStackChange(2);
}

// getCVT

GetCVT::GetCVT(Scope &aScope, TTICompPreprocessor &prep) : 
PredefinedFunctionDefinition(aScope, prep, tyFixed, "getCVT",
							 getOneParameter(tyUint)) {}

void GetCVT::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new ReadCVTInstruction());
	seq->notifyStackChange(1, 1);
}

// set..Vector

const char *getVectorName(bool projection, bool freedom, bool dual, bool perp) {
	//char *s = new char[25];
	static char s[25];
	strcpy(s, "set");
	if (projection) {
		if (freedom)
			strcat(s, "Vectors");
		else {
			if (dual)
				strcat(s, "DualProjection");
			else
				strcat(s, "Projection");
		}
	} else {
		assert(freedom);
		strcat(s, "Freedom");
	}
	if (perp)
		strcat(s, "Perp");
	return s;
}

const char *getVectorToAxisName(bool projection, bool freedom, bool x) {
	const char *s = getVectorName(projection, freedom, false, false);
	if (x)
		strcat((char*)s, "X");
	else
		strcat((char*)s, "Y");
	return s;
}

SetVectorToAxis::SetVectorToAxis(Scope &aScope, TTICompPreprocessor &prep, bool aProjection, bool aFreedom, bool aX) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, getVectorToAxisName(aProjection, aFreedom, aX), FormalParameters()) {
	assert(aProjection||aFreedom);
	projection = aProjection;
	freedom = aFreedom;
	x = aX;
}

void SetVectorToAxis::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.empty());
	if (projection) {
		if (freedom)
			seq->addInstruction(new VectorsToAxisInstruction(x));
		else
			seq->addInstruction(new ProjectionToAxisInstruction(x));
	} else {
		assert(freedom);
		seq->addInstruction(new FreedomToAxisInstruction(x));
	}
}


SetVectorLine::SetVectorLine(Scope &aScope, TTICompPreprocessor &prep, bool aProjection, bool aDual, bool aPerp) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, getVectorName(aProjection, !aProjection, aDual, aPerp),
							 getFourParameters(tyUint, tyUint, tyUint, tyUint)) {
	assert(!aDual || aProjection);
	projection = aProjection;
	dual = aDual;
	perp = aPerp;
}

void SetVectorLine::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 4);
	// p2 (zp1)
	params[3]->addToInstructionSequence(seq, true);
	// p1 (zp2)
	params[1]->addToInstructionSequence(seq, true);

	seq->setPointers(NULL, params[2], params[0]);

	if (projection) {
		if (dual)
			seq->addInstruction(new DualProjectionToLineInstruction(perp));
		else
			seq->addInstruction(new ProjectionToLineInstruction(perp));
	} else
		seq->addInstruction(new FreedomToLineInstruction(perp));
	seq->notifyStackChange(2);
}

SetFreedomProjection::SetFreedomProjection(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setFreedomProjection", FormalParameters()) {}

void SetFreedomProjection::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.empty());
	seq->addInstruction(new FreedomToProjectionInstruction());
}

SetVector::SetVector(Scope &aScope, TTICompPreprocessor &prep, bool aProjection) : 
PredefinedFunctionDefinition(aScope, prep, tyVoid, getVectorName(aProjection, !aProjection, false, false),
							 getTwoParameters(tyFixed, tyFixed)) {
	projection = aProjection;
}

void SetVector::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 2);

	params[0]->addToInstructionSequence(seq, true);
	params[1]->addToInstructionSequence(seq, true);

	if (projection)
		seq->addInstruction(new ProjectionFromStackInstruction());
	else
		seq->addInstruction(new FreedomFromStackInstruction());
	seq->notifyStackChange(2);
}

// setRound

const char *getRoundName(RoundType aType) {
	//char *s = new char[20];
	static char s[20];
	strcpy(s, "setRound");
	switch (aType) {
	case roHalf:
		strcat(s, "Half");
		break;
	case roGrid:
		strcat(s, "Grid");
		break;
	case roDouble:
		strcat(s, "Double");
		break;
	case roDown:
		strcat(s, "Down");
		break;
	case roUp:
		strcat(s, "Up");
		break;
	case roOff:
		strcat(s, "Off");
		break;
	case roSuper:
		break;
	case ro45:
		strcat(s, "45");
		break;
	default:
		assert(false);
	}
	return s;
}

SetRound::SetRound(Scope &aScope, TTICompPreprocessor &prep, RoundType aType) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, getRoundName(aType), FormalParameters()) {
	roundType = aType;
}

void SetRound::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.empty());
	switch (roundType) {
	case roHalf:
		seq->addInstruction(new RoundToHalfGridInstruction());
		break;
	case roGrid:
		seq->addInstruction(new RoundToGridInstruction());
		break;
	case roDouble:
		seq->addInstruction(new RoundToDoubleGridInstruction());
		break;
	case roDown:
		seq->addInstruction(new RoundDownToGridInstruction());
		break;
	case roUp:
		seq->addInstruction(new RoundUpToGridInstruction());
		break;
	default:
		assert(false);
	}
}

SuperRound::SuperRound(Scope &aScope, TTICompPreprocessor &prep, bool a45) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, getRoundName(a45 ? ro45 : roSuper),
							 getOneParameter(tyUint)) {
	round45 = a45;
}

void SuperRound::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	if (round45)
		seq->addInstruction(new SuperRound45Instruction());
	else
		seq->addInstruction(new SuperRoundInstruction());
	seq->notifyStackChange(1);
}


// Other Graphics State functions

SetMinimumDistance::SetMinimumDistance(Scope &aScope, TTICompPreprocessor &prep) : 
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setMinDist", getOneParameter(tyFixed)) {}

void SetMinimumDistance::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new SetMinimumDistanceInstruction());
	seq->notifyStackChange(1);
}

InstructionExecutionControl::InstructionExecutionControl(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setInstructionExecution",
							 getTwoParameters(tyUint, tyUint)) {}

void InstructionExecutionControl::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 2);
	params[0]->addToInstructionSequence(seq, true);
	params[1]->addToInstructionSequence(seq, true);
	seq->addInstruction(new InstructionControlInstruction());
	seq->notifyStackChange(2);
}

ScanConversionControl::ScanConversionControl(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setScanConversion", getOneParameter(tyUint)) {}

void ScanConversionControl::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new ScanConversionControlInstruction());
	seq->notifyStackChange(1);
}

ScanType::ScanType(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setScanType", getOneParameter(tyUint)) {}

void ScanType::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new ScanTypeInstruction());
	seq->notifyStackChange(1);
}

SetCVTCutIn::SetCVTCutIn(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setCVTCutIn", getOneParameter(tyFixed)) {}

void SetCVTCutIn::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new ControlValueCutInInstruction());
	seq->notifyStackChange(1);
}

SetSingleWidth::SetSingleWidth(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setSingleWidth", getOneParameter(tyFixed)) {}

void SetSingleWidth::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new SetSingleWidthInstruction());
	seq->notifyStackChange(1);
}

SetSingleWidthCutIn::SetSingleWidthCutIn(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setSingleWidthCutIn", getOneParameter(tyFixed)) {}

void SetSingleWidthCutIn::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new SetSingleWidthCutInInstruction());
	seq->notifyStackChange(1);
}

SetAutoFlip::SetAutoFlip(Scope &aScope, TTICompPreprocessor &prep, bool aOn) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, aOn ? "setAutoFlipOn" : "setAutoFlipOff", FormalParameters()) {
	on = aOn;
}

void SetAutoFlip::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.empty());
	seq->addInstruction(new SetAutoFlipInstruction(on));
}

SetDeltaBase::SetDeltaBase(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setDeltaBase", getOneParameter(tyUint)) {}

void SetDeltaBase::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new DeltaBaseInstruction());
	seq->notifyStackChange(1);
}

SetDeltaShift::SetDeltaShift(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setDeltaShift", getOneParameter(tyUint)) {}

void SetDeltaShift::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new DeltaShiftInstruction());
	seq->notifyStackChange(1);
}


// Size information

GetPPEM::GetPPEM(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyUint, "getPPEM", FormalParameters()) {}

void GetPPEM::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.empty());
	seq->addInstruction(new MeasurePPEMInstruction());
	seq->notifyStackChange(0,1);
}

GetPointSize::GetPointSize(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyUint, "getPointSize", FormalParameters()) {}

void GetPointSize::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.empty());
	seq->addInstruction(new MeasurePointSizeInstruction());
	seq->notifyStackChange(0,1);
}





/*** Manipulate Point functions ***/

GetCoordinate::GetCoordinate(Scope &aScope, TTICompPreprocessor &prep, bool aOriginal) :
PredefinedFunctionDefinition(aScope, prep, tyFixed, aOriginal ? "getCoordinateOrig" : "getCoordinate", getTwoParameters(tyUint, tyUint)) {
	original = aOriginal;
}

void GetCoordinate::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 2);

	// Two parameters: the zone and the point.
	params[1]->addToInstructionSequence(seq, true);
	seq->setPointers(NULL, NULL, params[0]);

	seq->addInstruction(new GetCoordinateInstruction(original));
	seq->notifyStackChange(1, 1);
}


SetCoordinate::SetCoordinate(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "setCoordinate", getThreeParameters(tyUint, tyUint, tyFixed)) {
}

void SetCoordinate::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 3);

	// Three parameters: the zone, the point and the distance.
	params[1]->addToInstructionSequence(seq, true);
	params[2]->addToInstructionSequence(seq, true);

	// set zp2
	seq->setPointers(NULL, NULL, params[0]);

	seq->addInstruction(new SetCoordinateInstruction());
	seq->notifyStackChange(2);
}


GetDistance::GetDistance(Scope &aScope, TTICompPreprocessor &prep, bool aOriginal) :
PredefinedFunctionDefinition(aScope, prep, tyFixed, aOriginal ? "getDistanceOrig" : "getDistance",
							 getFourParameters(tyUint, tyUint, tyUint, tyUint)) {
	original = aOriginal;
}

void GetDistance::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 4);

	// Set p2 (zp0)
	params[3]->addToInstructionSequence(seq, true);
	// Set p1 (zp1)
	params[1]->addToInstructionSequence(seq, true);

	// Set zp1 and zp0
	seq->setPointers(params[2], params[0], NULL);

	seq->addInstruction(new MeasureDistanceInstruction(original));
	seq->notifyStackChange(2, 1);
}

FlipPoint::FlipPoint(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "flipPoint", getOneParameter(tyUint)) {}

void FlipPoint::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 4);
	params[0]->addToInstructionSequence(seq, true);
	seq->setZonePointer(0, 1);
	seq->addInstruction(new FlipPointInstruction());
	seq->notifyStackChange(1);
}

FlipRange::FlipRange(Scope &aScope, TTICompPreprocessor &prep, bool aOn) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, aOn ? "flipRangeOn" : "FlipRangeOff",
							 getTwoParameters(tyUint, tyUint)) {
	on = aOn;
}

void FlipRange::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 2);
	params[0]->addToInstructionSequence(seq, true);
	params[1]->addToInstructionSequence(seq, true);
	seq->addInstruction(new FlipRangeInstruction());
	seq->notifyStackChange(2);
}

ShiftPoint::ShiftPoint(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "shiftPoint", getFourParameters(tyUint, tyUint, tyUint, tyUint)) {}

void ShiftPoint::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 4);

	// zp0/rp1, zp2/p

	// p
	params[3]->addToInstructionSequence(seq, true);

	// set rp1 and zone pointers
	seq->setPointers(params[0], NULL, params[2],
		NULL, params[1]);

	seq->addInstruction(new ShiftPointInstruction(true));
	seq->notifyStackChange(1);
}

ShiftContour::ShiftContour(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "shiftContour",
							 getThreeParameters(tyUint, tyUint, tyUint)) {}

void ShiftContour::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 3);
	
	params[2]->addToInstructionSequence(seq, true);
	seq->setPointers(NULL, params[0], NULL,
		NULL, NULL, params[1]);
	seq->addInstruction(new ShiftContourInstruction(false));
	seq->notifyStackChange(1);
}

ShiftZone::ShiftZone(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "shiftZone",
							 getThreeParameters(tyUint, tyUint, tyUint)) {}

void ShiftZone::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 3);
	
	params[2]->addToInstructionSequence(seq, true);
	seq->setPointers(NULL, params[0], NULL,
		NULL, NULL, params[1]);
	seq->addInstruction(new ShiftZoneInstruction(false));
	seq->notifyStackChange(1);
}

ShiftPointByPixels::ShiftPointByPixels(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "shiftPoint",
							 getThreeParameters(tyUint, tyUint, tyFixed)) {}

void ShiftPointByPixels::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 3);
	params[1]->addToInstructionSequence(seq, true);
	params[2]->addToInstructionSequence(seq, true);
	seq->setPointers(NULL, NULL, params[0]);
	seq->addInstruction(new ShiftPointByPixelsInstruction());
	seq->notifyStackChange(2);
}





/*** Manipulate points ***/


// roundPoint / touch

MoveDirectAbsolutePoint::MoveDirectAbsolutePoint(Scope &aScope, TTICompPreprocessor &prep, bool aRound) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, aRound ? "roundPoint" : "touch",
							getTwoParameters(tyUint, tyUint)) {
	round = aRound;
}


void MoveDirectAbsolutePoint::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	// Two parameters: the zone (zp0) and the point.
	assert(params.size() == 2);

	params[1]->addToInstructionSequence(seq, true);
	seq->setPointers(params[0]);
	seq->addInstruction(new MoveDirectAbsolutePointInstruction(round));
	// sets rp0 = rp1 = point p
	seq->notifyPointers(params[1], params[1]);
	seq->notifyStackChange(1);
}


MoveIndirectAbsolutePoint::MoveIndirectAbsolutePoint(Scope &aScope, TTICompPreprocessor &prep, bool aRound) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, aRound ? "movePointRound" : "movePoint",
							 getThreeParameters(tyUint, tyUint, tyUint)) {
	round = aRound;
}

void MoveIndirectAbsolutePoint::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size()==3);
	//set point: zp0/p
	params[1]->addToInstructionSequence(seq, true);
	// set CVT index
	params[2]->addToInstructionSequence(seq, true);

	seq->setPointers(params[0], NULL, NULL);
	seq->addInstruction(new MoveIndirectAbsolutePointInstruction(round));
	// sets rp0 = rp1 = point p
	seq->notifyPointers(params[1], params[1]);
	seq->notifyStackChange(2);
}


// moveDistance(zp, rp, zp, p, distance)

MoveStackIndirectRelativePoint::MoveStackIndirectRelativePoint(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "moveDistance",
							 getFiveParameters(tyUint, tyUint, tyUint, tyUint, tyFixed)) {}

void MoveStackIndirectRelativePoint::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size()==5);
	// zp1 with point p and zp0 with rp0

	params[3]->addToInstructionSequence(seq, true);
	// push distance
	params[4]->addToInstructionSequence(seq, true);

	seq->setPointers(params[0], params[2], NULL,
		params[1]);

	seq->addInstruction(new MoveStackIndirectRelativePointInstruction(false));
	// set rp1 = rp0, rp2 = point p, and if a=1, rp0 is set to point p
	seq->notifyStackChange(2);
	seq->notifyPointers(NULL, params[1], params[3]);
	seq->notifyAutomaticRef0(params[3], 0x1);
}

// moveDistance....

const char *getMoveDistanceName(const char *stem, bool minDist, bool round, Byte colour) {
	//char *s = new char[18+strlen(stem)];
	static char s[40];
	strcpy(s, stem);
	if (minDist) strcat(s, "MinDist");
	if (round) strcat(s, "Round");
	switch (colour) {
	case oiColourWhite:
		strcat(s, "White");
		break;
	case oiColourGrey:
		strcat(s, "Grey");
		break;
	case oiColourBlack:
		strcat(s, "Black");
		break;
	default:
		assert(false);
	}
	return s;
}

MoveDirectRelativePoint::MoveDirectRelativePoint(Scope &aScope, TTICompPreprocessor &prep, bool aMinDist,
												 bool aRound, Byte aColour) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, getMoveDistanceName("moveDistance", aMinDist, aRound, aColour),
							getFourParameters(tyUint, tyUint, tyUint, tyUint)) {
	minDist = aMinDist;
	round = aRound;
	colour = aColour;
}

void MoveDirectRelativePoint::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 4);
	// zp0 with rp0 and zp1 with point p

	// p
	params[3]->addToInstructionSequence(seq, true);

	seq->setPointers(params[0], params[2], NULL,
		params[1]);
	seq->addInstruction(new MoveDirectRelativePointInstruction(false, minDist, round, colour));
	seq->notifyStackChange(1);
	// sets rp1 = rp0, rp2 = point p; if the a flag is set to TRUE, rp0 is set equal to point p
	seq->notifyPointers(NULL, params[1], params[3]);
	seq->notifyAutomaticRef0(params[3], 0x10);
}


MoveIndirectRelativePoint::MoveIndirectRelativePoint(Scope &aScope, TTICompPreprocessor &prep,
													 bool aMinDist, bool aRound, Byte aColour) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, getMoveDistanceName("moveDistance", aMinDist, aRound, aColour),
							getFiveParameters(tyUint, tyUint, tyUint, tyUint, tyUint)) {
	minDist = aMinDist;
	round = aRound;
	colour = aColour;
}

void MoveIndirectRelativePoint::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size()==5);
	// zp0 with rp0 and zp1 with point p

	params[3]->addToInstructionSequence(seq, true);
	// Set CVT index
	params[4]->addToInstructionSequence(seq, true);

	seq->setPointers(params[0], params[2], NULL,
		params[1]);

	seq->addInstruction(new MoveIndirectRelativePointInstruction(false, minDist, round, colour));
	seq->notifyStackChange(2);
	// sets rp1 = rp0, rp2 = point p, and if a = 1, rp0 is set to point p
	seq->notifyPointers(NULL, params[1], params[3]);
	seq->notifyAutomaticRef0(params[3], 0x10);
}


// align

AlignRelativePoint::AlignRelativePoint(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "align", getFourParameters(tyUint, tyUint, tyUint, tyUint)) {}

void AlignRelativePoint::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 4);

	// zp1 with point p, zp0 with rp0

	// p
	params[3]->addToInstructionSequence(seq, true);
	seq->setPointers(params[0], params[2], NULL,
		params[1]);
	seq->addInstruction(new AlignInstruction());
	seq->notifyStackChange(1);
}

// intersection

Intersection::Intersection(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "intersection", getEightParameters(tyUint, tyUint, tyUint,
							 tyUint, tyUint, tyUint, tyUint, tyUint)) {}

void Intersection::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 8);

	// p
	params[7]->addToInstructionSequence(seq, true);

	// a0 and a1
	params[1]->addToInstructionSequence(seq, true);
	params[2]->addToInstructionSequence(seq, true);
	// b0 and b1
	params[4]->addToInstructionSequence(seq, true);
	params[5]->addToInstructionSequence(seq, true);

	// zp0 = B, zp1 = A, zp2 = p
	seq->setPointers(params[3], params[0],
		params[6]);

	seq->addInstruction(new MoveToIntersectionInstruction());
	seq->notifyStackChange(5);
}

// align (two points)

AlignPoints::AlignPoints(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "alignPoints",
							 getFourParameters(tyUint, tyUint, tyUint, tyUint)) {}

void AlignPoints::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 4);

	// p1 and p2
	params[1]->addToInstructionSequence(seq, true);
	params[3]->addToInstructionSequence(seq, true);

	// zp1 and zp0
	seq->setPointers(params[2], params[0], NULL);

	seq->addInstruction(new AlignPointsInstruction());
	seq->notifyStackChange(2);
}

// interpolate

InterpolatePoint::InterpolatePoint(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "interpolate",
							 getSixParameters(tyUint, tyUint, tyUint, tyUint, tyUint, tyUint)) {}

void InterpolatePoint::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 6);
	// zp0 with rp1, zp1 with rp2, zp2 with point p

	params[5]->addToInstructionSequence(seq, true);

	seq->setPointers(params[0], params[2], params[4],
		NULL, params[1], params[3]);

	seq->addInstruction(new InterpolatePointInstruction());
	seq->notifyStackChange(1);
}

// untouch

UnTouchPoint::UnTouchPoint(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, "untouch",
							 getTwoParameters(tyUint, tyUint)) {}

void UnTouchPoint::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 2);

	// Set point zp0/p
	params[1]->addToInstructionSequence(seq, true);

	seq->setPointers(params[0], NULL, NULL);
	seq->addInstruction(new UntouchPointInstruction());
	seq->notifyStackChange(1);
}


// interpolateX/Y

InterpolateUntouchedPoints::InterpolateUntouchedPoints(Scope &aScope, TTICompPreprocessor &prep, bool aX) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, aX ? "interpolateX" : "interpolateY", FormalParameters()) {
	x = aX;
}

void InterpolateUntouchedPoints::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.empty());
	seq->setZonePointer(2, 1);
	seq->addInstruction(new InterpolateUntouchedPointsInstruction(x));
}

// DeltaP/C

const char *getDeltaName(char pc, Byte type) {
	//char *s = new char[8];
	static char s[8];
	strcpy(s, "delta");
	s[5] = pc;
	s[6] = '0' + type;
	s[7] = '\0';
	return s;
}

DeltaP::DeltaP(Scope &aScope, TTICompPreprocessor &prep, Byte aType) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, getDeltaName('P', aType), getTwoParameters(tyUint, tyUint)) {
	type = aType;
}

void DeltaP::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 2);
	// Get args
	params[1]->addToInstructionSequence(seq, true);
	// Get point
	params[0]->addToInstructionSequence(seq, true);
	// One pair
	seq->push(1);
	seq->addInstruction(new DeltaPInstruction(type));
	seq->notifyStackChange(3);
}

DeltaC::DeltaC(Scope &aScope, TTICompPreprocessor &prep, Byte aType) :
PredefinedFunctionDefinition(aScope, prep, tyVoid, getDeltaName('C', aType), getTwoParameters(tyUint, tyUint)) {
	type = aType;
}

void DeltaC::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 2);
	// Get args
	params[1]->addToInstructionSequence(seq, true);
	// Get point
	params[0]->addToInstructionSequence(seq, true);
	// One pair
	seq->push(1);
	seq->addInstruction(new DeltaCInstruction(type));
	seq->notifyStackChange(3);
}

// Numericals

Odd::Odd(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyBool, "odd", getOneParameter(tyFixed)) {}

void Odd::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new OddInstruction());
	seq->notifyStackChange(1,1);
}

Even::Even(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyBool, "even", getOneParameter(tyFixed)) {}

void Even::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new EvenInstruction());
	seq->notifyStackChange(1,1);
}

Absolute::Absolute(Scope &aScope, TTICompPreprocessor &prep, bool aForFixed) :
PredefinedFunctionDefinition(aScope, prep, aForFixed ? tyFixed : tyInt, "abs",
							 getOneParameter(aForFixed ? tyFixed : tyInt)) {}

void Absolute::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new AbsoluteInstruction());
	seq->notifyStackChange(1,1);
}

Floor::Floor(Scope &aScope, TTICompPreprocessor &prep, bool aForFixed) :
PredefinedFunctionDefinition(aScope, prep, aForFixed ? tyFixed : tyInt, "floor",
							 getOneParameter(aForFixed ? tyFixed : tyInt)) {}

void Floor::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new FloorInstruction());
	seq->notifyStackChange(1,1);
}

Ceiling::Ceiling(Scope &aScope, TTICompPreprocessor &prep, bool aForFixed) :
PredefinedFunctionDefinition(aScope, prep, aForFixed ? tyFixed : tyInt, "ceiling",
							 getOneParameter(aForFixed ? tyFixed : tyInt)) {}

void Ceiling::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new CeilingInstruction());
	seq->notifyStackChange(1,1);
}

Minimum::Minimum(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyUint, "min", getTwoParameters(tyUint, tyUint)) {}

void Minimum::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 2);
	params[0]->addToInstructionSequence(seq, true);
	params[1]->addToInstructionSequence(seq, true);
	seq->addInstruction(new MinimumInstruction());
	seq->notifyStackChange(2,1);
}

Maximum::Maximum(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyUint, "max", getTwoParameters(tyUint, tyUint)) {}

void Maximum::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 2);
	params[0]->addToInstructionSequence(seq, true);
	params[1]->addToInstructionSequence(seq, true);
	seq->addInstruction(new MaximumInstruction());
	seq->notifyStackChange(2,1);
}

// Engine characterics

Round::Round(Scope &aScope, TTICompPreprocessor &prep, Byte aColour) :
PredefinedFunctionDefinition(aScope, prep, tyFixed, getMoveDistanceName("round", false, false, aColour),
							 getOneParameter(tyFixed)) {
	colour = aColour;
}

void Round::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new RoundInstruction(colour));
	seq->notifyStackChange(1,1);
}

NoRound::NoRound(Scope &aScope, TTICompPreprocessor &prep, Byte aColour) :
PredefinedFunctionDefinition(aScope, prep, tyFixed, getMoveDistanceName("compensate", false, false, aColour),
							 getOneParameter(tyFixed)) {
	colour = aColour;
}

void NoRound::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new NoRoundInstruction(colour));
	seq->notifyStackChange(1,1);
}

// Information

GetInformation::GetInformation(Scope &aScope, TTICompPreprocessor &prep) :
PredefinedFunctionDefinition(aScope, prep, tyUint, "getInformation", getOneParameter(tyUint)) {}

void GetInformation::callThis(InstructionSequence *seq, const FunctionCallParameters &params) const {
	assert(params.size() == 1);
	params[0]->addToInstructionSequence(seq, true);
	seq->addInstruction(new GetInformationInstruction());
	seq->notifyStackChange(1,1);
}
