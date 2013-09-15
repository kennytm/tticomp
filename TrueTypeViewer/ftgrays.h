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

// This file was taken from the FreeType library version 2.1.3.
// See www.freetype.org for more information.

/***************************************************************************/
/*                                                                         */
/*  ftgrays.h                                                              */
/*                                                                         */
/*    FreeType smooth renderer declaration                                 */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTGRAYS_H__
#define __FTGRAYS_H__

#ifdef __cplusplus
  extern "C" {
#endif


#ifdef _STANDALONE_
#include "ftimage.h"
#else
#include <ft2build.h>
#include FT_IMAGE_H
#endif


  /*************************************************************************/
  /*                                                                       */
  /* To make ftgrays.h independent from configuration files we check       */
  /* whether FT_EXPORT_VAR has been defined already.                       */
  /*                                                                       */
  /* On some systems and compilers (Win32 mostly), an extra keyword is     */
  /* necessary to compile the library as a DLL.                            */
  /*                                                                       */
#ifndef FT_EXPORT_VAR
#define FT_EXPORT_VAR( x )  extern  x
#endif

  FT_EXPORT_VAR( const FT_Raster_Funcs )  ft_grays_raster;


#ifdef __cplusplus
  }
#endif

#endif /* __FTGRAYS_H__ */


/* END */
