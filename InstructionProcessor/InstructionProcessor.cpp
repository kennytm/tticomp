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
	\file InstructionProcessor processes OpenType instructions.
*/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include "InstructionProcessor.h"
#include "Instructions.h"
#include "../OTFont/OTGlyph.h"
#include "../OTFont/OpenTypeFont.h"

using util::smart_ptr;
using util::String;
using util::fixed_fraction;

namespace OpenType {

/*** InstructionProcessor ***/

InstructionProcessor::InstructionProcessor () {
	assert (round (NewF26Dot6 (.5), NewF26Dot6 (1), NewF26Dot6 (0), NewF26Dot6 (.5)) == 1);
	assert (round (NewF26Dot6 (-.5), NewF26Dot6 (1), NewF26Dot6 (0), NewF26Dot6 (.5)) == -1);
	assert (round (NewF26Dot6 (2.5), NewF26Dot6 (1.5), NewF26Dot6 (.5), NewF26Dot6 (.75)) == 2);
}

InstructionProcessor::~InstructionProcessor() {}

void InstructionProcessor::setFont (smart_ptr <OpenTypeFont> aFont)
{
	font = aFont;
	unitsPerEm = font->getUnitsPerEm();

	Exception::FontContext c1 (*font);
	Exception::Context c2 ("initialising instruction processor");

	MemoryBlockPtr memory = font->getfpgm(false);
	fontProgram.programType = ptFontProgram;
	fontProgram.instructions = getInstructions (memory);
	
	// The cvt instructions may be already loaded, though they aren't executed yet.
	memory = font->getprep(false);
	cvtProgram.programType = ptCVTProgram;
	cvtProgram.instructions = getInstructions (memory);

	state = psNotActive;
}

void InstructionProcessor::setFont (const InstructionProcessor &aProc) {
	font = aProc.font;
	unitsPerEm = font->getUnitsPerEm();

	Exception::FontContext c1 (*font);
	Exception::Context c2 ("initialising instruction processor");

	fontProgram.programType = ptFontProgram;
	// Copying this makes a lot of difference in terms of memory use and
	// CPU time
	fontProgram.instructions = aProc.fontProgram.instructions;
	cvtProgram.programType = ptCVTProgram;
	cvtProgram.instructions = aProc.cvtProgram.instructions;

	state = psNotActive;
}

void InstructionProcessor::setPPEM (ULong aPPEMx, ULong aPPEMy, ULong aPointSize) {
	Exception::FontContext c1 (*font);
	Exception::Context c2 ("setting point sizes");

	ppemX = aPPEMx;
	ppemY = aPPEMy;
	pointSize = aPointSize;

	storage.clear();
	cvt.clear();
	functionDefinitions.clear();

	// Initialise storage
	StorageElement se = {0x0, false};
	storage.clear();
	storage.insert (storage.end(), font->getMaxStorage(), se);

	// Font program has to be executed first
	executeInstructions (psFontProgram);

	CVTEntry ce = {0, unitsPerEm, false, false, false};
	MemoryBlockPtr cvtMemory = font->getcvt(false);
	if (cvtMemory) {
		MemoryPen cvtPen (cvtMemory);
		while (!cvtPen.endOfBlock()) {
			ce.value = cvtPen.readShort();
			cvt.push_back (ce);
		}
	}

	try {
		executeInstructions (psCVTProgram);
	} catch (Exception & e) {
		// Now it's a pity there is no "finally" construct in C++
		// Set graphics state
		resetGraphicsState (false);
		defaultGraphicsState = currentGraphicsState;
		// And rethrow the exception
		throw e;
	}
	// Set graphics state
	resetGraphicsState (false);
	defaultGraphicsState = currentGraphicsState;
}

void InstructionProcessor::loadGlyph (GlyphPtr glyph) {
	points.clear();
	GridFittedPoint p;
	HorMetric hm = glyph->getHorMetric();
	Short displacement = - glyph->getDisplacement();

	Points bearings;

	/** Sidebearing points **/
	p.onCurve = true;
	p.touchedX = p.touchedY = false;
	p.lastContourPoint = true;
	// Set point n (left side bearing)
	p.currentX = p.originalX = 0;
	p.currentY = p.originalY = 0;
	bearings.push_back (p);

	// Set point n+1 (right side bearing)
	p.originalX = toF26Dot6x(hm.advanceWidth);
	p.currentX = p.originalX.round();
	p.currentY = p.originalY = 0;
	bearings.push_back (p);

	// Set point n+2 (upper bearing)
	p.currentX = p.originalX = 0;
	p.originalY = toF26Dot6y(font->getWinAscent());
	p.currentY = p.originalY.round();
	bearings.push_back (p);

	// Set point n+3 (lower bearing)
	p.currentX = p.originalX = 0;
	p.originalY = toF26Dot6y(-font->getWinDescent());
	p.currentY = p.originalY.round();
	bearings.push_back (p);

	if (!glyph->isComposite()) {
		Contours contours = glyph->getContours();
		points.reserve (contours.getPointNum() + 4);

		Contours::const_iterator contour;
		for (contour = contours.begin(); contour != contours.end(); contour ++) {
			Contour::const_iterator point = contour->begin();
			do {
				p.currentX = p.originalX = toF26Dot6x(point->x + displacement);
				p.currentY = p.originalY = toF26Dot6y(point->y);
				p.onCurve = point->onCurve;
				p.touchedX = p.touchedY = false;

				Contour::const_iterator nextPoint = point;
				nextPoint ++;
				p.lastContourPoint = (nextPoint == contour->begin());
				points.push_back (p);

				point ++;
			} while (point != contour->begin());
		}
	} else
	{
		Components components = glyph->getComponents();

		Points previousPoints;
		Components::iterator c;
		for (c = components.begin(); c != components.end(); c ++) {
			// Recursively load glyph
			Points componentPoints = getGlyphPoints ((*c)->getGlyphIndex());
			CompositeComponent::Scale scale = (*c)->getScale();

			Points::iterator cp;
			for (cp = componentPoints.begin(); cp != componentPoints.end(); cp ++) {
				NewF26Dot6 x = cp->currentX;
				NewF26Dot6 y = cp->currentY;
				cp->currentX = //OT_MULTIPLY_BY_F2DOT14_AND_ADD (x, scale.xx, y, scale.yx);
					scale.xx * x + scale.yx * y;
				cp->currentY = //OT_MULTIPLY_BY_F2DOT14_AND_ADD (x, scale.xy, y, scale.yy);
					scale.xy * x + scale.yy * y;
			}

			//CompositeComponent::Translation translation;
			struct Translation {
				NewF26Dot6 x, y;
			};
			Translation translation;
			UShort p1 = (*c)->getAttachmentPoint1();
			if (p1 == 0xFFFF) {
				// Positioned component
				CompositeComponent::Translation componentTranslation;
				componentTranslation = (*c)->getTranslation();
				translation.x = toF26Dot6x (componentTranslation.x + displacement);
				translation.y = toF26Dot6y (componentTranslation.y);
			} else {
				// Attached component
				if (p1 >= previousPoints.size())
					throw InstructionException ("Base attachment point index too large: " +
						String (p1));

				UShort p2 = (*c)->getAttachmentPoint2();
				if (p2 >= componentPoints.size())
					throw InstructionException ("Component attachment point index too large: " +
						String (p2));

				translation.x = previousPoints [p1].currentX - componentPoints [p2].currentX;
				translation.y = previousPoints [p1].currentY - componentPoints [p2].currentY;
			}

			if ((*c)->getFlags() & CompositeComponent::cfRoundXYToGrid) {
				translation.x = translation.x.round();
				translation.y = translation.y.round();
			}

			for (cp = componentPoints.begin(); cp != componentPoints.end() - 4; cp ++) {
				p.currentX = p.originalX = cp->currentX + translation.x;
				p.currentY = p.originalY = cp->currentY + translation.y;
				p.onCurve = cp->onCurve;
				p.touchedX = p.touchedY = false;
				p.lastContourPoint = cp->lastContourPoint;
				previousPoints.push_back (p);
			}

			if ((*c)->getFlags() & CompositeComponent::cfUseMyMetrics) {
				bearings.clear();
				bearings.insert (bearings.end(), componentPoints.end()-4, componentPoints.end());
			}
		}
		points = previousPoints;
	}

	points.insert (points.end(), bearings.begin(), bearings.end());

	glyphProgram.programType = ptGlyphProgram;

	// Execute glyph program only when the instruction execution flag allows it
	if (!(defaultGraphicsState.instructionControl & ieInhibitGridFitting)) {		
		// Load glyph instructions
		glyphProgram.instructions = getInstructions (glyph->getInstructions());
	} else {
		glyphProgram.instructions.clear();
	}
}

InstructionProcessor::Points InstructionProcessor::getGlyphPoints (UShort glyphId) {
	GlyphPtr glyph = font->getGlyph (glyphId);
	Exception::FontContext c1 (*font);
	Exception::Context c2 ("loading glyph " + String (glyphId) + " \'"
		+ glyph->getName() + "\'");
	loadGlyph (glyph);

	executeInstructions (psGlyphProgram);

	Points pts = points;
	points.clear();
	return pts;
}

void InstructionProcessor::resetGraphicsState(bool initially) {
	// Many values are set only at the start: the CVT program then sets the new default values
	if (initially) {
		currentGraphicsState.autoFlip = true;
		currentGraphicsState.deltaBase = 9;
		currentGraphicsState.deltaShift = 3;
	}

	currentGraphicsState.freedomVector.x = 1;
	currentGraphicsState.freedomVector.y = 0;
	currentGraphicsState.projectionVector = currentGraphicsState.freedomVector;
	currentGraphicsState.dualProjectionVector = currentGraphicsState.projectionVector;

	currentGraphicsState.loop = 1;
	if (initially) {
		currentGraphicsState.instructionControl = 0;
		currentGraphicsState.minimumDistance = 1;
	}

		// round to grid
	currentGraphicsState.roundPeriod = 1;
	currentGraphicsState.roundPhase = 0;
	currentGraphicsState.roundThreshold = NewF26Dot6 (.5);

	currentGraphicsState.rp[0] = currentGraphicsState.rp[1] = currentGraphicsState.rp[2] = 0;
	currentGraphicsState.zp[0] = currentGraphicsState.zp[1] = currentGraphicsState.zp[2] = 1;
	if (initially) {
		currentGraphicsState.controlValueCutIn = NewF26Dot6 (17)/16;
		currentGraphicsState.scanControl = false;
		currentGraphicsState.singleWidthCutIn = 0;
		currentGraphicsState.singleWidthValue = 0;
	}
}

void InstructionProcessor::setUnitVector(Vector &vector, F18Dot14 x, F18Dot14 y) {
	if (x*x + y*y == 1)
	{	// (x,y) is unit vector already
		vector.x = x;
		vector.y = y;
	} else {
		double size = sqrt(x*x + y*y);
		if (size==0) {
			throw InstructionException ("Cannot set vector to (0,0)");
			return;
		}
		vector.x = F2Dot14 (x / size);
		vector.y = F2Dot14 (y / size);
	}
}


void InstructionProcessor::executeInstructions (ProcessorState aState) {
	Exception::Context c ("executing " + stateToString (aState));

	state = aState;
	switch (state) {
	case psFontProgram:
		currentInstruction.stream = &fontProgram;
		resetGraphicsState (true);
		break;
	case psCVTProgram:
		currentInstruction.stream = &cvtProgram;
		resetGraphicsState (true);
		break;
	case psGlyphProgram:
		currentInstruction.stream = &glyphProgram;
		currentGraphicsState = defaultGraphicsState;
		break;
	}
	currentInstruction.position = currentInstruction.stream->instructions.begin();
	nextInstruction.stream = currentInstruction.stream;

	stack.clear();
	stack.reserve (font->getMaxStackElements());

	twilight.clear();
	GridFittedPoint p;// = {0,0, 0,0, true, false, false, true};
	p.currentX = p.currentY = p.originalX = p.originalY = 0;
	p.onCurve = true;
	p.touchedX = false;
	p.touchedY = false;
	p.lastContourPoint = true;
	twilight.insert (twilight.end(), font->getMaxTwilightPoints(), p);

	ULong instructionNum = 0;

	try {
		while (currentInstruction.position != currentInstruction.stream->instructions.end())
		{
			if (instructionNum > 100000)
				throw InstructionException ("More than 100000 instructions executed; possibly an endless loop");

			nextInstruction.position = currentInstruction.position + 1;
			(*currentInstruction.position)->execute (*this);

			currentInstruction = nextInstruction;

			instructionNum ++;
		}
	} catch (InstructionException &e) {
		e.setInstructionPosition (currentInstruction);
		throw e;
	}

	if (currentGraphicsState.loop != 1)
		addWarning (new InstructionException ("Loop variable " +
			String (currentGraphicsState.loop) + " after execution"));
	if (!callStack.empty())
		addWarning (new InstructionException ("Non-empty call stack after execution"));
	callStack.clear();
	if (!stack.empty())
		addWarning (new InstructionException ("Elements left on the stack after execution: "
			+ String (stack.size())));
	stack.clear();
	twilight.clear();

	state = psNotActive;
}

NewF26Dot6 InstructionProcessor::roundToGrid (NewF26Dot6 pos) {
	return pos.round();
}

NewF26Dot6 InstructionProcessor::round (NewF26Dot6 n, NewF26Dot6 period, NewF26Dot6 phase, NewF26Dot6 threshold) {
/*	NewF26Dot6 newN;
	if (n >= 0)
		newN = ((((ULong)n) - phase + threshold) / period) * period + phase;
	else
		newN = -((((ULong)-n) - phase + threshold) / period) * period + phase;
	
	return newN;*/
/*	if (n >= 0)
		return ((n - phase + threshold).get_i() / period.get_i()) * period + phase;
	else
		return -(((-n - phase + threshold).get_i() / period.get_i()) * period + phase);*/

/*	if (n >= 0) {
		NewF26Dot6 newN = ((n - phase + threshold).get_i() / period.get_i()) * period + phase;
		while (n > 0 && newN < 0)
			newN += period;
		return newN;
	} else {
		NewF26Dot6 newN = (((n - phase + threshold).get_i() - period.get_i() + 1) /
			period.get_i()) * period + phase;
		while (newN > 0)
			newN -= period;
		return newN;
	}*/

	/*** UNDOCUMENTED: strange semantics for negative values ***/

	bool neg = (n < 0);
	if (neg)
		n = -n;

	NewF26Dot6::int_type i = (n - phase + threshold).get_i();
	NewF26Dot6::int_type periodCorrection = 0;
	// i may be negative, so that truncating does not work correctly
	while (i < 0) {
		i += period.get_i();
		periodCorrection ++;
	}
	NewF26Dot6 newN = (i / period.get_i() - periodCorrection) * period + phase;
	if (n != 0) {
		while (newN < 0)
			newN += period;
	}
	if (neg)
		return -newN;
	else
		return newN;
}


InstructionProcessor::Instructions
InstructionProcessor::getInstructions (MemoryBlockPtr memory) {
	Exception::Context c ("reading instructions");

	if (!memory)
		return InstructionProcessor::Instructions ();
	else {
		InstructionProcessor::Instructions instructions;
		MemoryPen pen (memory);
		while (!pen.endOfBlock()) {
			instructions.push_back (getOneInstruction (pen));
		}
		return instructions;
	}
}

InstructionProcessor::InstructionPtr
InstructionProcessor::getOneInstruction (MemoryPen &pen) {
	// Automatically generated
	Byte instructionByte = (MemoryPen (pen).readByte());

	if (instructionByte==oiNPUSHB || instructionByte == oiNPUSHW ||
		(instructionByte >= oiPUSHB && instructionByte <= oiPUSHBend) ||
		(instructionByte >= oiPUSHW && instructionByte <= oiPUSHWend))
		return new PushInstruction (pen);

	if (instructionByte >= oiROUND && instructionByte <= oiROUNDend)
		return new RoundInstruction (pen);

	if (instructionByte >= oiMDRP && instructionByte <= oiMDRPend)
		return new MoveDirectRelativePointInstruction(pen);
	if (instructionByte >= oiMIRP && instructionByte <= oiMIRPend)
		return new MoveIndirectRelativePointInstruction(pen);

	switch (instructionByte) {
	case oiRS:
		return new ReadStoreInstruction(pen);
	case oiWS:
		return new WriteStoreInstruction(pen);
	case oiWCVTP:
		return new WriteCVTPixelsInstruction(pen);
	case oiWCVTF:
		return new WriteCVTFUnitsInstruction(pen);
	case oiRCVT:
		return new ReadCVTInstruction(pen);

	case oiSVTCAy: case oiSVTCAx:
		return new VectorsToAxisInstruction(pen);
	case oiSPVTCAy: case oiSPVTCAx:
		return new ProjectionToAxisInstruction(pen);
	case oiSFVTCAy: case oiSFVTCAx:
		return new FreedomToAxisInstruction(pen);
	case oiSPVTL: case oiSPVTLperp:
		return new ProjectionToLineInstruction(pen);
	case oiSFVTL: case oiSFVTLperp:
		return new FreedomToLineInstruction(pen);
	case oiSFVTPV:
		return new FreedomToProjectionInstruction(pen);
	case oiSDPVTL: case oiSDPVTLperp:
		return new DualProjectionToLineInstruction(pen);
	case oiSPVFS:
		return new ProjectionFromStackInstruction(pen);
	case oiSFVFS:
		return new FreedomFromStackInstruction(pen);
	case oiGPV:
		return new GetProjectionVectorInstruction(pen);
	case oiGFV:
		return new GetFreedomVectorInstruction(pen);
	case oiSRP0: case oiSRP1: case oiSRP2:
		return new SetReferencePointInstruction(pen);
	case oiSZP0: case oiSZP1: case oiSZP2:
		return new SetZonePointerInstruction(pen);
	case oiSZPS:
		return new SetZonePointersInstruction(pen);

	case oiRTHG:
		return new RoundToHalfGridInstruction(pen);
	case oiRTG:
		return new RoundToGridInstruction(pen);
	case oiRTDG:
		return new RoundToDoubleGridInstruction(pen);
	case oiRDTG:
		return new RoundDownToGridInstruction(pen);
	case oiRUTG:
		return new RoundUpToGridInstruction(pen);
	case oiROFF:
		return new RoundOffInstruction(pen);
	case oiSROUND:
		return new SuperRoundInstruction(pen);
	case oiS45ROUND:
		return new SuperRound45Instruction(pen);

	case oiSLOOP:
		return new SetLoopInstruction(pen);
	case oiSMD:
		return new SetMinimumDistanceInstruction(pen);
	case oiINSTCTRL:
		return new InstructionControlInstruction(pen);
	case oiSCANCTRL:
		return new ScanConversionControlInstruction(pen);
	case oiSCANTYPE:
		return new ScanTypeInstruction(pen);
	case oiSCVTCI:
		return new ControlValueCutInInstruction(pen);
	case oiSSWCI:
		return new SetSingleWidthCutInInstruction(pen);
	case oiSSW:
		return new SetSingleWidthInstruction(pen);
	case oiFLIPON: case oiFLIPOFF:
		return new SetAutoFlipInstruction(pen);
	case oiSDB:
		return new DeltaBaseInstruction(pen);
	case oiSDS:
		return new DeltaShiftInstruction(pen);
	case oiGCcur: case oiGCorig:
		return new GetCoordinateInstruction(pen);
	case oiSCFS:
		return new SetCoordinateInstruction(pen);

	case oiMDcur: case oiMDorig:
		return new MeasureDistanceInstruction(pen);
	case oiMPPEM:
		return new MeasurePPEMInstruction(pen);
	case oiMPS:
		return new MeasurePointSizeInstruction(pen);

	case oiFLIPPT:
		return new FlipPointInstruction(pen);
	case oiFLIPRGON: case oiFLIPRGOFF:
		return new FlipRangeInstruction(pen);
	case oiSHP21: case oiSHP10:
		return new ShiftPointInstruction(pen);
	case oiSHC21: case oiSHC10:
		return new ShiftContourInstruction(pen);
	case oiSHZ21: case oiSHZ10:
		return new ShiftZoneInstruction(pen);
	case oiSHPIX:
		return new ShiftPointByPixelsInstruction(pen);
	case oiMSIRP: case oiMSIRPset:
		return new MoveStackIndirectRelativePointInstruction(pen);
	case oiMDAP: case oiMDAPround:
		return new MoveDirectAbsolutePointInstruction(pen);
	case oiMIAP: case oiMIAPround:
		return new MoveIndirectAbsolutePointInstruction(pen);
	case oiALIGN:
		return new AlignInstruction(pen);
	case oiISECT:
		return new MoveToIntersectionInstruction(pen);
	case oiALIGNPTS:
		return new AlignPointsInstruction(pen);
	case oiIP:
		return new InterpolatePointInstruction(pen);
	case oiUTP:
		return new UntouchPointInstruction(pen);
	case oiIUPy: case oiIUPx:
		return new InterpolateUntouchedPointsInstruction(pen);
	case oiDELTAP1: case oiDELTAP2: case oiDELTAP3:
		return new DeltaPInstruction(pen);
	case oiDELTAC1: case oiDELTAC2: case oiDELTAC3:
		return new DeltaCInstruction(pen);
	case oiDUP:
		return new DuplicateStackElementInstruction(pen);
	case oiPOP:
		return new PopInstruction(pen);
	case oiCLEAR:
		return new ClearStackInstruction(pen);
	case oiSWAP:
		return new SwapStackElementsInstruction(pen);
	case oiDEPTH:
		return new StackDepthInstruction(pen);
	case oiCINDEX:
		return new CopyIndexInstruction(pen);
	case oiMINDEX:
		return new MoveIndexInstruction(pen);
	case oiROLL:
		return new RollStackInstruction(pen);
	case oiIF:
		return new IfInstruction(pen);
	case oiELSE:
		return new ElseInstruction(pen);
	case oiEIF:
		return new EndIfInstruction(pen);
	case oiJR:
		return new JumpRelOnTrueInstruction(pen);
	case oiJMP:
		return new JumpInstruction(pen);
	case oiJROF:
		return new JumpRelOnFalseInstruction(pen);
	case oiLT:
		return new LessThanInstruction(pen);
	case oiLTEQ:
		return new LessThanOrEqualInstruction(pen);
	case oiGT:
		return new GreaterThanInstruction(pen);
	case oiGTEQ:
		return new GreaterThanOrEqualInstruction(pen);
	case oiEQ:
		return new EqualInstruction(pen);
	case oiNEQ:
		return new NotEqualInstruction(pen);
	case oiODD:
		return new OddInstruction(pen);
	case oiEVEN:
		return new EvenInstruction(pen);
	case oiAND:
		return new AndInstruction(pen);
	case oiOR:
		return new OrInstruction(pen);
	case oiN:
		return new NotInstruction(pen);
	case oiADD:
		return new AddInstruction(pen);
	case oiSUB:
		return new SubtractInstruction(pen);
	case oiDIV:
		return new DivideInstruction(pen);
	case oiMUL:
		return new MultiplyInstruction(pen);
	case oiABS:
		return new AbsoluteInstruction(pen);
	case oiNEG:
		return new NegateInstruction(pen);
	case oiFLOOR:
		return new FloorInstruction(pen);
	case oiCEILING:
		return new CeilingInstruction(pen);
	case oiMAX:
		return new MaximumInstruction(pen);
	case oiMIN:
		return new MinimumInstruction(pen);
	case oiNROUND:
		return new NoRoundInstruction(pen);
	case oiFDEF:
		return new FunctionDefInstruction(pen);
	case oiENDF:
		return new EndFunctionDefInstruction(pen);
	case oiCALL:
		return new CallInstruction(pen);
	case oiLOOPCALL:
		return new LoopCallInstruction(pen);
	case oiIDEF:
		return new InstructionDefInstruction(pen);
	case oiGETINFO:
		return new GetInformationInstruction(pen);
	default:
		throw InstructionException ("Unknown instruction " +
			String (instructionByte));
	}
}


/*** InstructionProcessor methods for instructions ***/

String InstructionProcessor::positionToString (const InstructionPosition &position) {
	String pos;
	switch (position.stream->programType) {
	case ptFontProgram:
		pos = "Font program";
		break;
	case ptCVTProgram:
		pos = "CVT program";
		break;
	case ptGlyphProgram:
		pos = "Glyph program";
		break;
	default:
		assert (false);
	}
	if (position.position != position.stream->instructions.end())
		return pos + ": " + String ((*position.position)->getOffset());
	else
		return pos;
}

String InstructionProcessor::stateToString(ProcessorState s) {
	switch (s) {
	case psFontProgram:
		return "font program";
	case psCVTProgram:
		return "cvt program";
	case psGlyphProgram:
		return "glyph program";
	default:
		assert (false);
		return String();
	}
}

ULong InstructionProcessor::getPointSize() {
	if (state != psCVTProgram && state != psGlyphProgram)
		throw InstructionException ("Point size only known in cvt program and glyph program");
	return pointSize;
}

ULong InstructionProcessor::getPPEM() {
	if (state != psCVTProgram && state != psGlyphProgram)
		throw InstructionException ("Point size only known in cvt program and glyph program");
	return weightedAverage(F18Dot14 (ppemX), F18Dot14 (ppemY),
		currentGraphicsState.projectionVector).round();
}


bool InstructionProcessor::getGreyscale() {
	return true;
}

ULong InstructionProcessor::getPointNum() {
	if (state != psGlyphProgram)
		throw InstructionException ("Contour points are valid in the glyph program only");
	return points.size();
}

InstructionProcessor::InstructionIterator
InstructionProcessor::skipNextInstruction() {
	if (nextInstruction.position == nextInstruction.stream->instructions.end())
		throw InstructionException ("Searching beyond instruction stream");
	
	return nextInstruction.position ++;
}

void InstructionProcessor::jumpTo (ULong offset) {
	Instructions &instructions = currentInstruction.stream->instructions;
	if ((*(instructions.end() -1))->getOffset() + (*(instructions.end() -1))->getByteSize() == offset)
		nextInstruction.position = instructions.end();
	else {
		if (instructions.begin() == instructions.end() ||
			(*instructions.begin())->getOffset() > offset ||
			(*(instructions.end() - 1))->getOffset() < offset)
			throw InstructionException ("Cannot jump outside of instruction stream");
		Instructions::iterator lower, upper, guess;
		if (offset < (*currentInstruction.position)->getOffset()) {
			lower = instructions.begin();
			upper = currentInstruction.position;
		} else {
			lower = currentInstruction.position;
			upper = instructions.end();
		}

		while (lower != upper) {
			guess = lower + (upper-lower)/2;
			if ((*guess)->getOffset() < offset)
				lower = guess + 1;
			else {
				if (offset < (*guess)->getOffset())
					upper = guess;
				else {
					nextInstruction.position = guess;
					return;
				}
			}
		}

		throw InstructionException ("Jump offset not found");
	}
}

InstructionProcessor::FunctionDefinitionIterator
InstructionProcessor::getFunction (ULong id) {
	FunctionDefinitionIterator lower, upper, guess;
	lower = functionDefinitions.begin();
	upper = functionDefinitions.end();
	guess = lower + (upper - lower) / 2;
	while (lower != upper) {
		guess = lower + (upper - lower) / 2;
		if (id < guess->id)
			upper = guess;
		else {
			if (guess->id < id)
				lower = guess + 1;
			else
				return guess;
		}
	}
	return guess;
}

// Sets the function to (currentInstruction, nextInstruction)
void InstructionProcessor::defineFunction (ULong id) {
	FunctionDefinitionIterator pos = getFunction (id);
	if (pos != functionDefinitions.end() && pos->id == id)
		throw InstructionException ("Function " + String (id) +
			" has already been defined");

	FunctionDefinition fd;
	fd.id = id;
	fd.stream.programType = currentInstruction.stream->programType;
	fd.stream.instructions = Instructions (currentInstruction.position + 1, nextInstruction.position);
	functionDefinitions.push_back (fd);

	if (functionDefinitions.size() > font->getMaxFunctionDefs())
		addWarning (new InstructionException ("Too many function definitions"));
}

void InstructionProcessor::callFunction (ULong id) {
	FunctionDefinitionIterator pos = getFunction (id);
	if (pos == functionDefinitions.end() || pos->id != id)
		throw InstructionException ("Undefined function " +
			String (id));
	// Save position
	callStack.push_back (nextInstruction);

	// Set new position
	nextInstruction.stream = &pos->stream;
	nextInstruction.position = pos->stream.instructions.begin();
}

void InstructionProcessor::popFunctionCallStack() {
	if (callStack.empty())
		throw InstructionException ("Function call stack empty");
	nextInstruction = callStack.back();
	callStack.pop_back();
}

void InstructionProcessor::push (Long element) {
	stack.push_back (element);
	if (stack.size() > font->getMaxStackElements())
		addWarning (new InstructionException ("Too many stack elements: " +
			String (stack.size())));
}

void InstructionProcessor::push (const std::vector <Short> &elements) {
	PushInstruction::Elements::const_iterator i;
	for (i = elements.begin(); i != elements.end(); i ++)
		push (*i);
}

Long InstructionProcessor::pop() {
	if (stack.empty())
		throw InstructionException ("Stack empty");
	Long e = stack.back();
	stack.pop_back();
	return e;
}

Long InstructionProcessor::getNthStackElement (ULong indexFromLast) {
	if (stack.size() <= indexFromLast)
		throw InstructionException ("Cannot get " + String (indexFromLast)
			+ "th stack element: stack is not deep enough");
	return stack.at (stack.size() - 1 - indexFromLast);
}

Long InstructionProcessor::removeNthStackElement (ULong indexFromLast) {
	if ( stack.size() <= indexFromLast)
		throw InstructionException ("Cannot get " + String (indexFromLast)
			+ "th stack element: stack is not deep enough");
	Long value = stack.at (stack.size() - 1 - indexFromLast);
	stack.erase (stack.end() - 1 - indexFromLast);
	return value;
}

void InstructionProcessor::clearStack() {
	stack.clear();
}

ULong InstructionProcessor::getStackElementNum() {
	return stack.size();
}

void InstructionProcessor::setStorage (ULong location, Long value) {
	if (location < 0 || location >= storage.size())
		throw InstructionException ("Storage index " + String (location) +
			" out of range");
	storage [location].initialised = true;
	storage [location].n = value;
}

Long InstructionProcessor::getStorage (ULong location) {
	if (location < 0 || location >= storage.size())
		throw InstructionException ("Storage index " + String (location) +
			" out of range");
	if (!storage [location].initialised)
		throw InstructionException ("Storage entry " + String (location) +
			" not initialised");
	return storage [location].n;
}

void InstructionProcessor::setLoop(ULong aLoop) {
	currentGraphicsState.loop = aLoop;
}

ULong InstructionProcessor::getLoop() {
	return currentGraphicsState.loop;
}

void InstructionProcessor::setInstructionExecutionControl(ULong mask, ULong value) {
	currentGraphicsState.instructionControl =
		(currentGraphicsState.instructionControl & (~mask)) | (value & mask);
}


void InstructionProcessor::setFreedomVector (F18Dot14 x, F18Dot14 y) {
	assert(state != psNotActive);

	setUnitVector(currentGraphicsState.freedomVector, x, y);
}

void InstructionProcessor::setProjectionVector (F18Dot14 x, F18Dot14 y) {
	assert(state != psNotActive);

	setUnitVector(currentGraphicsState.projectionVector, x, y);
	setUnitVector(currentGraphicsState.dualProjectionVector, x, y);
}

void InstructionProcessor::setDualProjectionVector (F18Dot14 x, F18Dot14 y) {
	assert(state != psNotActive);

	setUnitVector(currentGraphicsState.dualProjectionVector, x, y);
}

InstructionProcessor::Vector InstructionProcessor::getFreedomVector() {
	return currentGraphicsState.freedomVector;
}

InstructionProcessor::Vector InstructionProcessor::getProjectionVector() {
	return currentGraphicsState.projectionVector;
}

void InstructionProcessor::setDeltaBase(ULong aDeltaBase) {
	currentGraphicsState.deltaBase = aDeltaBase;
}

ULong InstructionProcessor::getDeltaBase() {
	return currentGraphicsState.deltaBase;
}

void InstructionProcessor::setDeltaShift(ULong aDeltaShift) {
	currentGraphicsState.deltaShift = aDeltaShift;
}

ULong InstructionProcessor::getDeltaShift() {
	return currentGraphicsState.deltaShift;
}

void InstructionProcessor::setRoundingState(NewF26Dot6 aPeriod, NewF26Dot6 aPhase, NewF26Dot6 aThreshold) {
	assert(state != psNotActive);

	currentGraphicsState.roundPeriod = aPeriod;
	currentGraphicsState.roundPhase = aPhase;
	currentGraphicsState.roundThreshold = aThreshold;
}

NewF26Dot6 InstructionProcessor::round (NewF26Dot6 n) {
	assert(state != psNotActive);

	return round(n, currentGraphicsState.roundPeriod,
		currentGraphicsState.roundPhase, currentGraphicsState.roundThreshold);
}

void InstructionProcessor::setZonePointer(Byte index, Long aZone) {
	assert(state != psNotActive);
	if (index>2)
		throw InstructionException ("Invalid zonepointer index " +
			String (index));

	if (aZone!=0 && aZone!=1)
		throw InstructionException ("Invalid zone index " +
			String (index));


	currentGraphicsState.zp[index] = (Byte) aZone;
}

Long InstructionProcessor::getZonePointer(Byte index) {
	assert(state != psNotActive);

	if (index>2)
		throw InstructionException ("Invalid zonepointer index " +
			String (index));

	return currentGraphicsState.zp[index];
}

void InstructionProcessor::setReferencePoint(Byte index, Long aPoint) {
	assert(state != psNotActive);
	if (index>2)
		throw InstructionException ("Invalid reference pointer index " +
			String (index));

	currentGraphicsState.rp[index] = aPoint;
}

Long InstructionProcessor::getReferencePoint(Byte index) {
	assert(state != psNotActive);

	if (index>2)
		throw InstructionException ("Invalid reference pointer index " +
			String (index));

	return currentGraphicsState.rp[index];
}

NewF26Dot6 InstructionProcessor::getPoint(ULong zone, ULong index) {
	assert(state != psNotActive);

	if (zone==0)
	{	// Twilight zone
		if (index < 0 || index >= twilight.size())
			throw InstructionException ("Invalid twilight zone point index " + 
				String(index));

		return projectOnto(twilight[index].currentX, twilight[index].currentY,
			currentGraphicsState.projectionVector);
	} else
	{	// Normal zone
		if (index < 0 || index >= points.size())
			throw InstructionException ("Invalid contour point index " + 
				String(index));

		return projectOnto(points[index].currentX, points[index].currentY,
			currentGraphicsState.projectionVector);
	}
}

NewF26Dot6 InstructionProcessor::getOriginalPoint(ULong zone, ULong index, bool useDual) {
	assert(state != psNotActive);

	Vector dual;
	if (useDual) 
		dual = currentGraphicsState.dualProjectionVector;
	else
		dual = currentGraphicsState.projectionVector;

	if (zone==0)
	{	// Twilight zone
		if (index < 0 || index >= twilight.size())
			throw InstructionException ("Invalid twilight zone point index " + 
				String(index));

		return projectOnto(twilight[index].originalX, twilight[index].originalY, dual);
	} else
	{	// Normal zone
		if (index < 0 || index >= points.size())
			throw InstructionException ("Invalid contour point index " + 
				String(index));

		return projectOnto(points[index].originalX, points[index].originalY, dual);
	}
}

void InstructionProcessor::unTouchPoint(ULong index) {
	assert(state != psNotActive);

	if (state!=psGlyphProgram)
		throw InstructionException ("Points may only be untouched in the glyph program");

	if (currentGraphicsState.freedomVector.x != 0)
		points [index].touchedX = false;
	if (currentGraphicsState.freedomVector.y != 0)
		points [index].touchedY = false;
}

ULong InstructionProcessor::getTwilightPointNum() {
	assert(state != psNotActive);

	return twilight.size();
}

ULong InstructionProcessor::getLastContourPoint(ULong contour) {
	assert(state != psNotActive);

	Points::iterator p;
	for (p = points.begin(); p != points.end(); p ++) {
		if (p->lastContourPoint) {
			if (contour == 0)
				return p - points.begin();
			contour --;
		}
	}

	throw InstructionException ("Illegal contour index " + String (contour));
}

void InstructionProcessor::movePoint(ULong zone, ULong index, NewF26Dot6 newPos) {
	assert(state != psNotActive);

	if ((currentGraphicsState.projectionVector.x * currentGraphicsState.freedomVector.x + 
		currentGraphicsState.projectionVector.y * currentGraphicsState.freedomVector.y) == 0)
		throw InstructionException (
			"Projection and freedom vectors may not be orthogonal while moving points");

	if (zone==0)
	{	// Twilight zone
		if (index < 0 || index >= twilight.size())
			throw InstructionException ("Invalid twilight zone point index " + 
				String(index));

		NewF26Dot6 moveByProj =  newPos -
			projectOnto(twilight [index].currentX, twilight [index].currentY,
			currentGraphicsState.projectionVector);
		NewF26Dot6 moveByFreedom = moveByProj /
			(currentGraphicsState.projectionVector.x * currentGraphicsState.freedomVector.x +
			currentGraphicsState.projectionVector.y * currentGraphicsState.freedomVector.y);

		twilight [index].currentX += moveByFreedom * currentGraphicsState.freedomVector.x;
		twilight [index].currentY += moveByFreedom * currentGraphicsState.freedomVector.y;
		// Touch point
		if (currentGraphicsState.freedomVector.x != 0)
			twilight[index].touchedX = true;
		if (currentGraphicsState.freedomVector.y != 0)
			twilight[index].touchedY = true;
	} else
	{	// Normal zone
		if (index < 0 || index >= points.size())
			throw InstructionException ("Invalid contour point index " + 
				String(index));

		NewF26Dot6 moveByProj =  newPos -
			projectOnto(points [index].currentX, points [index].currentY,
			currentGraphicsState.projectionVector);
		NewF26Dot6 moveByFreedom = moveByProj /
			(currentGraphicsState.projectionVector.x * currentGraphicsState.freedomVector.x +
			currentGraphicsState.projectionVector.y * currentGraphicsState.freedomVector.y);

		points [index].currentX += moveByFreedom * currentGraphicsState.freedomVector.x;
		points [index].currentY += moveByFreedom * currentGraphicsState.freedomVector.y;
		// Touch point
		if (currentGraphicsState.freedomVector.x != 0)
			points[index].touchedX = true;
		if (currentGraphicsState.freedomVector.y != 0)
			points[index].touchedY = true;
	}
}

void InstructionProcessor::moveOriginalPointToXY (ULong zone, ULong index, NewF26Dot6 newX, NewF26Dot6 newY)
{
	// This strange operation is only used on twilight points.
	assert (state != psNotActive);
	assert (zone == 0);

	if (index < 0 || index >= twilight.size())
		throw InstructionException ("Invalid twilight zone point index " + 
			String(index));

	twilight [index].originalX = newX;
	twilight [index].originalY = newY;
}

void InstructionProcessor::moveOriginalPoint(ULong zone, ULong index, NewF26Dot6 newPos) {
	// This extremely strange instruction is only used on twilight points.
	// See MIAP.
	assert (state != psNotActive);
	assert (zone == 0);

	if ((currentGraphicsState.projectionVector.x * currentGraphicsState.freedomVector.x + 
		currentGraphicsState.projectionVector.y * currentGraphicsState.freedomVector.y) == 0)
		throw InstructionException (
			"Projection and freedom vectors may not be orthogonal while moving points");

	if (index < 0 || index >= twilight.size())
		throw InstructionException ("Invalid twilight zone point index " + 
			String(index));

	NewF26Dot6 moveByProj =  newPos -
		projectOnto(twilight [index].originalX, twilight [index].originalY,
		currentGraphicsState.projectionVector);
	NewF26Dot6 moveByFreedom = moveByProj /
		(currentGraphicsState.projectionVector.x * currentGraphicsState.freedomVector.x +
		currentGraphicsState.projectionVector.y * currentGraphicsState.freedomVector.y);

	twilight [index].originalX += moveByFreedom * currentGraphicsState.freedomVector.x;
	twilight [index].originalY += moveByFreedom * currentGraphicsState.freedomVector.y;
}

void InstructionProcessor::shiftPoint(ULong zone, ULong index, NewF26Dot6 amount, bool touch) {
	assert(state != psNotActive);

	if (zone==0)
	{	// Twilight zone
		if (index < 0 || index >= twilight.size())
			throw InstructionException ("Invalid twilight zone point index " + 
				String(index));

		twilight [index].currentX += amount * currentGraphicsState.freedomVector.x;
		twilight [index].currentY += amount * currentGraphicsState.freedomVector.y;

		if (touch) {
			// Touch point
			if (currentGraphicsState.freedomVector.x != 0)
				twilight [index].touchedX = true;
			if (currentGraphicsState.freedomVector.y != 0)
				twilight [index].touchedY = true;
		}
	} else
	{	// Normal zone
		if (index < 0 || index >= points.size())
			throw InstructionException ("Invalid contour point index " + 
				String(index));

		points [index].currentX += amount * currentGraphicsState.freedomVector.x;
		points [index].currentY += amount * currentGraphicsState.freedomVector.y;

		if (touch) {
			// Touch point
			if (currentGraphicsState.freedomVector.x != 0)
				points[index].touchedX = true;
			if (currentGraphicsState.freedomVector.y != 0)
				points[index].touchedY = true;
		}
	}
}

void InstructionProcessor::movePointToXY(ULong zone, ULong index, NewF26Dot6 newX, NewF26Dot6 newY) {
	assert(state != psNotActive);

	if (zone==0)
	{	// Twilight zone
		if (index < 0 || index >= twilight.size())
			throw InstructionException ("Invalid twilight zone point index " + 
				String(index));

		twilight [index].currentX = newX;
		twilight [index].currentY = newY;
		// Touch point
		twilight [index].touchedX = true;
		twilight [index].touchedY = true;
	} else
	{	// Normal zone
		if (index < 0 || index >= points.size())
			throw InstructionException ("Invalid contour point index " + 
				String(index));

		points [index].currentX = newX;
		points [index].currentY = newY;
		// Touch point
		points [index].touchedX = true;
		points [index].touchedY = true;
	}
}

void InstructionProcessor::setOnCurve(ULong index, bool aOnCurve) {
	assert(state != psNotActive);

	if (state != psGlyphProgram)
		throw InstructionException (
			"Flipping the on curve state of points is possible only in the glyph program");

	if (index > points.size())
		throw InstructionException (
			"Point index " + String (index) + " too high when flipping on curve state");

	points [index].onCurve = aOnCurve;
}

bool InstructionProcessor::getOnCurve(ULong index) {
	assert(state != psNotActive);

	if (state != psGlyphProgram)
		throw InstructionException (
			"Querying the on curve state of points is possible only in the glyph program");
	
	if (index > points.size())
		throw InstructionException (
			"Point index " + String (index) + " too high when looking at on curve state");

	return points [index].onCurve;
}

NewF26Dot6 InstructionProcessor::getPointX(ULong zone, ULong index) {
	assert(state != psNotActive);

	if (zone==0)
	{	// Twilight zone
		if (index < 0 || index >= twilight.size())
			throw InstructionException ("Invalid twilight zone point index " + 
				String(index));

		return twilight [index].currentX;
	} else
	{	// Normal zone
		if (index < 0 || index >= points.size())
			throw InstructionException ("Invalid contour point index " + 
				String(index));

		return points [index].currentX;
	}
}

NewF26Dot6 InstructionProcessor::getPointY(ULong zone, ULong index) {
	assert(state != psNotActive);

	if (zone==0)
	{	// Twilight zone
		if (index < 0 || index >= twilight.size())
			throw InstructionException ("Invalid twilight zone point index " + 
				String(index));

		return twilight [index].currentY;
	} else
	{	// Normal zone
		if (index < 0 || index >= points.size())
			throw InstructionException ("Invalid contour point index " + 
				String(index));

		return points [index].currentY;
	}
}


NewF26Dot6 InstructionProcessor::getOriginalPointX(ULong zone, ULong index) {
	assert(state != psNotActive);

	if (zone==0)
	{	// Twilight zone
		if (index < 0 || index >= twilight.size())
			throw InstructionException ("Invalid twilight zone point index " + 
				String(index));

		return twilight [index].originalX;
	} else
	{	// Normal zone
		if (index < 0 || index >= points.size())
			throw InstructionException ("Invalid contour point index " + 
				String(index));

		return points[index].originalX;
	}
}

NewF26Dot6 InstructionProcessor::getOriginalPointY(ULong zone, ULong index) {
	assert(state != psNotActive);

	if (zone==0)
	{	// Twilight zone
		if (index < 0 || index >= twilight.size())
			throw InstructionException ("Invalid twilight zone point index " + 
				String(index));

		return twilight [index].originalY;
	} else
	{	// Normal zone
		if (index < 0 || index >= points.size())
			throw InstructionException ("Invalid contour point index " + 
				String(index));

		return points [index].originalY;
	}
}


/***********

  Interpolating points in TrueType works thus:

  Between two touched points x1 and x2 with x1<=x2 the untouched points are
  interpolated:
  1. if xp <= x1 then move xp the amount x1 has.
  2. else if xp >= x2 then move xp the amount x2 has.
  3. else (if x1 < xp < x2) then move xp so that (x1-xp)/(x2-xp) remains the same.

  *********/

void InstructionProcessor::interpolatePointsX() {
	assert(state != psNotActive);

	if (state!=psGlyphProgram)
		throw InstructionException (
			"Points can only be interpolated within the glyph program");

	Points::iterator i;

	Points::iterator firstContourPoint;
	Points::iterator lastContourPoint;

	for (firstContourPoint = points.begin(); firstContourPoint != points.end();
	firstContourPoint = lastContourPoint + 1) {
		lastContourPoint = firstContourPoint;
		while (!lastContourPoint->lastContourPoint)
			lastContourPoint ++;

		i = firstContourPoint;
		while (i <= lastContourPoint && !i->touchedX)
			i++;
		if (i <= lastContourPoint) {
			Points::iterator first = i;
			do {
				Points::iterator second = first;
				// Find next touched pair
				do {
					second ++;
					if (second > lastContourPoint) {
						second = i;
						break;
					}
				} while (!second->touchedX);

				// Now "first" and "second" points are touched points with inbetween
				// untouched points in between them

				if (first + 1 != second && (first != lastContourPoint || firstContourPoint != second)) {
					Points::iterator lowest, highest;
					if (first->originalX <= second->originalX) {
						lowest = first;
						highest = second;
					} else {
						highest = first;
						lowest = second;
					}

					NewF26Dot6 lowestShift = lowest->currentX - lowest->originalX;
					NewF26Dot6 highestShift = highest->currentX - highest->originalX;
					NewF26Dot6 divisor = highest->originalX - lowest->originalX;

					Points::iterator k;
					if (first == lastContourPoint)
						k = firstContourPoint;
					else
						k = first + 1;
					while (k != second) {
						if (k->originalX <= lowest->originalX)
							k->currentX = k->originalX + lowestShift;
						else {
							if (k->originalX >= highest->originalX)
								k->currentX = k->originalX + highestShift;
							else {
								k->currentX = k->originalX + 
									(lowestShift * (highest->originalX - k->originalX) +
									highestShift * (k->originalX - lowest->originalX))/divisor;
							}
						}
						if (k == lastContourPoint)
							k = firstContourPoint;
						else
							k ++;
					}
				}
				
				first = second;
			} while (first != i);
		}
	}
}

// Same as above, except for having replaced "X" by "Y" throughout
// Except for InstructionEyception of course

void InstructionProcessor::interpolatePointsY() {
	assert(state != psNotActive);

	if (state!=psGlyphProgram)
		throw InstructionException (
			"Points can only be interpolated within the glyph program");

	Points::iterator i;

	Points::iterator firstContourPoint;
	Points::iterator lastContourPoint;

	for (firstContourPoint = points.begin(); firstContourPoint != points.end();
	firstContourPoint = lastContourPoint + 1) {
		lastContourPoint = firstContourPoint;
		while (!lastContourPoint->lastContourPoint)
			lastContourPoint ++;

		i = firstContourPoint;
		while (i <= lastContourPoint && !i->touchedY)
			i++;
		if (i <= lastContourPoint) {
			Points::iterator first = i;
			do {
				Points::iterator second = first;
				// Find neyt touched pair
				do {
					second ++;
					if (second > lastContourPoint) {
						second = i;
						break;
					}
				} while (!second->touchedY);

				// Now "first" and "second" points are touched points with inbetween
				// untouched points in between them

				if (first + 1 != second && (first != lastContourPoint || firstContourPoint != second)) {
					Points::iterator lowest, highest;
					if (first->originalY <= second->originalY) {
						lowest = first;
						highest = second;
					} else {
						highest = first;
						lowest = second;
					}

					NewF26Dot6 lowestShift = lowest->currentY - lowest->originalY;
					NewF26Dot6 highestShift = highest->currentY - highest->originalY;
					NewF26Dot6 divisor = highest->originalY - lowest->originalY;

					Points::iterator k;
					if (first == lastContourPoint)
						k = firstContourPoint;
					else
						k = first + 1;
					while (k != second) {
						if (k->originalY <= lowest->originalY)
							k->currentY = k->originalY + lowestShift;
						else {
							if (k->originalY >= highest->originalY)
								k->currentY = k->originalY + highestShift;
							else {
								k->currentY = k->originalY + 
									(lowestShift * (highest->originalY - k->originalY) +
									highestShift * (k->originalY - lowest->originalY))/divisor;
							}
						}
						if (k == lastContourPoint)
							k = firstContourPoint;
						else
							k ++;
					}
				}
				
				first = second;
			} while (first != i);
		}
	}
}


void InstructionProcessor::setControlValueCutIn (NewF26Dot6 n) {
	assert(state != psNotActive);
	currentGraphicsState.controlValueCutIn = n;
}

NewF26Dot6 InstructionProcessor::getControlValueCutIn() {
	assert(state != psNotActive);
	return currentGraphicsState.controlValueCutIn;
}

void InstructionProcessor::setMinimumDistance (NewF26Dot6 aMinDist) {
	assert(state != psNotActive);
	currentGraphicsState.minimumDistance = aMinDist;
}

NewF26Dot6 InstructionProcessor::getMinimumDistance() {
	assert(state != psNotActive);
	return currentGraphicsState.minimumDistance;
}

void InstructionProcessor::setAutoFlip(bool aAutoFlip) {
	assert(state != psNotActive);
	if (state==psFontProgram)
		throw InstructionException ("The auto_flip variable may not be set in the font program");
	currentGraphicsState.autoFlip = aAutoFlip;
}

bool InstructionProcessor::getAutoFlip() {
	assert(state != psNotActive);
	return currentGraphicsState.autoFlip;
}

void InstructionProcessor::setSingleWidthCutIn (NewF26Dot6 aCutIn) {
	assert(state != psNotActive);
	currentGraphicsState.singleWidthCutIn = aCutIn;
}

NewF26Dot6 InstructionProcessor::getSingleWidthCutIn() {
	assert(state != psNotActive);
	return currentGraphicsState.singleWidthCutIn;
}

void InstructionProcessor::setSingleWidthValue (NewF26Dot6 aValue) {
	assert(state != psNotActive);
	currentGraphicsState.singleWidthValue = aValue;
}

NewF26Dot6 InstructionProcessor::getSingleWidthValue() {
	assert(state != psNotActive);
	return currentGraphicsState.singleWidthValue;
}


NewF26Dot6 InstructionProcessor::getCVTValue (ULong index) {
	assert(state != psNotActive);
	if (index<0 || index >= cvt.size())
		throw InstructionException ("Invalid CVT entry index " + 
			String (index));

	if (state == psGlyphProgram) {
		if (cvt [index].local) {
			if (!cvt [index].set)
				throw InstructionException ("CVT entry " + 
					String (index) + " set in glyph program may not be used unset");
		} else {
			if (!cvt [index].set)
				cvt [index].global = true;
		}
	}

	return cvt [index].value *
		weightedAverage (F18Dot14 (ppemX), F18Dot14 (ppemY), currentGraphicsState.projectionVector) /
		cvt [index].ppem;
}

void InstructionProcessor::setCVTValuePixels(ULong index, NewF26Dot6 aValue) {
	assert(state != psNotActive);

	if (index<0 || index >= cvt.size())
		throw InstructionException ("Invalid CVT entry index " + 
			String (index));

	if (state == psGlyphProgram) {
		if (cvt [index].global) {
			if (!cvt [index].set)
				throw InstructionException ("CVT entry " + 
					String (index) + " used as global in glyph program may not be set");
		}
		cvt [index].local = true;
	}

	cvt [index].set = true;
	cvt [index].value = aValue.get_i();
	cvt [index].ppem = weightedAverage(NewF26Dot6 (ppemX), NewF26Dot6 (ppemY),
		currentGraphicsState.projectionVector).get_i();
}

void InstructionProcessor::setCVTValueFUnits(ULong index, Long aValue) {
	assert(state != psNotActive);

/*	if (state != psCVTProgram) {
		error("CVT values may only be set in the prep program (CVT program).", 0);
		return;
	}*/

	if (index<0 || index >= cvt.size())
		throw InstructionException ("Invalid CVT entry index " + 
			String (index));

	if (state == psGlyphProgram) {
		if (cvt [index].global) {
			if (!cvt [index].set)
				throw InstructionException ("CVT entry " + 
					String (index) + " used as global in glyph program may not be set");
		}
		cvt [index].local = true;
	}

	cvt [index].set = true;
	cvt [index].value = aValue;
	cvt [index].ppem = unitsPerEm;
}

NewF26Dot6 InstructionProcessor::compensateForColour (NewF26Dot6 n, Byte colour) {
	if (colour > oiColourWhite)
		throw InstructionException ("Invalid colour index " + String(colour));

	return n;
}




/*** Instruction ***/

Instruction::Instruction (MemoryPen &pen) {
	instruction = pen.readByte();
	offset = pen.getPosition();
}

Instruction::Instruction(Byte aInstruction) {
	instruction = aInstruction;
	offset = 0;
}

void Instruction::increaseInstruction(Byte with) {
	instruction += with;
}

ULong Instruction::getOffset() const {
	return offset;
}

ULong Instruction::getByteSize() const {
	return 1;
}

void Instruction::optimiseByteSize() {}

void Instruction::write (MemoryWritePen &pen) const {
	pen.writeByte (instruction);
}

/*** InstructionException ***/

InstructionException::~InstructionException() {}

void InstructionException::setInstructionPosition (const InstructionProcessor::InstructionPosition &aPosition) {

	context.push_back ("at " + InstructionProcessor::positionToString (aPosition));
}

} // end namespace OpenType
