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
	\file OTTags.h %OpenType tag definitions.
*/

#ifndef OTTAGS_H
#define OTTAGS_H

#include "OpenType.h"

namespace OpenType {
	/// Convenience macro for defining %Opentype tags
	#define OT_MAKE_TAG(a,b,c,d) ((Byte)(a) + ((Byte)(b)<<8) + ((Byte)(c)<<16) + ((Byte)(d)<<24))

	enum {
	// Known table tags
		headTag = OT_MAKE_TAG ('h','e','a','d'),
		hheaTag = OT_MAKE_TAG ('h','h','e','a'),
		maxpTag = OT_MAKE_TAG ('m','a','x','p'),
		hmtxTag = OT_MAKE_TAG ('h','m','t','x'),
		locaTag = OT_MAKE_TAG ('l','o','c','a'),
		postTag = OT_MAKE_TAG ('p','o','s','t'),
		glyfTag = OT_MAKE_TAG ('g','l','y','f'),
		cmapTag = OT_MAKE_TAG ('c','m','a','p'),
		OS2Tag  = OT_MAKE_TAG ('O','S','/','2'),
		GSUBTag = OT_MAKE_TAG ('G','S','U','B'),
		GPOSTag = OT_MAKE_TAG ('G','P','O','S'),
		GDEFTag = OT_MAKE_TAG ('G','D','E','F'),

	// Unknown table tags
		LTSHTag = OT_MAKE_TAG ('L','T','S','H'),
		VDMXTag = OT_MAKE_TAG ('V','D','M','X'),
		hdmxTag = OT_MAKE_TAG ('h','d','m','x'),
		fpgmTag = OT_MAKE_TAG ('f','p','g','m'),
		prepTag = OT_MAKE_TAG ('p','r','e','p'),
		cvtTag  = OT_MAKE_TAG ('c','v','t',' '),
		kernTag = OT_MAKE_TAG ('k','e','r','n'),
		nameTag = OT_MAKE_TAG ('n','a','m','e'),
		gaspTag = OT_MAKE_TAG ('g','a','s','p'),
		PCLTTag = OT_MAKE_TAG ('P','C','L','T'),
		DSIGTag = OT_MAKE_TAG ('D','S','I','G'),

	// Language table tags
		DFLTTag = OT_MAKE_TAG ('D','F','L','T')
	};
}

#endif // OTTAGS_H
