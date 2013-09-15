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
	\file Instructions.cpp contains code for handling various TrueType instructions.
*/

#include <cassert>

#include "../OTFont/OTMemoryBlock.h"

#include "Instructions.h"

using util::String;
using util::fixed_fraction;

namespace OpenType {


/******************************************************************************
	Notes to the implementation of the TrueType instructions as presented here.
	This contains anything not (directly) understandable from the TrueType
	specs. All these assumptions may be wrong and point to bugs.

  - Presumably the dualProjectionVector is set to projectionVector whenever
	that one is set. Only the SDPVTL instruction will set projectionVector
	and dualProjectionVector differently.

  - It is unclear which state variables are saved after the execution of
	the CVT program and which aren't. See InstructionProcessor::
	resetDefaultGraphicsState().

  - The way the CVT table is represented in memory is unclear. From the specs
	it appears that they should be in pixel units, but then they should be
	scaled according to the direction of the current projection vector. For
	now, they are stored with their original PPEM value (2048 initially, and
	if set with setCVTValueFUnits, ppem value weighted according to projection
	vector if set with setCVTValuePixels).

  - In the FreeType implementation, there are some things marked "UNDOCUMENTED"
	that I have not found so far. E.g. does SHP touch points and SHC and SHZ
	don't? Plus all alternative behaviour of Move instructions in the twilight
	zone...
	Update: I did some tests and came up with some fixes.
	All are marked with UNDOCUMENTED.

  - User-defined instructions are not (yet) implemented.

******************************************************************************/


// Helper function for instructions that have [x] or [y] as "parameter".

String getParameterXY(bool x) {
	if (x)
		return "[x]";
	else
		return "[y]";
}

String getParameterPerpendicular(bool perp) {
	if (perp)
		return "[perpendicular]";
	else
		return String();
}

String getParameterCurOrig(bool cur) {
	if (cur)
		return "[current]";
	else
		return "[original]";
}

String getParameter210(bool is21) {
	if (is21)
		return "[RP2 ZP1]";
	else
		return "[RP1 ZP0]";
}

String getParameterRound(bool round) {
	if (round)
		return "[round]";
	else
		return "[don't round]";
}

String getParameterMRP(Byte instruction) {
	String param = "[";

	if (!(instruction & oiMRPminDist))
		param += "don't ";
	param += "use min distance, ";

	if (!(instruction & oiMRPround))
		param += "don't ";
	param += "round, ";

	if ((instruction & oiMRPcolour)==oiColourGrey)
		param += "grey, ";
	else {
		if ((instruction & oiMRPcolour)==oiColourBlack)
			param += "black, ";
		else
			param += "white, ";
	}

	if (!(instruction & oiMRPsetrp0))
		param += "don't ";
	param += "set RP0]";
	return param;
}

String getParameterColour(Byte colour) {
	switch (colour) {
	case oiColourGrey:
		return "[grey]";
	case oiColourBlack:
		return "[black]";
	case oiColourWhite:
		return "[white]";
	default:
		return "ERROR";
	}
}

/*** PushInstruction: for all 4 kinds of PUSH instructions. Pushes values to the stack.  ***/

PushInstruction::PushInstruction (MemoryPen &pen)
	: Instruction(pen) {
	UShort elementNum;
	if (instruction==oiNPUSHB || instruction==oiNPUSHW)
		// Get number of elements from instruction stream
		elementNum = pen.readByte();
	else
		elementNum = (instruction & 0x07) +1;
	elements.reserve (elementNum);
	if (instruction==oiNPUSHB || (instruction >= oiPUSHB && instruction <= oiPUSHBend)) {
		for (int i=0; i<elementNum; i++)
			elements.push_back (pen.readByte());
	} else {
		for (int i=0; i<elementNum; i++)
			elements.push_back (pen.readShort());
	}
}

PushInstruction::~PushInstruction() {}

void PushInstruction::execute (InstructionProcessor &proc) const {
	proc.push(elements);
}

String PushInstruction::getName() const {
	String name = "Push ";
	Elements::const_iterator i;
	for (i = elements.begin(); i != elements.end(); i++) {
		name += String (*i) + ' ';
	}
	return name;
}


/*** The RS instruction ***/

void ReadStoreInstruction::execute (InstructionProcessor &proc) const {
	proc.push(proc.getStorage(proc.pop()));
}

String ReadStoreInstruction::getName() const {
	return "Read Store";
}

/*** The WS instruction ***/

void WriteStoreInstruction::execute (InstructionProcessor &proc) const {
	Long value = proc.pop();
	Long location = proc.pop();
	proc.setStorage(location, value);
}

String WriteStoreInstruction::getName() const {
	return "Write Store";
}

/*** The WCVTP instruction ***/

void WriteCVTPixelsInstruction::execute (InstructionProcessor &proc) const {
	NewF26Dot6 value = proc.popFixed();
	Long location = proc.pop();
	proc.setCVTValuePixels(location, value);
}

String WriteCVTPixelsInstruction::getName() const {
	return "Write CVT in Pixels";
}

/*** The WCVTF instruction ***/

void WriteCVTFUnitsInstruction::execute (InstructionProcessor &proc) const {
	Long value = proc.pop();
	Long location = proc.pop();
	proc.setCVTValueFUnits(location, value);
}

String WriteCVTFUnitsInstruction::getName() const {
	return "Write CVT Value in Font Units";
}

/*** The RCVT instruction ***/

void ReadCVTInstruction::execute (InstructionProcessor &proc) const {
	proc.push(proc.getCVTValue(proc.pop()));
}

String ReadCVTInstruction::getName() const {
	return "Read CVT Value";
}


/*** Projection and Freedom vector instructions ***/

/*** The SVTCA instruction ***/

VectorsToAxisInstruction::VectorsToAxisInstruction(bool x) : Instruction(x ? oiSVTCAx : oiSVTCAy) {}

void VectorsToAxisInstruction::execute (InstructionProcessor &proc) const {
	if (instruction == oiSVTCAy) {
		proc.setFreedomVector(0, 1);
		proc.setProjectionVector(0, 1);
	} else {
		proc.setFreedomVector(1, 0);
		proc.setProjectionVector(1, 0);
	}
}

String VectorsToAxisInstruction::getName() const {
	return "Set Vectors to Axis " + getParameterXY(instruction == oiSVTCAx);
}

/*** The SPVTCA instruction ***/

ProjectionToAxisInstruction::ProjectionToAxisInstruction(bool x) : Instruction(x ? oiSPVTCAx : oiSPVTCAy) {}

void ProjectionToAxisInstruction::execute (InstructionProcessor &proc) const {
	if (instruction == oiSPVTCAy)
		// y axis
		proc.setProjectionVector(0, 1);
	else
		proc.setProjectionVector(1, 0);
}

String ProjectionToAxisInstruction::getName() const {
	return "Set Projection Vector to Axis " + getParameterXY(instruction == oiSPVTCAx);
}

/*** The SFVTCA instruction ***/

FreedomToAxisInstruction::FreedomToAxisInstruction(bool x) : Instruction(x ? oiSFVTCAx : oiSFVTCAy) {}

void FreedomToAxisInstruction::execute (InstructionProcessor &proc) const {
	if (instruction == oiSFVTCAy)
		// y axis
		proc.setFreedomVector(0, 1);
	else
		proc.setFreedomVector(6, 0);
}

String FreedomToAxisInstruction::getName() const {
	return "Set Freedom Vector to Axis " + getParameterXY(instruction == oiSFVTCAx);
}

/*** The SPVTL instruction ***/

ProjectionToLineInstruction::ProjectionToLineInstruction(bool perp) :
Instruction(perp ? oiSPVTLperp : oiSPVTL) {}

void ProjectionToLineInstruction::execute (InstructionProcessor &proc) const {
	// zp1 and zp2 are reversed (making zp1 belong to p1 and zp2 to p2)
	Long p1 = proc.pop();
	Long p2 = proc.pop();
	Long zp1 = proc.getZonePointer(2);
	Long zp2 = proc.getZonePointer(1);

	NewF26Dot6 x, y;

	if (instruction==oiSPVTL) {
		x = proc.getPointX(zp2, p2) - proc.getPointX(zp1, p1);
		y = proc.getPointY(zp2, p2) - proc.getPointY(zp1, p1);
	} else {
		assert(instruction==oiSPVTLperp);
		x = proc.getPointY(zp1, p1) - proc.getPointY(zp2, p2);
		y = proc.getPointX(zp2, p2) - proc.getPointX(zp1, p1);
	}

	proc.setProjectionVector (x, y);
}

String ProjectionToLineInstruction::getName() const {
	return "Set Projection Vector to Line " +
		getParameterPerpendicular(instruction==oiSPVTLperp);
}

/*** The SFVTL instruction ***/

FreedomToLineInstruction::FreedomToLineInstruction(bool perp) : 
Instruction(perp? oiSFVTLperp : oiSFVTL) {}

void FreedomToLineInstruction::execute (InstructionProcessor &proc) const {
	// zp1 and zp2 are reversed (making zp1 belong to p1 and zp2 to p2)
	Long p1 = proc.pop();
	Long p2 = proc.pop();
	Long zp1 = proc.getZonePointer(2);
	Long zp2 = proc.getZonePointer(1);

	NewF26Dot6 x, y;

	if (instruction==oiSFVTL) {
		x = proc.getPointX(zp2, p2) - proc.getPointX(zp1, p1);
		y = proc.getPointY(zp2, p2) - proc.getPointY(zp1, p1);
	} else {
		assert(instruction==oiSFVTLperp);
		x = proc.getPointY(zp1, p1) - proc.getPointY(zp2, p2);
		y = proc.getPointX(zp2, p2) - proc.getPointX(zp1, p1);
	}

	proc.setFreedomVector(x, y);
}

String FreedomToLineInstruction::getName() const {
	return "Set Freedom Vector to Line " +
		getParameterPerpendicular(instruction==oiSFVTLperp);
}

/*** The SFVTPV instruction ***/

FreedomToProjectionInstruction::FreedomToProjectionInstruction() : Instruction(oiSFVTPV) {}

void FreedomToProjectionInstruction::execute (InstructionProcessor &proc) const {
	InstructionProcessor::Vector proj = proc.getProjectionVector();
	proc.setFreedomVector (proj.x, proj.y);
}

String FreedomToProjectionInstruction::getName() const {
	return "Set Freedom Vector to Projection Vector";
}

/*** The SDPVTL instruction ***/

DualProjectionToLineInstruction::DualProjectionToLineInstruction(bool perp) :
Instruction(perp ? oiSDPVTLperp : oiSDPVTL) {}

void DualProjectionToLineInstruction::execute (InstructionProcessor &proc) const {
	// zp1 and zp2 are reversed (making zp1 belong to p1 and zp2 to p2)
	Long p1 = proc.pop();
	Long p2 = proc.pop();
	Long zp1 = proc.getZonePointer(2);
	Long zp2 = proc.getZonePointer(1);

	NewF26Dot6 x, y, origX, origY;

	if (instruction==oiSDPVTL) {
		x = proc.getPointX(zp2, p2) - proc.getPointX(zp1, p1);
		y = proc.getPointY(zp2, p2) - proc.getPointY(zp1, p1);
		origX = proc.getOriginalPointX(zp2, p2) - proc.getOriginalPointX(zp1, p1);
		origY = proc.getOriginalPointY(zp2, p2) - proc.getOriginalPointY(zp1, p1);
	} else {
		assert(instruction==oiSDPVTLperp);
		x = proc.getPointY(zp1, p1) - proc.getPointY(zp2, p2);
		y = proc.getPointX(zp2, p2) - proc.getPointX(zp1, p1);
		origX = proc.getOriginalPointY(zp1, p1) - proc.getOriginalPointY(zp2, p2);
		origY = proc.getOriginalPointX(zp2, p2) - proc.getOriginalPointX(zp1, p1);
	}

	proc.setProjectionVector (x, y);
	proc.setDualProjectionVector (origX, origY);
}

String DualProjectionToLineInstruction::getName() const {
	return "Set Dual Projection Vector to Line " +
		getParameterPerpendicular(instruction == oiSDPVTLperp);
}

/*** The SPVFS instruction ***/

ProjectionFromStackInstruction::ProjectionFromStackInstruction() : Instruction(oiSPVFS) {}

void ProjectionFromStackInstruction::execute (InstructionProcessor &proc) const {
	NewF26Dot6 y, x;
	y = proc.popFixed();
	x = proc.popFixed();
	proc.setProjectionVector(x, y);
}

String ProjectionFromStackInstruction::getName() const {
	return "Set Projection Vector from Stack";
}

/*** The SFVFS instruction ***/

FreedomFromStackInstruction::FreedomFromStackInstruction() : Instruction(oiSFVFS) {}

void FreedomFromStackInstruction::execute (InstructionProcessor &proc) const {
	NewF26Dot6 y, x;
	y = proc.popFixed();
	x = proc.popFixed();
	proc.setFreedomVector(x, y);
}

String FreedomFromStackInstruction::getName() const {
	return "Set Freedom Vector from Stack";
}

/*** The GPV instruction ***/

void GetProjectionVectorInstruction::execute (InstructionProcessor &proc) const {
	InstructionProcessor::Vector proj = proc.getProjectionVector();
	proc.push (proj.x.get_i());
	proc.push (proj.y.get_i());
}

String GetProjectionVectorInstruction::getName() const {
	return "Get Projection Vector";
}

/*** The GFV instruction ***/

void GetFreedomVectorInstruction::execute (InstructionProcessor &proc) const {
	InstructionProcessor::Vector freedom = proc.getFreedomVector();
	proc.push (freedom.x.get_i());
	proc.push (freedom.y.get_i());
}

String GetFreedomVectorInstruction::getName() const {
	return "Get Freedom Vector";
}

/*** The SRP0, SRP1 and SRP2 instructions ***/

SetReferencePointInstruction::SetReferencePointInstruction(Byte rp) : Instruction(oiSRP0 + rp) {
	assert(rp<=2);
}

void SetReferencePointInstruction::execute (InstructionProcessor &proc) const {
	proc.setReferencePoint(instruction - oiSRP0, proc.pop());
}

String SetReferencePointInstruction::getName() const {
	return String ("Set Reference Point ") + ('0' + instruction - oiSRP0);
}

/*** The SZP0, SZP1 and SZP2 instructions ***/

SetZonePointerInstruction::SetZonePointerInstruction(Byte zp) : Instruction(oiSZP0 + zp) {
	assert(zp <= 2);
}

void SetZonePointerInstruction::execute (InstructionProcessor &proc) const {
	proc.setZonePointer(instruction - oiSZP0, proc.pop());
}

String SetZonePointerInstruction::getName() const {
	return String ("Set Zone Pointer ") + ('0' + instruction - oiSZP0);
}

/*** The SZPS instruction ***/

void SetZonePointersInstruction::execute (InstructionProcessor &proc) const {
	Long zone = proc.pop();
	proc.setZonePointer(0, zone);
	proc.setZonePointer(1, zone);
	proc.setZonePointer(2, zone);
}

String SetZonePointersInstruction::getName() const {
	return "Set Zone Pointers";
}

/*** The RTHG instruction ***/

RoundToHalfGridInstruction::RoundToHalfGridInstruction() : Instruction(oiRTHG) {}

void RoundToHalfGridInstruction::execute (InstructionProcessor &proc) const {
	proc.setRoundingState(1, NewF26Dot6 (.5), NewF26Dot6 (.5));
}

String RoundToHalfGridInstruction::getName() const {
	return "Set Round to Half Grid";
}

/*** The RTG instruction ***/

RoundToGridInstruction::RoundToGridInstruction() : Instruction(oiRTG) {}

void RoundToGridInstruction::execute (InstructionProcessor &proc) const {
	proc.setRoundingState(1, 0, NewF26Dot6 (.5));
}

String RoundToGridInstruction::getName() const {
	return "Set Round to Grid";
}

/*** The RTDG instruction ***/

RoundToDoubleGridInstruction::RoundToDoubleGridInstruction() : Instruction(oiRTDG) {}

void RoundToDoubleGridInstruction::execute (InstructionProcessor &proc) const {
	proc.setRoundingState(NewF26Dot6 (.5), 0, NewF26Dot6 (.25));
}

String RoundToDoubleGridInstruction::getName() const {
	return "Set Round to Double Grid";
}

/*** The RDTG instruction ***/

RoundDownToGridInstruction::RoundDownToGridInstruction() : Instruction(oiRDTG) {}

void RoundDownToGridInstruction::execute (InstructionProcessor &proc) const {
	proc.setRoundingState(1, 0, 0);
}

String RoundDownToGridInstruction::getName() const {
	return "Set Round Down to Grid";
}

/*** The RUTG instruction ***/

RoundUpToGridInstruction::RoundUpToGridInstruction() : Instruction(oiRUTG) {}

void RoundUpToGridInstruction::execute (InstructionProcessor &proc) const {
	proc.setRoundingState(1, 0, 1 - NewF26Dot6 (1, fixed_fraction()));
}

String RoundUpToGridInstruction::getName() const {
	return "Set Round Up to Grid";
}

/*** The ROFF instruction ***/

RoundOffInstruction::RoundOffInstruction() : Instruction(oiROFF) {}

void RoundOffInstruction::execute (InstructionProcessor &proc) const {
	proc.setRoundingState(NewF26Dot6 (1, fixed_fraction()), 0, 0);
}

String RoundOffInstruction::getName() const {
	return "Set Round Off";
}

/*** The SROUND instruction ***/

SuperRoundInstruction::SuperRoundInstruction() : Instruction(oiSROUND) {}

void SuperRoundInstruction::execute (InstructionProcessor &proc) const {
	Long n = proc.pop();
	NewF26Dot6 period, phase, threshold;

	switch ((n & oiRoundPeriod) >> oiRoundPeriodSh) {
	case 0:
		period = NewF26Dot6 (.5);
		break;
	case 1:
		period = 1;
		break;
	case 2:
		period = 2;
		break;
	default:
		throw InstructionException ("Invalid SROUND period value 3");
	}
	phase = period * ((n & oiRoundPhase) >> oiRoundPhaseSh) / 4;
	if ((n & oiRoundThreshold) == 0)
		threshold = period - 1;
	else
		threshold = (period * ((n & oiRoundThreshold)-4)) / 8;
	proc.setRoundingState(period, phase, threshold);
}

String SuperRoundInstruction::getName() const {
	return "Super Round";
}

/*** The S45ROUND instruction is equal to the SuperRound instruction, except for the default period
	 which is sqrt(2)/2 rather than 1 pixel. ***/

SuperRound45Instruction::SuperRound45Instruction() : Instruction(oiSROUND) {}

void SuperRound45Instruction::execute (InstructionProcessor &proc) const {
	Long n = proc.pop();
	NewF26Dot6 period, phase, threshold;

	switch ((n & oiRoundPeriod) >> oiRoundPeriodSh) {
	case 0:
		period = NewF26Dot6 (.25 * sqrt(2));
		break;
	case 1:
		period = NewF26Dot6 (.5 * sqrt(2));
		break;
	case 2:
		period = NewF26Dot6 (sqrt(2));
		break;
	default:
		throw InstructionException ("Invalid S45ROUND period value 3");
	}

	phase = period * ((n & oiRoundPhase) >> oiRoundPhaseSh) / 4;
	if ((n & oiRoundThreshold) == 0)
		threshold = period - 1;
	else
		threshold = (period * ((n & oiRoundThreshold)-4)) / 8;
	proc.setRoundingState(period, phase, threshold);
}

String SuperRound45Instruction::getName() const {
	return "Set Round 45 Degrees";
}

/*** The SLOOP instruction ***/

void SetLoopInstruction::execute (InstructionProcessor &proc) const {
	proc.setLoop(proc.pop());
}

String SetLoopInstruction::getName() const {
	return "Set Loop";
}

/*** The SMD instruction ***/

SetMinimumDistanceInstruction::SetMinimumDistanceInstruction() : Instruction(oiSMD) {}

void SetMinimumDistanceInstruction::execute (InstructionProcessor &proc) const {
	proc.setMinimumDistance(proc.popFixed());
}

String SetMinimumDistanceInstruction::getName() const {
	return "Set Minimum Distance";
}

/*** The INSTCTRL instruction ***/

InstructionControlInstruction::InstructionControlInstruction() : Instruction(oiINSTCTRL) {}

void InstructionControlInstruction::execute (InstructionProcessor &proc) const {
	ULong mask, value;
	mask = proc.pop();
	value = proc.pop();
	if ((mask & 0x03) != mask)
		proc.addWarning(new InstructionException (
			"Only bits 0 and 1 of instruction_control may be set"));
	proc.setInstructionExecutionControl(mask & 0x03, value & 0x03);
}

String InstructionControlInstruction::getName() const {
	return "Instruction Control";
}

/*** The SCANCTRL instruction ***/

ScanConversionControlInstruction::ScanConversionControlInstruction() : Instruction(oiSCANCTRL) {}

void ScanConversionControlInstruction::execute (InstructionProcessor &proc) const {
	// does nothing (yet)
	proc.pop();
}

String ScanConversionControlInstruction::getName() const {
	return "Scan Conversion Control";
}

/*** The SCANTYPE instruction ***/

ScanTypeInstruction::ScanTypeInstruction() : Instruction(oiSCANTYPE) {}

void ScanTypeInstruction::execute (InstructionProcessor &proc) const {
	// does nothing (yet)
	proc.pop();
}

String ScanTypeInstruction::getName() const {
	return "Scan Type";
}

/*** The SCVTCI instruction ***/

ControlValueCutInInstruction::ControlValueCutInInstruction() : Instruction(oiSCVTCI) {}

void ControlValueCutInInstruction::execute (InstructionProcessor &proc) const {
	proc.setControlValueCutIn(proc.popFixed());
}

String ControlValueCutInInstruction::getName() const {
	return "Set Control Value Cut-in";
}

/*** The SSWCI instruction ***/

SetSingleWidthCutInInstruction::SetSingleWidthCutInInstruction() : Instruction(oiSSWCI) {}

void SetSingleWidthCutInInstruction::execute (InstructionProcessor &proc) const {

	proc.setSingleWidthCutIn(proc.popFixed());
}

String SetSingleWidthCutInInstruction::getName() const {
	return "Set Single-width Cut-in";
}

/*** The SSW instruction ***/

SetSingleWidthInstruction::SetSingleWidthInstruction() : Instruction(oiSSW) {}

void SetSingleWidthInstruction::execute (InstructionProcessor &proc) const {

	// pop in 16.16 units, apparently
	proc.setSingleWidthValue(util::fixed <16, 16> (proc.pop(), fixed_fraction()));
}

String SetSingleWidthInstruction::getName() const {
	return "Set Single Width";
}

/*** The FLIPON and FLIPOFF instructions ***/

SetAutoFlipInstruction::SetAutoFlipInstruction(bool on) : Instruction(on ? oiFLIPON : oiFLIPOFF) {}

void SetAutoFlipInstruction::execute (InstructionProcessor &proc) const {

	proc.setAutoFlip(instruction == oiFLIPON);
}

String SetAutoFlipInstruction::getName() const {
	String name =  "Set Auto-flip ";
	if (instruction == oiFLIPON)
		name += "[on]";
	else
		name += "[off]";
	return name;
}

/*** The SDB instruction ***/

DeltaBaseInstruction::DeltaBaseInstruction() : Instruction(oiSDB) {}

void DeltaBaseInstruction::execute (InstructionProcessor &proc) const {

	proc.setDeltaBase(proc.pop());
}

String DeltaBaseInstruction::getName() const {
	return "Set Delta Base";
}

/*** The SDS instruction ***/

DeltaShiftInstruction::DeltaShiftInstruction() : Instruction(oiSDS) {}

void DeltaShiftInstruction::execute (InstructionProcessor &proc) const {

	proc.setDeltaShift(proc.pop());
}

String DeltaShiftInstruction::getName() const {
	return "Set Delta Shift";
}

/*** The GC instruction returns the current or the original position of a point ***/

GetCoordinateInstruction::GetCoordinateInstruction(bool orig) : Instruction(orig ? oiGCorig : oiGCcur) {}

void GetCoordinateInstruction::execute (InstructionProcessor &proc) const
{
	Long p = proc.pop();
	Long zp2 = proc.getZonePointer(2);
	if (instruction==oiGCcur)
		proc.push(proc.getPoint(zp2, p));
	else {
		assert(instruction==oiGCorig);
		proc.push(proc.getOriginalPoint(zp2, p, true));
	}
}

String GetCoordinateInstruction::getName() const {
	return "Get Coordinate " + getParameterCurOrig(instruction == oiGCcur);
}

/*** The SCFS instruction ***/

SetCoordinateInstruction::SetCoordinateInstruction() : Instruction(oiSCFS) {}

void SetCoordinateInstruction::execute (InstructionProcessor &proc) const {

	NewF26Dot6 value = proc.popFixed();
	Long p = proc.pop();
	Long zp2 = proc.getZonePointer(2);
	proc.movePoint(zp2, p, value);

	/*** UNDOCUMENTED: if the point is in the twilight zone, its original position
	is changed as well. ***/
	if (zp2 == 0)
		proc.moveOriginalPoint (zp2, p, value);
}

String SetCoordinateInstruction::getName() const {
	return "Set Coordinate";
}

/*** The MD instruction ***/

MeasureDistanceInstruction::MeasureDistanceInstruction(bool orig) : Instruction(orig ? oiMDorig : oiMDcur) {}

void MeasureDistanceInstruction::execute (InstructionProcessor &proc) const {

	Long p1, p2, zp0, zp1;
	p1 = proc.pop();
	p2 = proc.pop();
	zp1 = proc.getZonePointer(1);
	zp0 = proc.getZonePointer(0);
	if (instruction == oiMDcur)
		proc.push(proc.getPoint(zp0, p2) - proc.getPoint(zp1, p1));
	else {
		assert(instruction == oiMDorig);
		proc.push(proc.getOriginalPoint(zp0, p2, true) - proc.getOriginalPoint(zp1, p1, true));
	}
}

String MeasureDistanceInstruction::getName() const {
	return "Measure Distance " + getParameterCurOrig(instruction == oiMDcur);
}

/*** The MPPEM instruction ***/

MeasurePPEMInstruction::MeasurePPEMInstruction() : Instruction(oiMPPEM) {}

void MeasurePPEMInstruction::execute (InstructionProcessor &proc) const {

	proc.push(proc.getPPEM());
}

String MeasurePPEMInstruction::getName() const {
	return "Measure PPEM";
}

/*** The MPS instruction ***/

MeasurePointSizeInstruction::MeasurePointSizeInstruction() : Instruction(oiMPS) {}

void MeasurePointSizeInstruction::execute (InstructionProcessor &proc) const {
	proc.push(proc.getPointSize());
}

String MeasurePointSizeInstruction::getName() const {
	return "Measure Point Size";
}

/*** The FLIPPT instruction ***/

FlipPointInstruction::FlipPointInstruction() : Instruction(oiFLIPPT) {}

void FlipPointInstruction::execute (InstructionProcessor &proc) const {
	// Use loop variable
	ULong loop = proc.getLoop();
	if (proc.getZonePointer(0) != 1)
		throw InstructionException ("Points to be flipped may only be in zone 1");

	while (loop > 0) {
		ULong p = proc.pop();
		proc.setOnCurve(p, proc.getOnCurve(p));

		loop--;
	}

	proc.setLoop(1);
}

String FlipPointInstruction::getName() const {
	return "Flip Point";
}

/*** The FLIPRGON instruction ***/

FlipRangeInstruction::FlipRangeInstruction() : Instruction(oiFLIPRGON) {}

void FlipRangeInstruction::execute (InstructionProcessor &proc) const {
	ULong high, low;
	high = proc.pop();
	low = proc.pop();
	if (high < low)
		throw InstructionException ("Range defined in the wrong direction (high<low)");

	for (ULong i = low; i <= high; i ++)
		proc.setOnCurve(i, instruction==oiFLIPRGON);
}

String FlipRangeInstruction::getName() const {
	return "Flip Range";
}

/*** The SHP instruction ***/

ShiftPointInstruction::ShiftPointInstruction(bool use10) : Instruction(use10 ? oiSHP10 : oiSHP21) {}

void ShiftPointInstruction::execute (InstructionProcessor &proc) const {
	// Use Loop variable
	ULong loop = proc.getLoop();
	ULong zp2, zp, rp;
	zp2 = proc.getZonePointer(2);
	if (instruction==oiSHP21) {
		rp = proc.getReferencePoint(2);
		zp = proc.getZonePointer(1);
	} else {
		assert(instruction==oiSHP10);
		rp = proc.getReferencePoint(1);
		zp = proc.getZonePointer(0);
	}

	NewF26Dot6 distance = proc.getPoint(zp, rp) - proc.getOriginalPoint(zp, rp, false);

	while (loop>0) {
		ULong p = proc.pop();
		proc.shiftPoint(zp2, p, distance);

		loop--;
	}

	proc.setLoop(1);
}

String ShiftPointInstruction::getName() const {
	return "Shift Point " + getParameter210(instruction==oiSHP21);
}

/*** The SHC instruction ***/

ShiftContourInstruction::ShiftContourInstruction(bool use10) : Instruction(use10 ? oiSHC10 : oiSHC21) {}

void ShiftContourInstruction::execute (InstructionProcessor &proc) const {
	ULong zp2, zp, rp;
	zp2 = proc.getZonePointer(2);
	if (instruction==oiSHC21) {
		rp = proc.getReferencePoint(2);
		zp = proc.getZonePointer(1);
	} else {
		assert(instruction==oiSHC10);
		rp = proc.getReferencePoint(1);
		zp = proc.getZonePointer(0);
	}

	if (proc.getZonePointer(2)!=1)
		throw InstructionException ("Contour shifting can only be done in zone 1");

	NewF26Dot6 distance = proc.getPoint(zp, rp) - proc.getOriginalPoint(zp, rp, false);

	ULong n = proc.pop();
	ULong first, last;
	if (n>0)
		first = proc.getLastContourPoint(n-1)+1;
	else
		first = 0;
	last = proc.getLastContourPoint(n);

	assert (! (first > last));

	for (ULong i=first; i<=last; i++) {
		if (zp!=1 || i != rp)
			proc.shiftPoint(1, i, distance);
	}
}

String ShiftContourInstruction::getName() const {
	return "Shift Contour" + getParameter210(instruction==oiSHC21);
}


/*** The SHZ instruction ***/

ShiftZoneInstruction::ShiftZoneInstruction(bool use10) : Instruction(use10 ? oiSHZ10 : oiSHZ21) {}

void ShiftZoneInstruction::execute (InstructionProcessor &proc) const {
	ULong zone = proc.pop();

	ULong zp, rp;
	if (instruction==oiSHZ21) {
		rp = proc.getReferencePoint(2);
		zp = proc.getZonePointer(1);
	} else {
		assert(instruction==oiSHZ10);
		rp = proc.getReferencePoint(1);
		zp = proc.getZonePointer(0);
	}

	NewF26Dot6 distance = proc.getPoint(zp, rp) - proc.getOriginalPoint(zp, rp, false);

	ULong max;
	if (zp==0)
		max = proc.getTwilightPointNum();
	else {
		assert(zp==1);
		max = proc.getPointNum()-4; // -4: leave phantom points alone
	}

	/*** UNDOCUMENTED: the points are not touched. ***/

	for (ULong i=0; i<max; i++) {
		if (zone!=zp || i != rp)
			proc.shiftPoint(zone, i, distance, false);
	}
}

String ShiftZoneInstruction::getName() const {
	return "Shift Zone " + getParameter210(instruction==oiSHZ21);
}


/*** The SHPIX instruction ***/

ShiftPointByPixelsInstruction::ShiftPointByPixelsInstruction() : Instruction(oiSHPIX) {}

void ShiftPointByPixelsInstruction::execute (InstructionProcessor &proc) const {
	NewF26Dot6 amount = proc.popFixed();
	Long zp2 = proc.getZonePointer(2);

	ULong loop = proc.getLoop();
	while (loop>0) {
		Long p = proc.pop();
		proc.shiftPoint(zp2, p, amount);
		loop --;
	}

	proc.setLoop(1);
}

String ShiftPointByPixelsInstruction::getName() const {
	return "Shift Point by Pixels";
}

/*** The MSIRP instruction ***/

MoveStackIndirectRelativePointInstruction::MoveStackIndirectRelativePointInstruction(bool setrp0) :
Instruction(setrp0 ? oiMSIRPset : oiMSIRP) {}

void MoveStackIndirectRelativePointInstruction::execute (InstructionProcessor &proc) const {
	NewF26Dot6 distance = proc.popFixed();
	Long p = proc.pop();
	Long rp0 = proc.getReferencePoint(0);
	Long zp0 = proc.getZonePointer(0);
	Long zp1 = proc.getZonePointer(1);

	/*** UNDOCUMENTED: when moving a twilight zone point, its original position is
	changed to the original position before it is moved as well.
	Also, the current position is set regardless of vector states. ***/
	if (zp1 == 0) {
		proc.moveOriginalPointToXY (zp1, p, proc.getPointX (zp0, rp0), proc.getPointY (zp0, rp0));
		proc.moveOriginalPoint (zp1, p, proc.getPoint (zp0, rp0) + distance);
		proc.movePointToXY (zp1, p, proc.getPointX (zp0, rp0), proc.getPointY (zp0, rp0));
	}

	proc.movePoint(zp1, p, proc.getPoint(zp0, rp0) + distance);

	proc.setReferencePoint(1, rp0);
	proc.setReferencePoint(2, p);
	if (instruction==oiMSIRPset)
		proc.setReferencePoint(0, p);
}

String MoveStackIndirectRelativePointInstruction::getName() const {
	return "Move Stack Indirect Relative Point";
}

/*** The MDAP instruction ***/

MoveDirectAbsolutePointInstruction::MoveDirectAbsolutePointInstruction(bool round) :
Instruction(round ? oiMDAPround : oiMDAP) {}

void MoveDirectAbsolutePointInstruction::execute (InstructionProcessor &proc) const {
	Long p = proc.pop();
	Long zp0 = proc.getZonePointer(0);
	if (instruction == oiMDAP)
		// Just touch point
		proc.movePoint(zp0, p, proc.getPoint(zp0, p));
	else {
		assert(instruction == oiMDAPround);
		proc.movePoint(zp0, p, proc.round (proc.getPoint (zp0, p)));
	}
	proc.setReferencePoint(0, p);
	proc.setReferencePoint(1, p);
}

String MoveDirectAbsolutePointInstruction::getName() const {
	return "Move Direct Absolute Point " +
		getParameterRound(instruction == oiMDAPround);
}

/*** The MIAP instruction ***/

MoveIndirectAbsolutePointInstruction::MoveIndirectAbsolutePointInstruction(bool round) :
Instruction(round ? oiMIAPround : oiMIAP) {}

void MoveIndirectAbsolutePointInstruction::execute (InstructionProcessor &proc) const {
	ULong n, p;
	n = proc.pop();
	p = proc.pop();
	ULong z = proc.getZonePointer(0);
	NewF26Dot6 cvtPos = proc.getCVTValue(n);
	NewF26Dot6 curPos = proc.getPoint(z, p);

/*** UNDOCUMENTED: In the twilight zone, the original position is set to the
	unrounded CVT distance. (Thanks to FreeType for this.) ***/

    if ( z == 0 )   /* If in twilight zone */
		proc.moveOriginalPoint (z, p, cvtPos);

	// Apparently autoflip is not used for MIAP?
/*	if (proc.getAutoFlip()) {
		if ((curPos<0 && cvtPos>0) || (curPos>0 && cvtPos<0))
			cvtPos = -cvtPos;
	}*/


	if (instruction == oiMIAP) {
		proc.movePoint(z, p, cvtPos);
	} else {
		assert(instruction==oiMIAPround);
		NewF26Dot6 cutIn = proc.getControlValueCutIn();
/*** UNDOCUMENTED: In the twilight zone, no cvt cut-in checking is done. ***/
		if (z == 0 || (cvtPos - curPos) < cutIn && (curPos - cvtPos) < cutIn)
			proc.movePoint(z, p, proc.round (cvtPos));
		else
			// Round original position according to cvtCutIn
			proc.movePoint(z, p, proc.round (curPos));
	}
	proc.setReferencePoint(0, p);
	proc.setReferencePoint(1, p);
}

String MoveIndirectAbsolutePointInstruction::getName() const {
	return "Move Indirect Absolute Point " +
		getParameterRound(instruction == oiMIAPround);
}

/*** The MDRP instruction ***/

MoveDirectRelativePointInstruction::MoveDirectRelativePointInstruction(bool setrp0, bool minDist,
																		   bool round, Byte colour) :
Instruction(oiMDRP | (setrp0 ? oiMRPsetrp0 : 0) | (minDist ? oiMRPminDist : 0) |
			  (round ? oiMRPround : 0) | colour) {}

void MoveDirectRelativePointInstruction::execute (InstructionProcessor &proc) const {
	Long p = proc.pop();
	Long zp1 = proc.getZonePointer(1);
	Long rp0 = proc.getReferencePoint(0);
	Long zp0 = proc.getZonePointer(0);

	// Calculate distance
	NewF26Dot6 newDist = proc.getOriginalPoint(zp1, p, true) - proc.getOriginalPoint(zp0, rp0, true);
	bool autoFlip = proc.getAutoFlip();
	bool negative = (newDist<0);

	// Compare with single-width cut-in
	NewF26Dot6 cutIn = proc.getSingleWidthCutIn();
	NewF26Dot6 singleWidthValue = proc.getSingleWidthValue();
//	if (autoFlip) {
		if ((singleWidthValue>=0) == negative) {
			singleWidthValue = -singleWidthValue;
		}
//	}
	if ((newDist - singleWidthValue) < cutIn && (singleWidthValue - newDist) < cutIn)
		newDist = singleWidthValue;

	// Round
	if (instruction & oiMRPround)
		newDist = proc.round (newDist);

	// Compare with minimum distance
	if (instruction & oiMRPminDist) {
		NewF26Dot6 minDist = proc.getMinimumDistance();
//		if (autoFlip) {
			if ((minDist>=0)==negative) {
				if (-minDist < newDist)
					newDist = -minDist;
			} else {
				if (newDist < minDist)
					newDist = minDist;
			}
//		} else {
//			if (newDist < minDist)
//				newDist = minDist;
//		}
	}

	// Compensate for engine characteristics
	newDist = proc.compensateForColour (newDist, instruction & oiMRPcolour);

	// Move point
	proc.movePoint(zp1, p, proc.getPoint(zp0, rp0) + newDist);

	// Set reference points
	proc.setReferencePoint(1, rp0);
	proc.setReferencePoint(2, p);
	if (instruction & oiMRPsetrp0)
		proc.setReferencePoint(0, p);
}

String MoveDirectRelativePointInstruction::getName() const {
	return "Move Direct Relative Point" + getParameterMRP (instruction);
}

/*** The MIRP instruction ***/

MoveIndirectRelativePointInstruction::MoveIndirectRelativePointInstruction(bool setrp0, bool minDist,
																			   bool round, Byte colour) :
Instruction(oiMIRP | (setrp0 ? oiMRPsetrp0 : 0) | (minDist ? oiMRPminDist : 0) |
			  (round ? oiMRPround : 0) | colour) {}

void MoveIndirectRelativePointInstruction::execute (InstructionProcessor &proc) const {
	NewF26Dot6 cvtDist = proc.getCVTValue (proc.pop());
	Long p = proc.pop();
	Long rp0 = proc.getReferencePoint(0);
	Long zp0 = proc.getZonePointer(0);
	Long zp1 = proc.getZonePointer(1);
	
	NewF26Dot6 rp0Pos = proc.getPoint(zp0, rp0);
	NewF26Dot6 newDist = proc.getOriginalPoint(zp1, p, true) - proc.getOriginalPoint(zp0, rp0, true);
	bool negative = newDist<0;
	bool autoFlip = proc.getAutoFlip();

	if (autoFlip) {
		if ((cvtDist>=0) == negative)
			cvtDist = -cvtDist;
	}

	if (instruction & oiMRPround)
	{	// Do round and do look at CVT cutin value
		NewF26Dot6 cutIn = proc.getControlValueCutIn();
		if ((cvtDist - newDist) < cutIn && (newDist - cvtDist) < cutIn)
			newDist = proc.round (cvtDist);
		else
			newDist = proc.round (newDist);
	} else
		// Do not round or look at CVT cutin value
		newDist = cvtDist;
	
	if (instruction & oiMRPminDist)
	{	// Minimum distance given
		NewF26Dot6 minDist = proc.getMinimumDistance();
		if (autoFlip) {
			if ((minDist>=0)==negative) {
				if (-minDist < newDist)
					newDist = -minDist;
			} else {
				if (newDist < minDist)
					newDist = minDist;
			}
		} else {
			if (newDist < minDist)
				newDist = minDist;
		}
	}

	// Compensate for engine characteristics
	newDist = proc.compensateForColour (newDist, instruction & oiMRPcolour);

	/*** UNDOCUMENTED: when moving a twilight zone point, its original position is changed as well. ***/
	if (zp1 == 0) {
		proc.movePointToXY (zp1, p, proc.getPointX (zp0, rp0), proc.getPointY (zp0, rp0));
		proc.moveOriginalPointToXY (zp1, p, proc.getPointX (zp0, rp0), proc.getPointY (zp0, rp0));
		proc.moveOriginalPoint (zp1, p, rp0Pos + newDist);
	}

	proc.movePoint(zp1, p, rp0Pos + newDist);
	proc.setReferencePoint(1, rp0);
	proc.setReferencePoint(2, p);
	if (instruction & oiMRPsetrp0)
		proc.setReferencePoint(0, p);
}

String MoveIndirectRelativePointInstruction::getName() const {
	return "Move Indirect Relative Point " + getParameterMRP(instruction);
}

/*** The ALIGN instruction ***/

AlignInstruction::AlignInstruction() : Instruction(oiALIGN) {}

void AlignInstruction::execute (InstructionProcessor &proc) const {
	Long rp0 = proc.getReferencePoint(0);
	Long zp0 = proc.getZonePointer(0);
	Long zp1 = proc.getZonePointer(1);
	NewF26Dot6 targetPos = proc.getPoint(zp0, rp0);

	ULong loop = proc.getLoop();
	while (loop) {
		Long p = proc.pop();
		proc.movePoint(zp1, p, proc.getPoint (zp0, rp0));

		loop--;
	}

	proc.setLoop(1);
}

String AlignInstruction::getName() const {
	return "Align";
}

/*** The ISECT instruction ***/

MoveToIntersectionInstruction::MoveToIntersectionInstruction() : Instruction(oiISECT) {}


// See documentation for calculations behind all this; keep in mind that parallel lines
// Should result in putting the point in the exact middle.
void MoveToIntersectionInstruction::execute (InstructionProcessor &proc) const {
	ULong a0, a1, b0, b1, p;
	b1 = proc.pop();
	b0 = proc.pop();
	a1 = proc.pop();
	a0 = proc.pop();
	p = proc.pop();

	ULong zp2, zp1, zp0;
	zp2 = proc.getZonePointer(2);
	zp1 = proc.getZonePointer(1);
	zp0 = proc.getZonePointer(0);

/*	NewF26Dot6 a0x, a0y, a1x, a1y, dax, day, b0x, b0y, b1x, b1y, dbx, dby;

	a0x = proc.getPointX(zp1, a0);
	a0y = proc.getPointY(zp1, a0);
	a1x = proc.getPointX(zp1, a1);
	a1y = proc.getPointY(zp1, a1);
	dax = a1x - a0x;
	day = a1y - a0y;

	b0x = proc.getPointX(zp1, b0);
	b0y = proc.getPointY(zp1, b0);
	b1x = proc.getPointX(zp1, b1);
	b1y = proc.getPointY(zp1, b1);
	dbx = b1x - b0x;
	dby = b1y - b0y;

	if (dax * dby == day * dbx) {
		// Put point in the middle
		proc.movePointToXY(zp2, p, (a0x+a1x+b0x+b1x+2)/4, (a0y+a1y+b0y+b1y+2)/4);
	} else {

		NewF26Dot6 x, y;
		if (dax != 0 && dbx != 0) {
			F18Dot14 gradienta = (day / dax);
			F18Dot14 gradientb = (dby / dbx);

			x = ((b0y - a0y) - (dby / dbx) * b0x + (day / dax) * a0x) / ((day / dax) - (dby / dbx));
		} else {
			if (dax == 0)
				x = a0x;
			else
				x = b0x;
		}

		if (day != 0 && dby != 0) {
			F18Dot14 gradienta = (day / dax);
			F18Dot14 gradientb = (dby / dbx);

			y = ((b0x - a0x) - (dby / dbx) * b0y + (day / dax) * a0y) / ((day / dax) - (dby / dbx));
		} else {
			if (day == 0)
				y = a0y;
			else
				y = b0y;
		}

		proc.movePointToXY(zp2, p, x, y);
	}*/

	NewF26Dot6 xa = proc.getPointX (zp1, a0);
	NewF26Dot6 ya = proc.getPointY (zp1, a0);
	NewF26Dot6 dxa = proc.getPointX (zp1, a1) - xa;
	NewF26Dot6 dya = proc.getPointY (zp1, a1) - ya;

	NewF26Dot6 xb = proc.getPointX (zp0, b0);
	NewF26Dot6 yb = proc.getPointY (zp0, b0);
	NewF26Dot6 dxb = proc.getPointX (zp0, b1) - xb;
	NewF26Dot6 dyb = proc.getPointY (zp0, b1) - yb;

	if (dxb*dya == dxa*dyb)
	{	// Put point in the middle
		proc.movePointToXY(zp2, p,
			(xa + xb + (dxa + dxb) / 2) / 2,
			(ya + yb + (dya + dyb) / 2) / 2);
	} else {
		NewF26Dot6 x = (-xb*dxa*dyb + dxa*dxb*yb - dxa*dxb*ya + xa*dxb*dya) /
			(dxb*dya - dxa*dyb);
		NewF26Dot6 y = (dxb*dya*yb - dxa*ya*dyb - xb*dya*dyb + xa*dya*dyb) /
			(dxb*dya - dxa*dyb);

		proc.movePointToXY (zp2, p, x, y);
	}
}

String MoveToIntersectionInstruction::getName() const {
	return "Move to Intersection";
}

/*** The ALIGNPTS instruction ***/

AlignPointsInstruction::AlignPointsInstruction() : Instruction(oiALIGNPTS) {}

void AlignPointsInstruction::execute (InstructionProcessor &proc) const {
	Long rp0 = proc.getReferencePoint(0);
	Long zp0 = proc.getZonePointer(0);
	Long zp1 = proc.getZonePointer(1);

	ULong p1 = proc.pop();
	ULong p2 = proc.pop();
	NewF26Dot6 newPos = (proc.getPoint(zp1, p1) + proc.getPoint(zp0, p2) - p1 + 1) / 2;
	proc.movePoint(zp1, p1, newPos);
	proc.movePoint(zp0, p2, newPos);
}

String AlignPointsInstruction::getName() const {
	return "Align Points";
}

/*** The IP instruction ***/

InterpolatePointInstruction::InterpolatePointInstruction() : Instruction(oiIP) {}

void InterpolatePointInstruction::execute (InstructionProcessor &proc) const {
	ULong loop = proc.getLoop();
	while (loop) {
		NewF26Dot6 original1, original2, originalPos, current1, current2, newPos;
		
		Long p = proc.pop();
		Long zp0 = proc.getZonePointer(0);
		Long zp1 = proc.getZonePointer(1);
		Long zp2 = proc.getZonePointer(2);
		Long rp1 = proc.getReferencePoint(1);
		Long rp2 = proc.getReferencePoint(2);
		
		original1 = proc.getOriginalPoint(zp0, rp1, true);
		original2 = proc.getOriginalPoint(zp1, rp2, true);
		originalPos  = proc.getOriginalPoint(zp2, p, true);
		current1 = proc.getPoint(zp0, rp1);
		current2 = proc.getPoint(zp1, rp2);
		
		// Interpolate third point so that its relation with the other 2 holds again
		// Prevent from divide by zero
		if (original1 == original2)
			newPos = (current1 + current2) / 2;
		else
			newPos = (current1 + ((originalPos - original1) *
				(current2 - current1)) / (original2 - original1));
		proc.movePoint(zp2, p, newPos);
		
		loop--;
	}
	proc.setLoop(1);
}

String InterpolatePointInstruction::getName() const {
	return "Interpolate Point";
}

/*** The UTP instruction ***/

UntouchPointInstruction::UntouchPointInstruction() : Instruction(oiUTP) {}

void UntouchPointInstruction::execute (InstructionProcessor &proc) const {
	if (proc.getZonePointer(0)!=1)
		throw InstructionException ("Untouching points is only possible in zone 1");

	proc.unTouchPoint(proc.pop());
}

String UntouchPointInstruction::getName() const {
	return "Untouch Point";
}

/*** The IUP instruction ***/

InterpolateUntouchedPointsInstruction::InterpolateUntouchedPointsInstruction(bool x) :
Instruction(x ? oiIUPx : oiIUPy) {}

void InterpolateUntouchedPointsInstruction::execute (InstructionProcessor &proc) const {
	if (proc.getZonePointer (2) != 1)
		throw InstructionException ("IUP instruction only works with zone 1");

	if (instruction == oiIUPx)
		proc.interpolatePointsX();
	else
		proc.interpolatePointsY();
}

String InterpolateUntouchedPointsInstruction::getName() const {
	return "Interpolate Untouched Points " + getParameterXY(instruction == oiIUPx);
}

/*** The DELTAP instructions ***/

DeltaPInstruction::DeltaPInstruction(Byte type) : Instruction(oiDELTAP1 - 1 + type) {
	assert(type>=1 && type <=3);
}

void DeltaPInstruction::execute (InstructionProcessor &proc) const {
	Long n = proc.pop();
	ULong zp = proc.getZonePointer(0);
	ULong deltaBase = proc.getDeltaBase();
	ULong deltaShift = proc.getDeltaShift();

	if (instruction == oiDELTAP2)
		deltaBase +=16;
	else {
		if (instruction == oiDELTAP3)
			deltaBase +=32;
		else
			assert(instruction == oiDELTAP1);
	}

	ULong ppem = proc.getPPEM();

	while (n>0) {
		ULong p = proc.pop();
		ULong arg = proc.pop();
		if (ppem == deltaBase + ((arg>>4)&0x0F)) {
			NewF26Dot6 magnitude = (arg & 0x0F);
			if (magnitude<=7)
				magnitude -= 8;
			else
				magnitude -= 7;
			proc.shiftPoint(zp, p, magnitude / (1 << deltaShift));
		}
		n--;
	}
}

String DeltaPInstruction::getName() const {
	String name = "DeltaP ";
	if (instruction == oiDELTAP2)
		name += "[base+16]";
	else {
		if (instruction == oiDELTAP3)
			name += "[base+32]";
		else {
			assert(instruction == oiDELTAP1);
			name += "[base+0]";
		}
	}
	return name;
}

/*** The DELTAC instruction ***/

DeltaCInstruction::DeltaCInstruction(Byte type) : Instruction(oiDELTAC1 - 1 + type) {
	assert(type>=1 && type <=3);
}

void DeltaCInstruction::execute (InstructionProcessor &proc) const {
	Long n = proc.pop();
	ULong zp = proc.getZonePointer(0);
	ULong deltaBase = proc.getDeltaBase();
	ULong deltaShift = proc.getDeltaShift();

	if (instruction == oiDELTAC2)
		deltaBase +=16;
	else {
		if (instruction == oiDELTAC3)
			deltaBase +=32;
		else
			assert(instruction == oiDELTAC1);
	}

	ULong ppem = proc.getPPEM();

	for (int i = 0; i<n; i++) {
		ULong p = proc.pop();
		ULong arg = proc.pop();
		if (ppem == deltaBase + ((arg>>4)&0x0F)) {
			NewF26Dot6 magnitude = (arg & 0x0F);
			if (magnitude <= 7)
				magnitude -= 8;
			else
				magnitude -= 7;
			proc.setCVTValuePixels(p, proc.getCVTValue(p) + magnitude / (1 << deltaShift));
		}
	}
}

String DeltaCInstruction::getName() const {
	String name = "DeltaC";
	if (instruction == oiDELTAC2)
		name += "[base+16]";
	else {
		if (instruction == oiDELTAC3)
			name += "[base+32]";
		else {
			assert(instruction == oiDELTAC1);
			name += "[base+0]";
		}
	}
	return name;
}

/*** The DUP instruction ***/

DuplicateStackElementInstruction::DuplicateStackElementInstruction() : Instruction(oiDUP) {}

void DuplicateStackElementInstruction::execute (InstructionProcessor &proc) const {
	Long el = proc.pop();
	proc.push(el);
	proc.push(el);
}

String DuplicateStackElementInstruction::getName() const {
	return "Duplicate Stack Element";
}

/*** The POP instruction ***/

PopInstruction::PopInstruction() : Instruction(oiPOP) {}

void PopInstruction::execute (InstructionProcessor &proc) const {
	proc.pop();
}

String PopInstruction::getName() const {
	return "Pop";
}

/*** The CLEAR instruction ***/

void ClearStackInstruction::execute (InstructionProcessor &proc) const {
	proc.clearStack();
}

String ClearStackInstruction::getName() const {
	return "Clear Stack";
}

/*** The SWAP instruction ***/

SwapStackElementsInstruction::SwapStackElementsInstruction() : Instruction(oiSWAP) {}

void SwapStackElementsInstruction::execute (InstructionProcessor &proc) const {
	Long e1, e2;
	e2 = proc.pop();
	e1 = proc.pop();
	proc.push(e2);
	proc.push(e1);
}

String SwapStackElementsInstruction::getName() const {
	return "Swap Stack Elements";
}

/*** The DEPTH instruction ***/

void StackDepthInstruction::execute (InstructionProcessor &proc) const {
	proc.push(proc.getStackElementNum());
}

String StackDepthInstruction::getName() const {
	return "Get Stack Depth";
}

/*** The CINDEX instruction ***/

void CopyIndexInstruction::execute (InstructionProcessor &proc) const {
	// Duplicate n - pop()th stack element; compensate for popping by subtracting 1
	proc.push(proc.getNthStackElement(proc.pop()-1));
}

String CopyIndexInstruction::getName() const {
	return "Copy Index";
}

/*** The MINDEX instruction ***/

void MoveIndexInstruction::execute (InstructionProcessor &proc) const {
	proc.push(proc.removeNthStackElement(proc.pop()-1));
}

String MoveIndexInstruction::getName() const {
	return "Move Index";
}

/*** The ROLL instruction ***/

void RollStackInstruction::execute (InstructionProcessor &proc) const {
	Long a = proc.pop();
	Long b = proc.pop();
	Long c = proc.pop();
	proc.push(b);
	proc.push(a);
	proc.push(c);
}

String RollStackInstruction::getName() const {
	return "Roll Stack";
}

/*** The IF instruction ***/

void IfInstruction::execute (InstructionProcessor &proc) const {
	if (! proc.pop())
	{
		// Level of IF statements
		int level = 1;
		while (true) {
			InstructionProcessor::InstructionIterator i = proc.skipNextInstruction();
			if ((*i)->instruction == oiELSE && level == 1)
				return;
			if ((*i)->instruction == oiEIF) {
				level --;
				if (level == 0)
					return;
			}
			if ((*i)->instruction == oiIF)
				level ++;
		}
	}
}

String IfInstruction::getName() const {
	return "If";
}

/*** The ELSE instruction jumps over the "else" block when executed ***/

void ElseInstruction::execute (InstructionProcessor &proc) const {
	int level = 1;
	while (true) {
		InstructionProcessor::InstructionIterator i = proc.skipNextInstruction();
		if ((*i)->instruction == oiELSE && level == 1)
			throw InstructionException ("Stray ELSE instruction in ELSE block");
		if ((*i)->instruction == oiEIF) {
			level --;
			if (level == 0)
				return;
		}
		if  ((*i)->instruction == oiIF)
			level ++;
	}
}

String ElseInstruction::getName() const {
	return "Else";
}

/*** The EIF instruction signifies the end of an IF-THEN-ELSE block. It does nothing when executed. ***/

void EndIfInstruction::execute (InstructionProcessor &proc) const {}

String EndIfInstruction::getName() const {
	return "End If";
}

/*** The JR instruction ***/

void JumpRelOnTrueInstruction::execute (InstructionProcessor &proc) const {
	Long b = proc.pop();
	Long jumpOffset = proc.pop();
	if (b)
		proc.jumpTo (offset + jumpOffset);
}

String JumpRelOnTrueInstruction::getName() const {
	return "Jump Relative on True";
}

/*** The JMP instruction ***/

JumpInstruction::JumpInstruction() : Instruction(oiJMP) {}

void JumpInstruction::execute (InstructionProcessor &proc) const {
	Long jumpOffset = proc.pop();
	proc.jumpTo (offset + jumpOffset);
}

String JumpInstruction::getName() const {
	return "Jump";
}

/*** The JROF instruction ***/

JumpRelOnFalseInstruction::JumpRelOnFalseInstruction() : Instruction (oiJROF) {}

void JumpRelOnFalseInstruction::execute (InstructionProcessor &proc) const {
	Long b = proc.pop();
	Long jumpOffset = proc.pop();
	if (!b)
		proc.jumpTo (offset + jumpOffset);
}

String JumpRelOnFalseInstruction::getName() const {
	return "Jump Relative on False";
}

/*** The LT instruction ***/

LessThanInstruction::LessThanInstruction() : Instruction(oiLT) {}

void LessThanInstruction::execute (InstructionProcessor &proc) const {
	Long e2 = proc.pop();
	Long e1 = proc.pop();
	if (e1 < e2)
		proc.push(1);
	else
		proc.push(0);
}

String LessThanInstruction::getName() const {
	return "Less Than";
}

/*** The LTEQ instruction ***/

LessThanOrEqualInstruction::LessThanOrEqualInstruction() : Instruction(oiLTEQ) {}

void LessThanOrEqualInstruction::execute (InstructionProcessor &proc) const {
	Long e2 = proc.pop();
	Long e1 = proc.pop();
	if (e1 <= e2)
		proc.push(1);
	else
		proc.push(0);
}

String LessThanOrEqualInstruction::getName() const {
	return "Less Than or Equal";
}

/*** The GT instruction ***/

GreaterThanInstruction::GreaterThanInstruction() : Instruction(oiGT) {}

void GreaterThanInstruction::execute (InstructionProcessor &proc) const {
	Long e2 = proc.pop();
	Long e1 = proc.pop();
	if (e1 > e2)
		proc.push(1);
	else
		proc.push(0);
}

String GreaterThanInstruction::getName() const {
	return "Greater Than";
}

/*** The GTEQ instruction ***/

GreaterThanOrEqualInstruction::GreaterThanOrEqualInstruction() : Instruction(oiGTEQ) {}

void GreaterThanOrEqualInstruction::execute (InstructionProcessor &proc) const {
	Long e2 = proc.pop();
	Long e1 = proc.pop();
	if (e1 >= e2)
		proc.push(1);
	else
		proc.push(0);
}

String GreaterThanOrEqualInstruction::getName() const {
	return "Greater Than or Equal";
}

/*** The EQ instruction ***/

EqualInstruction::EqualInstruction() : Instruction(oiEQ) {}

void EqualInstruction::execute (InstructionProcessor &proc) const {
	ULong e2 = proc.pop();
	ULong e1 = proc.pop();
	if (e1 == e2)
		proc.push(1);
	else
		proc.push(0);
}

String EqualInstruction::getName() const {
	return "Equal";
}

/*** The NEQ instruction ***/

NotEqualInstruction::NotEqualInstruction() : Instruction(oiNEQ) {}

void NotEqualInstruction::execute (InstructionProcessor &proc) const {
	ULong e2 = proc.pop();
	ULong e1 = proc.pop();
	if (e1 != e2)
		proc.push(1);
	else
		proc.push(0);
}

String NotEqualInstruction::getName() const {
	return "Not Equal";
}

/*** The ODD instruction ***/

OddInstruction::OddInstruction() : Instruction (oiODD) {}

void OddInstruction::execute (InstructionProcessor &proc) const {
	proc.push ((proc.round (proc.popFixed()).get_integer()) & 0x01);
}

String OddInstruction::getName() const {
	return "Odd";
}

/*** The EVEN instruction ***/

EvenInstruction::EvenInstruction() : Instruction(oiEVEN) {}

void EvenInstruction::execute (InstructionProcessor &proc) const {
	proc.push (!((proc.round (proc.popFixed()).get_integer()) & 0x01));
}

String EvenInstruction::getName() const {
	return "Even";
}

/*** The AND instruction ***/

AndInstruction::AndInstruction() : Instruction(oiAND) {}

void AndInstruction::execute (InstructionProcessor &proc) const {
	Long n1 = proc.pop();
	Long n2 = proc.pop();
	proc.push(n1 && n2);
}

String AndInstruction::getName() const {
	return "And";
}

/*** The OR instruction ***/

OrInstruction::OrInstruction() : Instruction(oiOR) {}

void OrInstruction::execute (InstructionProcessor &proc) const {
	Long n1 = proc.pop();
	Long n2 = proc.pop();
	proc.push(n1 || n2);
}

String OrInstruction::getName() const {
	return "Or";
}

/*** The N instruction ***/

NotInstruction::NotInstruction() : Instruction(oiN) {}

void NotInstruction::execute (InstructionProcessor &proc) const {
	proc.push(!proc.pop());
}

String NotInstruction::getName() const {
	return "Not";
}

/*** The ADD instruction ***/

AddInstruction::AddInstruction() : Instruction(oiADD) {}

void AddInstruction::execute (InstructionProcessor &proc) const {
	proc.push(proc.pop() + proc.pop());
}

String AddInstruction::getName() const {
	return "Add";
}

/*** The SUB instruction ***/

SubtractInstruction::SubtractInstruction() : Instruction(oiSUB) {}

void SubtractInstruction::execute (InstructionProcessor &proc) const {
	Long n1, n2;
	n1 = proc.pop();
	n2 = proc.pop();
	proc.push(n2 - n1);
}

String SubtractInstruction::getName() const {
	return "Sub";
}

/*** The DIV instruction ***/

DivideInstruction::DivideInstruction() : Instruction(oiDIV) {}

void DivideInstruction::execute (InstructionProcessor &proc) const {
	NewF26Dot6 n1, n2;
	n1 = proc.popFixed();
	n2 = proc.popFixed();
	if (n1==0)
		throw InstructionException ("Division by zero");

	proc.push (NewF26Dot6 (n2.divide_unrounded (n1), util::fixed_unrounded()));
}

String DivideInstruction::getName() const {
	return "Divide";
}

/*** The MUL instruction ***/

MultiplyInstruction::MultiplyInstruction() : Instruction(oiMUL) {}

void MultiplyInstruction::execute (InstructionProcessor &proc) const {
	NewF26Dot6 n1, n2;
	n1 = proc.popFixed();
	n2 = proc.popFixed();
	proc.push (n1 * n2);
}

String MultiplyInstruction::getName() const {
	return "Multiply";
}

/*** The ABS instruction ***/

AbsoluteInstruction::AbsoluteInstruction() : Instruction(oiABS) {}

void AbsoluteInstruction::execute (InstructionProcessor &proc) const {
	Long n = proc.pop();
	if (n<0)
		proc.push(-n);
	else
		proc.push(n);
}

String AbsoluteInstruction::getName() const {
	return "Abs";
}

/*** The NEG instruction ***/

NegateInstruction::NegateInstruction() : Instruction(oiNEG) {}

void NegateInstruction::execute (InstructionProcessor &proc) const {
	proc.push(-proc.pop());
}

String NegateInstruction::getName() const {
	return "Negate";
}

/*** The FLOOR instruction ***/

FloorInstruction::FloorInstruction() : Instruction(oiFLOOR) {}

void FloorInstruction::execute (InstructionProcessor &proc) const {
	proc.push(proc.pop() & ~0x3F);
}

String FloorInstruction::getName() const {
	return "Floor";
}

/*** The CEILING instruction ***/

CeilingInstruction::CeilingInstruction() : Instruction(oiCEILING) {}

void CeilingInstruction::execute (InstructionProcessor &proc) const {
	proc.push((proc.pop() + 0x3F) & ~0x3F);
}

String CeilingInstruction::getName() const {
	return "Ceiling";
}

/*** The MAX instruction ***/

MaximumInstruction::MaximumInstruction() : Instruction(oiMAX) {}

void MaximumInstruction::execute (InstructionProcessor &proc) const {
	Long n1, n2;
	n1 = proc.pop();
	n2 = proc.pop();
	if (n1 > n2)
		proc.push(n1);
	else
		proc.push(n2);
}

String MaximumInstruction::getName() const {
	return "Maximum";
}

/*** The MIN instruction ***/

MinimumInstruction::MinimumInstruction() : Instruction(oiMIN) {}

void MinimumInstruction::execute (InstructionProcessor &proc) const {
	Long n1, n2;
	n1 = proc.pop();
	n2 = proc.pop();
	if (n1 < n2)
		proc.push(n1);
	else
		proc.push(n2);
}

String MinimumInstruction::getName() const {
	return "Minimum";
}

/*** The ROUND instruction ***/

RoundInstruction::RoundInstruction(Byte colour) : Instruction(oiROUND + colour) {
	assert(colour<=2);
}

void RoundInstruction::execute (InstructionProcessor &proc) const {
	proc.push (proc.round (proc.compensateForColour (proc.popFixed(), instruction & 0x03)));
}

String RoundInstruction::getName() const {
	return "Round " + getParameterColour(instruction & 0x03);
}

/*** The NROUND instruction ***/

NoRoundInstruction::NoRoundInstruction(Byte colour) : Instruction(oiNROUND + colour) {
	assert(colour<=2);
}

void NoRoundInstruction::execute (InstructionProcessor &proc) const {
	proc.push (proc.compensateForColour (proc.popFixed(), instruction & 0x03));
}

String NoRoundInstruction::getName() const {
	return "No Round " + getParameterColour(instruction & 0x03);
}

/*** The FDEF instruction defines the begin of a function; executing it results in the 
	 function being defined. ***/

FunctionDefInstruction::FunctionDefInstruction() : Instruction(oiFDEF) {}

void FunctionDefInstruction::execute (InstructionProcessor &proc) const {
	Long id = proc.pop();
	if (id < 0 || id > 0xFFFF)
		throw InstructionException ("Function identifier " + String (id) + " invalid");

	// Find ENDF
	InstructionProcessor::InstructionIterator instruction;
	while (true) {
		instruction = proc.skipNextInstruction();
		if ((*instruction)->instruction == oiENDF) {
			proc.defineFunction (id);
			return;
		}
	}
}

String FunctionDefInstruction::getName() const {
	return "Function Definition";
}

/*** The ENDF instruction defines the end of a function ***/

EndFunctionDefInstruction::EndFunctionDefInstruction() : Instruction(oiENDF) {}

void EndFunctionDefInstruction::execute (InstructionProcessor &proc) const {
	proc.popFunctionCallStack();
}


String EndFunctionDefInstruction::getName() const {
	return "End Function Definition\n";
}

/*** The CALL instruction ***/

CallInstruction::CallInstruction() : Instruction(oiCALL) {}

void CallInstruction::execute (InstructionProcessor &proc) const {
	proc.callFunction(proc.pop());
}

String CallInstruction::getName() const {
	return "Call";
}

/*** The LOOPCALL instruction ***/

void LoopCallInstruction::execute (InstructionProcessor &proc) const {
	Long f, count;
	f = proc.pop();
	count = proc.pop();
	while (count) {
		proc.callFunction(f);
		count--;
	}
}

String LoopCallInstruction::getName() const {
	return "Loop Call";
}

/*** The IDEF instruction ***/

void InstructionDefInstruction::execute (InstructionProcessor &proc) const {
	//Default handler
	//Instruction::execute (proc);
	throw InstructionException ("The IDEF instruction is not supported");
}

String InstructionDefInstruction::getName() const {
	return "Instruction Definition";
}

/*** The GETINFO instruction ***/

GetInformationInstruction::GetInformationInstruction() : Instruction (oiGETINFO) {}

void GetInformationInstruction::execute (InstructionProcessor &proc) const {
	Long selector = proc.pop();
	Long result = 0;

	if (selector & oiInfoSelectVersion)
		result |= 37;//proc.getSimulatedVersion();
	// Ignore rotation and stretching here
	if ((selector & oiInfoSelectGreyscale) && proc.getGreyscale())
		result |= oiInfoResultGreyscale;
	// These may be returned by the MS rasteriser; I don't know what they mean.
/*	if (selector & 64)
		result |= 8192;
	if (selector & 128)
		result |= 16384;*/
/*	if (selector & 256)
		result |= 32768;
	if (selector & 512)
		result |= 65536;*/

	proc.push(result);
}

String GetInformationInstruction::getName() const {
	return "Get Information";
}

} // end namespace OpenType
