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

	In addition, as a special exception, Rogier van Dalen gives permission
	to link the code of this program with Qt non-commercial edition (or with
	modified versions of Qt non-commercial edition that use the	same license
	as Qt non-commercial edition), and distribute linked combinations
	including the two.  You must obey the GNU General Public License in all
	respects for all of the code used other than Qt non-commercial edition.
	If you modify this file, you may extend this exception to your version of
	the file, but you are not obligated to do so.  If you do not wish to do
	so, delete this exception statement from your version.
*/

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include "glyphprocessor.h"
#include "messagedialog.h"

using util::String;

GlyphProcessor::GlyphProcessor (MessageDialog *aMessageDialog)
: MessageInstructionProcessor (aMessageDialog), instructionCount (0) {}

GlyphProcessor::~GlyphProcessor() {}

void GlyphProcessor::setGlyphId (UShort aGlyphId) {
	glyphId = aGlyphId;
}

const GlyphProcessor::Instructions & GlyphProcessor::getFontProgram() {
	return fontProgram.instructions;
}

const GlyphProcessor::Instructions & GlyphProcessor::getCVTProgram() {
	return cvtProgram.instructions;
}
	
const GlyphProcessor::Instructions & GlyphProcessor::getGlyphProgram() {
	return glyphProgram.instructions;
}

const GlyphProcessor::Points & GlyphProcessor::getPoints() {
	return points;
}

int GlyphProcessor::getCurrentInstructionCount() {
	return instructionCount;
}

bool GlyphProcessor::isLastInstruction() {
	return currentInstruction.position == currentInstruction.stream->instructions.end();
}

QString GlyphProcessor::getCurrentInstructionPosition() {
	return positionToString (currentInstruction).getCString();
}

QString GlyphProcessor::getCurrentInstructionDescription() {
	if (currentInstruction.position == currentInstruction.stream->instructions.end())
		return error.isEmpty() ? "(end of instructions)" : QString ("Error: ") + error;
	else
		return (*currentInstruction.position)->getName().getCString();
}

void GlyphProcessor::restart() {
	try {
		error = "";

		GlyphPtr glyph = font->getGlyph (glyphId);
		Exception::FontContext c1 (*font);
		Exception::Context c2 ("loading glyph " + String (glyphId) + " \'"
			+ glyph->getName() + "\'");

		callStack.clear();

		stack.clear();
		stack.reserve (font->getMaxStackElements());

		twilight.clear();

		InstructionProcessor::loadGlyph (glyph);

		GridFittedPoint p;// = {0,0, 0,0, true, false, false, true};
		p.currentX = p.currentY = p.originalX = p.originalY = 0;
		p.onCurve = true;
		p.touchedX = false;
		p.touchedY = false;
		p.lastContourPoint = true;
		twilight.insert (twilight.end(), font->getMaxTwilightPoints(), p);

		instructionCount = 0;
		currentInstruction.stream = &glyphProgram;
		currentInstruction.position = glyphProgram.instructions.begin();

		currentGraphicsState = defaultGraphicsState;
		nextInstruction.stream = currentInstruction.stream;

		state = psGlyphProgram;
	} catch (Exception & e) {
		messageDialog->addMessage (e, true);
	}
}

void GlyphProcessor::runToCount (int count) {
	assert (instructionCount <= count);
	GlyphPtr glyph = font->getGlyph (glyphId);
	Exception::FontContext c1 (*font);
	Exception::Context c2 ("executing instructions for glyph " + String (glyphId) + " \'"
		+ glyph->getName() + "\'");

	try {
		while (instructionCount < count &&
			currentInstruction.position != currentInstruction.stream->instructions.end())
		{
			nextInstruction.position = currentInstruction.position + 1;
			(*currentInstruction.position)->execute (*this);

			currentInstruction = nextInstruction;
			instructionCount ++;
		}

		if (currentInstruction.position == currentInstruction.stream->instructions.end()) {
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
	} catch (InstructionException & e) {
		e.setInstructionPosition (currentInstruction);
		messageDialog->addMessage (e, true);
		error = String (e.getDescriptions().front()).getCString();
		currentInstruction.position = currentInstruction.stream->instructions.end();
		instructionCount ++;
	}
}

void  GlyphProcessor::runToEnd() {
	runToCount (100000);
}

const std::vector <Long> &  GlyphProcessor::getStack() {
	return stack;
}

const GlyphProcessor::Storage & GlyphProcessor::getStorageElements() {
	return storage;
}

const GlyphProcessor::CVT GlyphProcessor::getCVTEntries() {
	return cvt;
}

NewF26Dot6 GlyphProcessor::getPPEMPixels() {
	if (state == psNotActive)
		return ppemX;
	return weightedAverage(NewF26Dot6 (ppemX), NewF26Dot6 (ppemY),
		currentGraphicsState.projectionVector);
}

const GlyphProcessor::GraphicsState & GlyphProcessor::getGraphicsState() {
	return currentGraphicsState;
}
