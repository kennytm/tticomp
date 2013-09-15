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
	OTInstructionProcessor
*/

#ifndef OTINSTRUCTIONPROCESSOR_H
#define OTINSTRUCTIONPROCESSOR_H

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <vector>
#include <deque>
#include <cmath>

#include "../Util/smart_ptr.h"
#include "../Util/String.h"
#include "../OTFont/OpenType.h"
#include "../OTFont/OTException.h"

namespace OpenType {
	class Instruction;
	class InstructionException;
	typedef util::smart_ptr <InstructionException> InstructionExceptionPtr;

	class InstructionProcessor {
	public:
		/*** Definitions ***/

		typedef struct {
			NewF26Dot6 currentX, currentY;
			NewF26Dot6 originalX, originalY;
			bool onCurve;
			bool touchedX, touchedY;
			bool lastContourPoint;
		} GridFittedPoint;

		typedef std::vector <GridFittedPoint> Points;

		typedef util::smart_ptr <Instruction> InstructionPtr;
		typedef std::vector <InstructionPtr> Instructions;
		typedef Instructions::iterator InstructionIterator;

		enum ProcessorState {
			psNotActive, psFontProgram, psCVTProgram, psGlyphProgram
		};

		enum ProgramType {
			ptFontProgram, ptCVTProgram, ptGlyphProgram, ptUnknown
		};

		struct InstructionStream {
			ProgramType programType;
			Instructions instructions;
		};

		struct InstructionPosition {
			InstructionStream *stream;
			InstructionIterator position;
		};

		static util::String positionToString(const InstructionPosition &p);
		static util::String stateToString(ProcessorState s);

		struct StorageElement {
			Long n;
			bool initialised;
		};

		typedef std::vector <StorageElement> Storage;

		struct CVTEntry {
			Long value;
			Long ppem;
			bool local;
			bool global;
				// set indicates whether the entry has been set in this execution block
			bool set;
		};

		typedef std::vector <CVTEntry> CVT;

		struct Vector {
			// x and y should always form a unit vector, i.e. x*x+y*y == 1
			F2Dot14 x, y;
		};

		typedef struct {
			bool		autoFlip;
			NewF26Dot6	controlValueCutIn;
			ULong		deltaBase;
			ULong		deltaShift;
			Vector		dualProjectionVector;
			Vector		freedomVector;
			Vector		projectionVector;
			ULong		instructionControl;
			int			loop;
			NewF26Dot6	minimumDistance;
			NewF26Dot6	roundPeriod, roundPhase, roundThreshold;
			int			rp[3];
			Byte		zp[3];
			bool		scanControl;
			NewF26Dot6	singleWidthCutIn;
			NewF26Dot6	singleWidthValue;
		} GraphicsState;

	protected:
		util::smart_ptr <OpenTypeFont> font;

		Instructions getInstructions (MemoryBlockPtr memory);
		InstructionPtr getOneInstruction (MemoryPen &pen);

		typedef std::vector <InstructionPosition> CallStack;

		typedef struct {
			ULong id;
			InstructionStream stream;
		} FunctionDefinition;

		typedef std::vector <FunctionDefinition> FunctionDefinitions;
		typedef FunctionDefinitions::iterator FunctionDefinitionIterator;

		static NewF26Dot6 projectOnto (NewF26Dot6 a, NewF26Dot6 b, const Vector & vec) {
			return a * vec.x + b * vec.y;
		}

		static NewF26Dot6 weightedAverage (NewF26Dot6 a, NewF26Dot6 b, const Vector & vec) {
			return NewF26Dot6 (sqrt (vec.x * vec.x * a * a + vec.y * vec.y * b * b));
		}

		static F18Dot14 weightedAverage (F18Dot14 a, F18Dot14 b, const Vector & vec) {
			return F18Dot14 (sqrt (vec.x * vec.x * a * a + vec.y * vec.y * b * b));
		}

	protected:
		/*** Currently loaded font ***/

		InstructionStream fontProgram;
		InstructionStream cvtProgram;
		InstructionStream glyphProgram;
		FunctionDefinitions functionDefinitions;

		FunctionDefinitionIterator getFunction (ULong id);

		ULong unitsPerEm, ppemX, ppemY, pointSize;
		Storage storage;
		CVT cvt;

		GraphicsState defaultGraphicsState;
		GraphicsState currentGraphicsState;
		void resetGraphicsState (bool initially);
		void setUnitVector (Vector &vector, F18Dot14 x, F18Dot14 y);
		Points points;

		/*** Current processor state ***/
		std::vector <Long> stack;

		Points twilight;
		Points contourPoints;

		ProcessorState state;
		InstructionPosition currentInstruction;
		InstructionPosition nextInstruction;
		CallStack callStack;

	public:
		/*** Public methods ***/
		InstructionProcessor();
		// Set current font
		void setFont (util::smart_ptr <OpenTypeFont> aFont);
		// Set font from InstructionProcessor, eliminating the instruction
		// extraction process (and using less memory!)
		void setFont (const InstructionProcessor &aProc);
		virtual ~InstructionProcessor();
		void setPPEM (ULong aPPEMx, ULong aPPEMy, ULong aPointSize);
		Points getGlyphPoints (UShort glyphId);

		static NewF26Dot6 roundToGrid (NewF26Dot6 pos);
		static NewF26Dot6 round (NewF26Dot6 n, NewF26Dot6 period, NewF26Dot6 phase, NewF26Dot6 threshold);

		NewF26Dot6 toF26Dot6x (FWord a) {
			return NewF26Dot6 (a) * ppemX / unitsPerEm;
		}

		NewF26Dot6 toF26Dot6y (FWord a) {
			return NewF26Dot6 (a) * ppemY / unitsPerEm;
		}

	protected:
		void loadGlyph (GlyphPtr glyph);
		virtual void executeInstructions(ProcessorState aState);

	protected:
		/*** Methods used by instructions ***/

		virtual void	addWarning (InstructionExceptionPtr newWarning) = 0;
		ULong			getPointSize();
		bool			getGreyscale();
		ULong			getPointNum();
		InstructionIterator
						skipNextInstruction();
		void			jumpTo (ULong offset);
			// Set the function to (currentInstruction, nextInstruction)
		void			defineFunction(ULong id);
		void			callFunction(ULong id);
		void			popFunctionCallStack();

		void			push (const std::vector <Short> &elements);
		void			push (Long element);
		void			push (NewF26Dot6 element) { push (element.get_i()); }
		Long			pop();
		NewF26Dot6		popFixed() { return NewF26Dot6 (pop(), util::fixed_fraction()); }
		Long			getNthStackElement (ULong indexFromLast);
		Long			removeNthStackElement (ULong indexFromLast);
		void			clearStack();
		ULong			getStackElementNum();

		virtual void	setStorage(ULong location, Long value);
		Long			getStorage(ULong location);
		ULong			getPPEM();
		void			setLoop(ULong aLoop);
		ULong			getLoop();
		void			setInstructionExecutionControl(ULong mask, ULong value);

		void			setFreedomVector(F18Dot14 x, F18Dot14 y);
			// Set both the projectionVector and the dualProjectionVector
		void			setProjectionVector(F18Dot14 x, F18Dot14 y);
		void			setDualProjectionVector(F18Dot14 x, F18Dot14 y);
		Vector			getFreedomVector();
		Vector			getProjectionVector();
		void			setRoundingState(NewF26Dot6 aPeriod, NewF26Dot6 aPhase, NewF26Dot6 aThreshold);
		void			setZonePointer (Byte index, Long zone);
		Long			getZonePointer (Byte index);
		void			setReferencePoint (Byte index, Long aPoint);
		Long			getReferencePoint (Byte index);
		virtual void	setCVTValuePixels (ULong index, NewF26Dot6 aValue);
		virtual void	setCVTValueFUnits (ULong index, Long aValue);		// Convert aValue according to current projectionVector
		virtual NewF26Dot6	getCVTValue(ULong index);
		void			setControlValueCutIn (NewF26Dot6 n);
		NewF26Dot6		getControlValueCutIn();
		void			setMinimumDistance (NewF26Dot6 aMinDist);
		NewF26Dot6		getMinimumDistance();
		void			setAutoFlip(bool aAutoFlip);
		bool			getAutoFlip();
		void			setSingleWidthCutIn (NewF26Dot6 aCutIn);
		NewF26Dot6		getSingleWidthCutIn();
		void			setSingleWidthValue (NewF26Dot6 aValue);
		NewF26Dot6		getSingleWidthValue();
		NewF26Dot6		round (NewF26Dot6 n);		//< Round n according to rounding state
		void			setDeltaBase(ULong aDeltaBase);
		ULong			getDeltaBase();
		void			setDeltaShift(ULong aDeltaShift);
		ULong			getDeltaShift();

		void			movePoint (ULong zone, ULong index, NewF26Dot6 newPos);
		void			moveOriginalPoint (ULong zone, ULong index, NewF26Dot6 newPos);
		void			moveOriginalPointToXY (ULong zone, ULong index, NewF26Dot6 newX, NewF26Dot6 newY);
		void			shiftPoint (ULong zone, ULong index, NewF26Dot6 amount, bool touch = true);
		void			movePointToXY (ULong zone, ULong index, NewF26Dot6 newX, NewF26Dot6 newY);
		void			setOnCurve (ULong index, bool aOnCurve);
		bool			getOnCurve (ULong index);
		ULong			getLastContourPoint (ULong contour);
		void			interpolatePointsX();
		void			interpolatePointsY();

						// Return point coordinate according to projectionVector
		NewF26Dot6		getPoint(ULong zone, ULong index);
		NewF26Dot6		getOriginalPoint(ULong zone, ULong index, bool dual);
		NewF26Dot6		getPointX(ULong zone, ULong index);
		NewF26Dot6		getPointY(ULong zone, ULong index);
		NewF26Dot6		getOriginalPointX(ULong zone, ULong index);
		NewF26Dot6		getOriginalPointY(ULong zone, ULong index);
		void			unTouchPoint(ULong index);
		ULong			getTwilightPointNum();

		NewF26Dot6		compensateForColour (NewF26Dot6 n, Byte colour);

		friend class Instruction;
		friend class InstructionException;

		// friend classes, Instruction-derived:

		friend class PushInstruction;
		friend class ReadStoreInstruction;
		friend class WriteStoreInstruction;
		friend class WriteCVTPixelsInstruction;
		friend class WriteCVTFUnitsInstruction;
		friend class ReadCVTInstruction;
		friend class VectorsToAxisInstruction;
		friend class ProjectionToAxisInstruction;
		friend class FreedomToAxisInstruction;
		friend class ProjectionToLineInstruction;
		friend class FreedomToLineInstruction;
		friend class FreedomToProjectionInstruction;
		friend class DualProjectionToLineInstruction;
		friend class ProjectionFromStackInstruction;
		friend class FreedomFromStackInstruction;
		friend class GetProjectionVectorInstruction;
		friend class GetFreedomVectorInstruction;
		friend class SetReferencePointInstruction;
		friend class SetZonePointerInstruction;
		friend class SetZonePointersInstruction;
		friend class RoundToHalfGridInstruction;
		friend class RoundToGridInstruction;
		friend class RoundToDoubleGridInstruction;
		friend class RoundDownToGridInstruction;
		friend class RoundUpToGridInstruction;
		friend class RoundOffInstruction;
		friend class SuperRoundInstruction;
		friend class SuperRound45Instruction;
		friend class SetLoopInstruction;
		friend class SetMinimumDistanceInstruction;
		friend class InstructionControlInstruction;
		friend class ScanConversionControlInstruction;
		friend class ScanTypeInstruction;
		friend class ControlValueCutInInstruction;
		friend class SetSingleWidthCutInInstruction;
		friend class SetSingleWidthInstruction;
		friend class SetAutoFlipInstruction;
		friend class SetAngleWeightInstruction;
		friend class DeltaBaseInstruction;
		friend class DeltaShiftInstruction;
		friend class GetCoordinateInstruction;
		friend class SetCoordinateInstruction;
		friend class MeasureDistanceInstruction;
		friend class MeasurePPEMInstruction;
		friend class MeasurePointSizeInstruction;
		friend class FlipPointInstruction;
		friend class FlipRangeInstruction;
		friend class ShiftPointInstruction;
		friend class ShiftContourInstruction;
		friend class ShiftZoneInstruction;
		friend class ShiftPointByPixelsInstruction;
		friend class MoveStackIndirectRelativePointInstruction;
		friend class MoveDirectAbsolutePointInstruction;
		friend class MoveIndirectAbsolutePointInstruction;
		friend class MoveDirectRelativePointInstruction;
		friend class MoveIndirectRelativePointInstruction;
		friend class AlignInstruction;
		friend class MoveToIntersectionInstruction;
		friend class AlignPointsInstruction;
		friend class InterpolatePointInstruction;
		friend class UntouchPointInstruction;
		friend class InterpolateUntouchedPointsInstruction;
		friend class DeltaPInstruction;
		friend class DeltaCInstruction;
		friend class DuplicateStackElementInstruction;
		friend class PopInstruction;
		friend class ClearStackInstruction;
		friend class SwapStackElementsInstruction;
		friend class StackDepthInstruction;
		friend class CopyIndexInstruction;
		friend class MoveIndexInstruction;
		friend class RollStackInstruction;
		friend class IfInstruction;
		friend class ElseInstruction;
		friend class EndIfInstruction;
		friend class JumpRelOnTrueInstruction;
		friend class JumpInstruction;
		friend class JumpRelOnFalseInstruction;
		friend class LessThanInstruction;
		friend class LessThanOrEqualInstruction;
		friend class GreaterThanInstruction;
		friend class GreaterThanOrEqualInstruction;
		friend class EqualInstruction;
		friend class NotEqualInstruction;
		friend class OddInstruction;
		friend class EvenInstruction;
		friend class AndInstruction;
		friend class OrInstruction;
		friend class NotInstruction;
		friend class AddInstruction;
		friend class SubtractInstruction;
		friend class DivideInstruction;
		friend class MultiplyInstruction;
		friend class AbsoluteInstruction;
		friend class NegateInstruction;
		friend class FloorInstruction;
		friend class CeilingInstruction;
		friend class MaximumInstruction;
		friend class MinimumInstruction;
		friend class RoundInstruction;
		friend class NoRoundInstruction;
		friend class FunctionDefInstruction;
		friend class EndFunctionDefInstruction;
		friend class CallInstruction;
		friend class LoopCallInstruction;
		friend class InstructionDefInstruction;
		friend class GetInformationInstruction;

		// end friend classes
	};

	class Instruction {
	protected:
		Byte instruction;
		ULong offset;

		friend class IfInstruction;
		friend class ElseInstruction;
		friend class FunctionDefInstruction;

	public:
						// Read data from *aBuffer and set to next instruction in instruction stream
		Instruction(MemoryPen &pen);
		Instruction(Byte instruction);
		virtual ~Instruction() {}
						// Execute and return next instruction to execute
		virtual void execute (InstructionProcessor &proc) const = 0;
		ULong getOffset() const;

		virtual util::String getName() const = 0;

		virtual ULong getByteSize() const;
		virtual void optimiseByteSize();
		virtual void write (MemoryWritePen &pen) const;

		void increaseInstruction (Byte with);
	};

	class InstructionException : public Exception {
	public:
		InstructionException (const util::String aDescription)
			: Exception (aDescription) {}
		virtual ~InstructionException();

		void setInstructionPosition (const InstructionProcessor::InstructionPosition &aPosition);
	};
}

#endif // OTINSTRUCTIONPROCESSOR_H
