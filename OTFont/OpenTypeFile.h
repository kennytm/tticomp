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
	\file OpenTypeFile.h Contains an %OpenType file, which only knows of table data.
	Any higher-level processing should be done in its decendents.
	See OpenTypeFont.h for an example of this.
*/

#ifndef OPENTYPEFILE_H
#define OPENTYPEFILE_H

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <vector>
#include <deque>

#include "../Util/String.h"

#include "OpenType.h"

namespace OpenType {

	typedef std::vector <TablePtr> Tables;

	/**
		\brief Contains logic to work with OpenType (sfnt) files.

		OpenTypeFile can be used to manipulate tables and table data.
		It is not meant to directly provide access to the data table; use
		a derivative class (e.g., OpenTypeFont) for that.
	*/

	class OpenTypeFile {
		Tables tables;

		std::deque <ExceptionPtr> warnings;
		Tables::iterator getTableIterator (ULong tag);

	protected:
		friend class Exception;
		util::String fileName;

		/// Return a pointer to the table, tables.end() if not found.
		TablePtr getTable (ULong tag, bool fail = true);
		/// Add a table to the file; returns false or throws if one exists already.
		bool addTable (TablePtr newTable, bool fail = true);
		/// Delete a table; returns false or throws if none exists.
		bool deleteTable (ULong tag, bool fail = true);
		/// Delete a table; returns false or throws if none exists. If !fail, the
		/// table is newly added to the font.
		bool replaceTable (TablePtr newTable, bool fail = true);

	public:
		OpenTypeFile ();
		virtual ~OpenTypeFile();

		/// \brief Read an OpenType font from file.
		///
		/// Reads the tables; does not try to read the information from them,
		/// however. Derivatives may do this as needed.
		/// The tables are kept in memory.
		void readFromFile (util::String aFileName);

		/// Return the current file name.
		util::String getFileName() const;

		/// \brief Write the font to file.
		///
		/// Derivatives may override this method to
		/// generate tables before actually writing them.
		virtual void writeToFile (util::String outFileName);

		/// \brief Add a warning.
		///
		/// This method needs to be overridden to show the
		/// message to the user.
		/// All warning messages generated when reading or using the font
		/// data are sent here.
		virtual void addWarning (ExceptionPtr aWarning) = 0;
	};

	util::String tagToString (Tag tag);

	/// \brief Compare two tags alphabetically and return true if t1<t2.
	bool compareTags (Tag t1, Tag t2);
}

#endif	// OPENTYPEFILE_H
