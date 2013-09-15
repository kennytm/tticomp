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

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <cassert>
#include "OTException.h"
#include "OpenTypeFile.h"

using std::ostream;
using std::vector;
using util::String;

namespace OpenType {

std::vector <String> exceptionContext;
std::vector <String> exceptionFont;

Exception::Exception (String description)
{
	if (!exceptionFont.empty())
		context.push_back ("Font \"" + exceptionFont.back() + '"');
	std::copy (exceptionContext.begin(), exceptionContext.end(),
		std::back_insert_iterator <Strings> (context));
	descriptions.push_back (description);
}

Exception::~Exception() {}

void Exception::addDescription (String description) {
	descriptions.push_back (description);
}

String Exception::getFullDescription() const {
	String descr;
	if (!context.empty()) {
		Strings::const_iterator i;
		for (i = context.begin(); i != context.end() - 1; i ++) {
			descr += *i;
			descr += ", ";
		}
		descr += *i;
	}
	descr += ": ";

	Strings::const_iterator i;
	assert (descriptions.size() >= 1);
	for (i = descriptions.begin(); i != descriptions.end() - 1; i ++) {
		descr += *i;
		descr += "; ";
	}
	descr += *i;
	descr += ".";
	return descr;
}

const Exception::Strings & Exception::getContext() const {
	return context;
}

const Exception::Strings & Exception::getDescriptions() const {
	return descriptions;
}

ostream & operator << (ostream &o, const Exception & e) {
	return o << e.getFullDescription();
}

// Exception::FontContext

Exception::FontContext::FontContext (const OpenTypeFile &font) {
	exceptionFont.push_back (font.getFileName());
}

Exception::FontContext::~FontContext() {
	exceptionFont.pop_back();
}

// Exception::Context

Exception::Context::Context (String contextDescription) {
	exceptionContext.push_back (contextDescription);
}

Exception::Context::~Context() {
	exceptionContext.pop_back();
}

} // end namespace OpenType
