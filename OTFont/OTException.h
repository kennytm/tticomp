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

// Some comments in this file may be parsed by doygen to yield nice
// documentation files for the source.

/**
	\file Provides an exception class for OpenType fonts.
*/

#ifndef OTEXCEPTION_H
#define OTEXCEPTION_H

#include <ostream>
#include <vector>

#include "../Util/String.h"

#include "OpenType.h"

namespace OpenType {

	class OpenTypeFile;

	/**
		\brief Exception provides generalised error handling throughout the OTFont
		library.
		
		It maintains a stack of the current context and when an
		Exception is constructed (be it by throwing it, or otherwise) the
		current context is saved in the Exception to provide more sensible
		data about the error that occurred.

		A function may define a Exception::Context or Exception::FontContext
		variable (which does not take any space on the stack) to define
		the current context.
	*/

	class Exception {
	protected:
		typedef std::vector <util::String> Strings;
		Strings context;
		Strings descriptions;
		Tag table;
	public:
		/// \brief Construct an Exception from a description.
		///
		/// Do not supply a period with the description.
		/// It will be added as needed.
		/// Do not give context information either; you should use
		/// the Context class for this.
		Exception (util::String description);
		virtual ~Exception();

		/// Add a description to the Exception. This may be used after the
		/// exception is caught and is propagated to add information about
		/// the consequence of the error like "Such and such feature will
		/// not be available".
		/// Do not add a period; such punctuation
		/// will be added automatically as needed.
		void addDescription (util::String description);

		/// \brief Return a string that contains a full description,
		/// including the context, of the error.
		///
		/// This is useful if you want to print the
		/// error to the console without much formatting.
		util::String getFullDescription() const;
		/// Return the context as a number of String objects. This is useful
		/// if you want to format the context (e.g., put every context item
		/// on a new line). Use getDescriptions() to get a list of descriptions.
		const Strings & getContext() const;
		/// Return a list of descriptions. The first description is the main
		/// descriptions; the other descriptions may supply extra information,
		/// for example about the consequence of the error.
		const Strings & getDescriptions() const;

		/// \brief FontContext makes it easy to specify the current
		/// font just in case there will be an error.
		class FontContext {
		public:
			FontContext (const OpenTypeFile &font);
			~FontContext();
		};

		/// \brief Context makes it easy to specify the current context just in
		/// case there will be an error.
		class Context {
		public:
			Context (util::String contextDescription);
			~Context();
		};
	};

	typedef util::smart_ptr <Exception> ExceptionPtr;

	std::ostream & operator << (std::ostream &, const Exception & e);
}

#endif	// OTEXCEPTION_H
