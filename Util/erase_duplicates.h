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
	along with Foobar; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef ERASE_DUPLICATES_H
#define ERASE_DUPLICATES_H

namespace util {

	/// \brief Erase duplicates from a sorted container
	///
	/// The container must be a STL-compatible container that has iterators
	/// and begin() and end() methods.

	template <class Container>
		void erase_duplicates (Container & c)
	{
		typename Container::iterator i = c.begin();
		if (i != c.end()) {
			typename Container::iterator j = i;
			++ j;
			while (j != c.end()) {
				if (*i == *j)
					j = c.erase(j);
				else {
					i = j;
					++ j;
				}
			}
		}
	}
}

#endif	// ERASE_DUPLICATES_H
