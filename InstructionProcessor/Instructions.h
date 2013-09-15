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
	\file OTInstructions.h contains code for handling various TrueType instructions.
*/

#ifndef OTINSTRUCTIONS_H
#define OTINSTRUCTIONS_H

#include "InstructionProcessor.h"

namespace OpenType {

	// Order as in TrueType specification
	enum InstructionByte {
		oiNPUSHB		= 0x40,
		oiNPUSHW		= 0x41,
		oiPUSHB			= 0xB0,		// 0xB0 to 0xB7
		oiPUSHBend		= 0xB7,
		oiPUSHW			= 0xB8		,// 0xB8 to 0xBF
		oiPUSHWend		= 0xBF,
		oiPUSHelementNum	= 0x07,

		oiRS			= 0x43,
		oiWS			= 0x42,
		oiWCVTP			= 0x44,
		oiWCVTF			= 0x70,
		oiRCVT			= 0x45,

		oiSVTCAy		= 0x00		,// 0x00 to 0x01
		oiSVTCAx		= 0x01,
		oiSPVTCAy		= 0x02		,// 0x02 to 0x03
		oiSPVTCAx		= 0x03,
		oiSFVTCAy		= 0x04		,// 0x04 to 0x05
		oiSFVTCAx		= 0x05,
		oiSPVTL			= 0x06		,// 0x06 to 0x07
		oiSPVTLperp		= 0x07,
		oiSFVTL			= 0x08		,// 0x08 to 0x09
		oiSFVTLperp		= 0x09,
		oiSFVTPV		= 0x0E,
		oiSDPVTL		= 0x86		,// 0x86 to 0x87
		oiSDPVTLperp	= 0x87,
		oiSPVFS			= 0x0A,
		oiSFVFS			= 0x0B,

		oiGPV			= 0x0C,
		oiGFV			= 0x0D,

		oiSRP0			= 0x10,
		oiSRP1			= 0x11,
		oiSRP2			= 0x12,
		oiSZP0			= 0x13,
		oiSZP1			= 0x14,
		oiSZP2			= 0x15,
		oiSZPS			= 0x16,

		oiRTHG			= 0x19,
		oiRTG			= 0x18,
		oiRTDG			= 0x3D,
		oiRDTG			= 0x7D,
		oiRUTG			= 0x7C,
		oiROFF			= 0x7A,
		oiSROUND		= 0x76,
		oiS45ROUND		= 0x77,

		oiRoundPeriod		= 0xC0,
		oiRoundPeriodSh		= 6,
		oiRoundPhase		= 0x30,
		oiRoundPhaseSh		= 4,
		oiRoundThreshold	= 0x0F,

		oiSLOOP			= 0x17,
		oiSMD			= 0x1A,
		oiINSTCTRL		= 0x8E,
		oiSCANCTRL		= 0x85,
		oiSCANTYPE		= 0x8D,
		oiSCVTCI		= 0x1D,
		oiSSWCI			= 0x1E,
		oiSSW			= 0x1F,
		oiFLIPON		= 0x4D,
		oiFLIPOFF		= 0x4E,

		oiSDB			= 0x5E,
		oiSDS			= 0x5F,

		oiGCcur			= 0x46		,// 0x46 to 0x47
		oiGCorig		= 0x47,
		oiSCFS			= 0x48,
		oiMDcur			= 0x49		,// 0x49 to 0x4A
		oiMDorig		= 0x4A,
		oiMPPEM			= 0x4B,
		oiMPS			= 0x4C,

		// Section 2 of specs
		oiFLIPPT		= 0x80,
		oiFLIPRGON		= 0x81,
		oiFLIPRGOFF		= 0x82,
		oiSHP21			= 0x32		,// 0x32 to 0x33
		oiSHP10			= 0x33,
		oiSHC21			= 0x34		,// 0x34 to 0x25
		oiSHC10			= 0x35,
		oiSHZ21			= 0x36		,// 0x36 to 0x37
		oiSHZ10			= 0x37,
		oiSHPIX			= 0x38,
		oiMSIRP			= 0x3A		,// 0x3A to 0x3B
		oiMSIRPset		= 0x3B,
		oiMDAP			= 0x2E		,// 0x2E to 0x2F
		oiMDAPround		= 0x2F,
		oiMIAP			= 0x3E		,// 0x3E to ox3F
		oiMIAPround		= 0x3F,
		oiMDRP			= 0xC0		,// 0xC0 to 0xDF
		oiMDRPend		= 0xDF,
		oiMIRP			= 0xE0		,// 0xE0 to 0xFF
		oiMIRPend		= 0xFF,
		oiMRPsetrp0			= 0x10,
		oiMRPminDist		= 0x08,
		oiMRPround			= 0x04,
		oiMRPcolour			= 0x03,

		oiALIGN			= 0x3C,
		oiISECT			= 0x0F,
		oiALIGNPTS		= 0x27,
		oiIP			= 0x39,
		oiUTP			= 0x29,
		oiIUPy			= 0x30		,// 0x30 to 0x31
		oiIUPx			= 0x31,

		oiDELTAP1		= 0x5D,
		oiDELTAP2		= 0x71,
		oiDELTAP3		= 0x72,
		oiDELTAC1		= 0x73,
		oiDELTAC2		= 0x74,
		oiDELTAC3		= 0x75,

		oiDUP			= 0x20,
		oiPOP			= 0x21,
		oiCLEAR			= 0x22,
		oiSWAP			= 0x23,
		oiDEPTH			= 0x24,
		oiCINDEX		= 0x25,
		oiMINDEX		= 0x26,
		oiROLL			= 0x8A,

		oiIF			= 0x58,
		oiELSE			= 0x1B,
		oiEIF			= 0x59,
		oiJR			= 0x78,
		oiJMP			= 0x1C,
		oiJROF			= 0x79,

		oiLT			= 0x50,
		oiLTEQ			= 0x51,
		oiGT			= 0x52,
		oiGTEQ			= 0x53,
		oiEQ			= 0x54,
		oiNEQ			= 0x55,

		oiODD			= 0x56,
		oiEVEN			= 0x57,

		oiAND			= 0x5A,
		oiOR			= 0x5B,
		oiN			= 0x5C,
		oiADD			= 0x60,
		oiSUB			= 0x61,
		oiDIV			= 0x62,
		oiMUL			= 0x63,
		oiABS			= 0x64,
		oiNEG			= 0x65,
		oiFLOOR			= 0x66,
		oiCEILING		= 0x67,
		oiMAX			= 0x8B,
		oiMIN			= 0x8C,
		oiROUND			= 0x68		,// 0x68 to 0x6b
		oiROUNDend		= 0x6B,
		oiNROUND		= 0x6C,
		oiNROUNDend		= 0x6F,
		oiFDEF			= 0x2C,
		oiENDF			= 0x2D,
		oiCALL			= 0x2B,
		oiLOOPCALL		= 0x2A,
		oiIDEF			= 0x89,
		oiGETINFO		= 0x88
	};

	#define oiInfoSelectVersion		0x01
	#define oiInfoSelectRotation	0x02
	#define oiInfoSelectStretched	0x04
	#define oiInfoSelectGreyscale	0x20

	#define oiInfoResultVersion		0x00FF
	#define oiInfoResultRotation	0x0200
	#define oiInfoResultStretched	0x0400
	#define oiInfoResultGreyscale	0x1000

	#define oiColourGrey			0x00
	#define oiColourBlack			0x01
	#define oiColourWhite			0x02

	// Instruction execution control values
	#define ieInhibitGridFitting	0x01
	#define ieResetState			0x02

	/*** Classes ***/

	class PushInstruction : public Instruction {
	public:
		typedef std::vector <Short> Elements;

		PushInstruction(MemoryPen &pen);
		virtual ~PushInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	private:
		Elements elements;
	};

	class ReadStoreInstruction : public Instruction {
	public:
		ReadStoreInstruction(MemoryPen &pen) : Instruction (pen) {}
		ReadStoreInstruction() : Instruction (oiRS) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class WriteStoreInstruction : public Instruction {
	public:
		WriteStoreInstruction(MemoryPen &pen) : Instruction (pen) {}
		WriteStoreInstruction() : Instruction (oiWS) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class WriteCVTPixelsInstruction : public Instruction {
	public:
		WriteCVTPixelsInstruction(MemoryPen &pen) : Instruction (pen) {}
		WriteCVTPixelsInstruction(): Instruction(oiWCVTP) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class WriteCVTFUnitsInstruction : public Instruction {
	public:
		WriteCVTFUnitsInstruction(MemoryPen &pen) : Instruction (pen) {}
		WriteCVTFUnitsInstruction() : Instruction(oiWCVTF) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ReadCVTInstruction : public Instruction {
	public:
		ReadCVTInstruction(MemoryPen &pen) : Instruction (pen) {}
		ReadCVTInstruction() : Instruction(oiRCVT) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class VectorsToAxisInstruction : public Instruction {
	public:
		VectorsToAxisInstruction(MemoryPen &pen) : Instruction (pen) {}
		VectorsToAxisInstruction(bool x);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ProjectionToAxisInstruction : public Instruction {
	public:
		ProjectionToAxisInstruction(MemoryPen &pen) : Instruction (pen) {}
		ProjectionToAxisInstruction(bool x);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class FreedomToAxisInstruction : public Instruction {
	public:
		FreedomToAxisInstruction(MemoryPen &pen) : Instruction (pen) {}
		FreedomToAxisInstruction(bool x);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ProjectionToLineInstruction : public Instruction {
	public:
		ProjectionToLineInstruction(MemoryPen &pen) : Instruction (pen) {}
		ProjectionToLineInstruction(bool perp);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class FreedomToLineInstruction : public Instruction {
	public:
		FreedomToLineInstruction(MemoryPen &pen) : Instruction (pen) {}
		FreedomToLineInstruction(bool perp);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class FreedomToProjectionInstruction : public Instruction {
	public:
		FreedomToProjectionInstruction(MemoryPen &pen) : Instruction (pen) {}
		FreedomToProjectionInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class DualProjectionToLineInstruction : public Instruction {
	public:
		DualProjectionToLineInstruction(MemoryPen &pen) : Instruction (pen) {}
		DualProjectionToLineInstruction(bool perp);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ProjectionFromStackInstruction : public Instruction {
	public:
		ProjectionFromStackInstruction(MemoryPen &pen) : Instruction (pen) {}
		ProjectionFromStackInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class FreedomFromStackInstruction : public Instruction {
	public:
		FreedomFromStackInstruction(MemoryPen &pen) : Instruction (pen) {}
		FreedomFromStackInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class GetProjectionVectorInstruction : public Instruction {
	public:
		GetProjectionVectorInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class GetFreedomVectorInstruction : public Instruction {
	public:
		GetFreedomVectorInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SetReferencePointInstruction : public Instruction {
	public:
		SetReferencePointInstruction(MemoryPen &pen) : Instruction (pen) {}
		SetReferencePointInstruction(Byte rp);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SetZonePointerInstruction : public Instruction {
	public:
		SetZonePointerInstruction(MemoryPen &pen) : Instruction (pen) {}
		SetZonePointerInstruction(Byte zp);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SetZonePointersInstruction : public Instruction {
	public:
		SetZonePointersInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class RoundToHalfGridInstruction : public Instruction {
	public:
		RoundToHalfGridInstruction(MemoryPen &pen) : Instruction (pen) {}
		RoundToHalfGridInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class RoundToGridInstruction : public Instruction {
	public:
		RoundToGridInstruction(MemoryPen &pen) : Instruction (pen) {}
		RoundToGridInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class RoundToDoubleGridInstruction : public Instruction {
	public:
		RoundToDoubleGridInstruction(MemoryPen &pen) : Instruction (pen) {}
		RoundToDoubleGridInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class RoundDownToGridInstruction : public Instruction {
	public:
		RoundDownToGridInstruction(MemoryPen &pen) : Instruction (pen) {}
		RoundDownToGridInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class RoundUpToGridInstruction : public Instruction {
	public:
		RoundUpToGridInstruction(MemoryPen &pen) : Instruction (pen) {}
		RoundUpToGridInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class RoundOffInstruction : public Instruction {
	public:
		RoundOffInstruction(MemoryPen &pen) : Instruction (pen) {}
		RoundOffInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SuperRoundInstruction : public Instruction {
	public:
		SuperRoundInstruction(MemoryPen &pen) : Instruction (pen) {}
		SuperRoundInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SuperRound45Instruction : public Instruction {
	public:
		SuperRound45Instruction(MemoryPen &pen) : Instruction (pen) {}
		SuperRound45Instruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SetLoopInstruction : public Instruction {
	public:
		SetLoopInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SetMinimumDistanceInstruction : public Instruction {
	public:
		SetMinimumDistanceInstruction(MemoryPen &pen) : Instruction (pen) {}
		SetMinimumDistanceInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class InstructionControlInstruction : public Instruction {
	public:
		InstructionControlInstruction(MemoryPen &pen) : Instruction (pen) {}
		InstructionControlInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ScanConversionControlInstruction : public Instruction {
	public:
		ScanConversionControlInstruction(MemoryPen &pen) : Instruction (pen) {}
		ScanConversionControlInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ScanTypeInstruction : public Instruction {
	public:
		ScanTypeInstruction(MemoryPen &pen) : Instruction (pen) {}
		ScanTypeInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ControlValueCutInInstruction : public Instruction {
	public:
		ControlValueCutInInstruction(MemoryPen &pen) : Instruction (pen) {}
		ControlValueCutInInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SetSingleWidthCutInInstruction : public Instruction {
	public:
		SetSingleWidthCutInInstruction(MemoryPen &pen) : Instruction (pen) {}
		SetSingleWidthCutInInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SetSingleWidthInstruction : public Instruction {
	public:
		SetSingleWidthInstruction(MemoryPen &pen) : Instruction (pen) {}
		SetSingleWidthInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SetAutoFlipInstruction : public Instruction {
	public:
		SetAutoFlipInstruction(MemoryPen &pen) : Instruction (pen) {}
		SetAutoFlipInstruction(bool on);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class DeltaBaseInstruction : public Instruction {
	public:
		DeltaBaseInstruction(MemoryPen &pen) : Instruction (pen) {}
		DeltaBaseInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class DeltaShiftInstruction : public Instruction {
	public:
		DeltaShiftInstruction(MemoryPen &pen) : Instruction (pen) {}
		DeltaShiftInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class GetCoordinateInstruction : public Instruction {
	public:
		GetCoordinateInstruction(MemoryPen &pen) : Instruction (pen) {}
		GetCoordinateInstruction(bool orig);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SetCoordinateInstruction : public Instruction {
	public:
		SetCoordinateInstruction(MemoryPen &pen) : Instruction (pen) {}
		SetCoordinateInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MeasureDistanceInstruction : public Instruction {
	public:
		MeasureDistanceInstruction(MemoryPen &pen) : Instruction (pen) {}
		MeasureDistanceInstruction(bool orig);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MeasurePPEMInstruction : public Instruction {
	public:
		MeasurePPEMInstruction(MemoryPen &pen) : Instruction (pen) {}
		MeasurePPEMInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MeasurePointSizeInstruction : public Instruction {
	public:
		MeasurePointSizeInstruction(MemoryPen &pen) : Instruction (pen) {}
		MeasurePointSizeInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class FlipPointInstruction : public Instruction {
	public:
		FlipPointInstruction(MemoryPen &pen) : Instruction (pen) {}
		FlipPointInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class FlipRangeInstruction : public Instruction {
	public:
		FlipRangeInstruction(MemoryPen &pen) : Instruction (pen) {}
		FlipRangeInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ShiftPointInstruction : public Instruction {
	public:
		ShiftPointInstruction(MemoryPen &pen) : Instruction (pen) {}
		ShiftPointInstruction(bool use10);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ShiftContourInstruction : public Instruction {
	public:
		ShiftContourInstruction(MemoryPen &pen) : Instruction (pen) {}
		ShiftContourInstruction(bool use10);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ShiftZoneInstruction : public Instruction {
	public:
		ShiftZoneInstruction(MemoryPen &pen) : Instruction (pen) {}
		ShiftZoneInstruction(bool use10);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ShiftPointByPixelsInstruction : public Instruction {
	public:
		ShiftPointByPixelsInstruction(MemoryPen &pen) : Instruction (pen) {}
		ShiftPointByPixelsInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MoveStackIndirectRelativePointInstruction : public Instruction {
	public:
		MoveStackIndirectRelativePointInstruction(MemoryPen &pen) : Instruction (pen) {}
		MoveStackIndirectRelativePointInstruction(bool setrp0);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MoveDirectAbsolutePointInstruction : public Instruction {
	public:
		MoveDirectAbsolutePointInstruction(MemoryPen &pen) : Instruction (pen) {}
		MoveDirectAbsolutePointInstruction(bool round);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MoveIndirectAbsolutePointInstruction : public Instruction {
	public:
		MoveIndirectAbsolutePointInstruction(MemoryPen &pen) : Instruction (pen) {}
		MoveIndirectAbsolutePointInstruction(bool round);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MoveDirectRelativePointInstruction : public Instruction {
	public:
		MoveDirectRelativePointInstruction(MemoryPen &pen) : Instruction (pen) {}
		MoveDirectRelativePointInstruction(bool setrp0, bool minDist, bool round, Byte colour);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MoveIndirectRelativePointInstruction : public Instruction {
	public:
		MoveIndirectRelativePointInstruction(MemoryPen &pen) : Instruction (pen) {}
		MoveIndirectRelativePointInstruction(bool setrp0, bool minDist, bool round, Byte colour);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class AlignInstruction : public Instruction {
	public:
		AlignInstruction(MemoryPen &pen) : Instruction (pen) {}
		AlignInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MoveToIntersectionInstruction : public Instruction {
	public:
		MoveToIntersectionInstruction(MemoryPen &pen) : Instruction (pen) {}
		MoveToIntersectionInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class AlignPointsInstruction : public Instruction {
	public:
		AlignPointsInstruction(MemoryPen &pen) : Instruction (pen) {}
		AlignPointsInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class InterpolatePointInstruction : public Instruction {
	public:
		InterpolatePointInstruction(MemoryPen &pen) : Instruction (pen) {}
		InterpolatePointInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class UntouchPointInstruction : public Instruction {
	public:
		UntouchPointInstruction(MemoryPen &pen) : Instruction (pen) {}
		UntouchPointInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class InterpolateUntouchedPointsInstruction : public Instruction {
	public:
		InterpolateUntouchedPointsInstruction(MemoryPen &pen) : Instruction (pen) {}
		InterpolateUntouchedPointsInstruction(bool x);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class DeltaPInstruction : public Instruction {
	public:
		DeltaPInstruction(MemoryPen &pen) : Instruction (pen) {}
		DeltaPInstruction(Byte type);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class DeltaCInstruction : public Instruction {
	public:
		DeltaCInstruction(MemoryPen &pen) : Instruction (pen) {}
		DeltaCInstruction(Byte type);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class DuplicateStackElementInstruction : public Instruction {
	public:
		DuplicateStackElementInstruction(MemoryPen &pen) : Instruction (pen) {}
		DuplicateStackElementInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class PopInstruction : public Instruction {
	public:
		PopInstruction();
		PopInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ClearStackInstruction : public Instruction {
	public:
		ClearStackInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SwapStackElementsInstruction : public Instruction {
	public:
		SwapStackElementsInstruction(MemoryPen &pen) : Instruction (pen) {}
		SwapStackElementsInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class StackDepthInstruction : public Instruction {
	public:
		StackDepthInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class CopyIndexInstruction : public Instruction {
	public:
		CopyIndexInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MoveIndexInstruction : public Instruction {
	public:
		MoveIndexInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class RollStackInstruction : public Instruction {
	public:
		RollStackInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class IfInstruction : public Instruction {
	public:
		IfInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class ElseInstruction : public Instruction {
	public:
		ElseInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class EndIfInstruction : public Instruction {
	public:
		EndIfInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class JumpRelOnTrueInstruction : public Instruction {
	public:
		JumpRelOnTrueInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class JumpInstruction : public Instruction {
	public:
		JumpInstruction(MemoryPen &pen) : Instruction (pen) {}
		JumpInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class JumpRelOnFalseInstruction : public Instruction {
	public:
		JumpRelOnFalseInstruction(MemoryPen &pen) : Instruction (pen) {}
		JumpRelOnFalseInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class LessThanInstruction : public Instruction {
	public:
		LessThanInstruction(MemoryPen &pen) : Instruction (pen) {}
		LessThanInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class LessThanOrEqualInstruction : public Instruction {
	public:
		LessThanOrEqualInstruction(MemoryPen &pen) : Instruction (pen) {}
		LessThanOrEqualInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class GreaterThanInstruction : public Instruction {
	public:
		GreaterThanInstruction(MemoryPen &pen) : Instruction (pen) {}
		GreaterThanInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class GreaterThanOrEqualInstruction : public Instruction {
	public:
		GreaterThanOrEqualInstruction(MemoryPen &pen) : Instruction (pen) {}
		GreaterThanOrEqualInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class EqualInstruction : public Instruction {
	public:
		EqualInstruction(MemoryPen &pen) : Instruction (pen) {}
		EqualInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class NotEqualInstruction : public Instruction {
	public:
		NotEqualInstruction(MemoryPen &pen) : Instruction (pen) {}
		NotEqualInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class OddInstruction : public Instruction {
	public:
		OddInstruction(MemoryPen &pen) : Instruction (pen) {}
		OddInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class EvenInstruction : public Instruction {
	public:
		EvenInstruction(MemoryPen &pen) : Instruction (pen) {}
		EvenInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class AndInstruction : public Instruction {
	public:
		AndInstruction(MemoryPen &pen) : Instruction (pen) {}
		AndInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class OrInstruction : public Instruction {
	public:
		OrInstruction(MemoryPen &pen) : Instruction (pen) {}
		OrInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class NotInstruction : public Instruction {
	public:
		NotInstruction(MemoryPen &pen) : Instruction (pen) {}
		NotInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class AddInstruction : public Instruction {
	public:
		AddInstruction(MemoryPen &pen) : Instruction (pen) {}
		AddInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class SubtractInstruction : public Instruction {
	public:
		SubtractInstruction(MemoryPen &pen) : Instruction (pen) {}
		SubtractInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class DivideInstruction : public Instruction {
	public:
		DivideInstruction(MemoryPen &pen) : Instruction (pen) {}
		DivideInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MultiplyInstruction : public Instruction {
	public:
		MultiplyInstruction(MemoryPen &pen) : Instruction (pen) {}
		MultiplyInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class AbsoluteInstruction : public Instruction {
	public:
		AbsoluteInstruction(MemoryPen &pen) : Instruction (pen) {}
		AbsoluteInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class NegateInstruction : public Instruction {
	public:
		NegateInstruction(MemoryPen &pen) : Instruction (pen) {}
		NegateInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class FloorInstruction : public Instruction {
	public:
		FloorInstruction(MemoryPen &pen) : Instruction (pen) {}
		FloorInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class CeilingInstruction : public Instruction {
	public:
		CeilingInstruction(MemoryPen &pen) : Instruction (pen) {}
		CeilingInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MaximumInstruction : public Instruction {
	public:
		MaximumInstruction(MemoryPen &pen) : Instruction (pen) {}
		MaximumInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class MinimumInstruction : public Instruction {
	public:
		MinimumInstruction(MemoryPen &pen) : Instruction (pen) {}
		MinimumInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class RoundInstruction : public Instruction {
	public:
		RoundInstruction(MemoryPen &pen) : Instruction (pen) {}
		RoundInstruction(Byte colour);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class NoRoundInstruction : public Instruction {
	public:
		NoRoundInstruction(MemoryPen &pen) : Instruction (pen) {}
		NoRoundInstruction(Byte colour);
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class FunctionDefInstruction : public Instruction {
	public:
		FunctionDefInstruction(MemoryPen &pen) : Instruction (pen) {}
		FunctionDefInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class EndFunctionDefInstruction : public Instruction {
	public:
		EndFunctionDefInstruction(MemoryPen &pen) : Instruction (pen) {}
		EndFunctionDefInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class CallInstruction : public Instruction {
	public:
		CallInstruction(MemoryPen &pen) : Instruction (pen) {}
		CallInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class LoopCallInstruction : public Instruction {
	public:
		LoopCallInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class InstructionDefInstruction : public Instruction {
	public:
		InstructionDefInstruction(MemoryPen &pen) : Instruction (pen) {}
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};

	class GetInformationInstruction : public Instruction {
	public:
		GetInformationInstruction(MemoryPen &pen) : Instruction (pen) {}
		GetInformationInstruction();
		virtual void execute (InstructionProcessor &proc) const;
		virtual util::String getName() const;
	};
}

#endif	// OTINSTRUCTIONS_H


