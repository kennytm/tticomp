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

#ifndef OTGLYPHTOOUTLINE_H
#define OTGLYPHTOOUTLINE_H

#include "OTGlyph.h"

namespace OpenType {

	template <class Point> class PointCurve;
	template <class Point> class LineCurve;
	template <class Point> class QuadraticCurve;

	class ContourOutlineIterator {
		Contour::const_iterator i;

		bool validPosition (const Contour::const_iterator & i) const {
			if (i->onCurve)
				return true;
			else {
				Contour::const_iterator previous = i;
				previous --;
				return !previous->onCurve;
			}
		}

	public:
		explicit ContourOutlineIterator (const Contour::const_iterator & _i);
		ContourOutlineIterator (const ContourOutlineIterator & o);

		template <typename Point>
			util::smart_ptr <Curve <Point> > get (util::return_type_specifier <Point>) const
		{
			typedef typename Point::coordinate_type coordinate_type;

			Point firstPoint;
			firstPoint.x = i->x;
			firstPoint.y = i->y;

			if (i->onCurve)
			{
				firstPoint.onCurve = true;
				Contour::const_iterator next = i;
				++ next;
				if (next == i)
					return new PointCurve <Point> (firstPoint);
				else
				{
					Point nextPoint;
					nextPoint.x = next->x;
					nextPoint.y = next->y;
					nextPoint.onCurve = next->onCurve;

					if (next->onCurve)
						return new LineCurve <Point> (firstPoint, nextPoint);
					else
					{
						Contour::const_iterator nextnext = next;
						++ nextnext;

						Point nextnextPoint;

						if (nextnext->onCurve)
						{
							nextnextPoint.x = nextnext->x;
							nextnextPoint.y = nextnext->y;
						} else
						{
							nextnextPoint.x = coordinate_type (next->x + nextnext->x) / 2;
							nextnextPoint.y = coordinate_type (next->y + nextnext->y) / 2;
						}
						nextnextPoint.onCurve = true;
						return new QuadraticCurve <Point> (firstPoint, nextPoint, nextnextPoint);
					}
				}
			} else
			{
				Contour::const_iterator previous = i;
				-- previous;
				assert (!previous->onCurve);
				if (previous == i)
				{
					firstPoint.onCurve = true;
					return new PointCurve <Point> (firstPoint);
				} else
				{
					firstPoint.onCurve = false;

					Point previousPoint;
					previousPoint.x = coordinate_type (previous->x + i->x) / 2;
					previousPoint.y = coordinate_type (previous->y + i->y) / 2;
					previousPoint.onCurve = true;

					Contour::const_iterator next = i;
					++ next;
					Point nextPoint;
					nextPoint.onCurve = true;
					if (next->onCurve) {
						nextPoint.x = next->x;
						nextPoint.y = next->y;
					} else
					{
						nextPoint.x = coordinate_type (next->x + i->x) / 2;
						nextPoint.y = coordinate_type (next->y + i->y) / 2;
					}
					return new QuadraticCurve <Point> (previousPoint, firstPoint, nextPoint);
				}
			}
		}

		size_t getNextStepSize() const;

		ContourOutlineIterator & operator ++ ();
		ContourOutlineIterator & operator ++ (int) { return operator ++ (); }
		bool operator == (const ContourOutlineIterator _i) const { return i == _i.i; }
		bool operator != (const ContourOutlineIterator _i) const { return i != _i.i; }
	};
}

#endif	// OTGLYPHTOOUTLINE_H
