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

#ifndef RETURN_TYPE_SPECIFIER_H
#define RETURN_TYPE_SPECIFIER_H

/**
	\file return_type_specifier.h \brief Defines return_type_specifier.
*/

namespace util {

	/** \brief Workaround class for MS VC.

		Microsoft Visual C++ 6 does not support template methods
		(or rather, it does support them, but you can't call them with
		their template parameters because it will think it's a syntax error).
		You can pass this empty struct to a method to make it return
		a variable of type T.
	*/
	template <class T>
		struct return_type_specifier
	{
		typedef T return_type;
	};
}

#endif // RETURN_TYPE_SPECIFIER_H
