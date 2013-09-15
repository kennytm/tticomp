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
	\file OTOutline.h represents a TrueType (quadratic) outline.
*/

#ifndef OTOUTLINE_H
#define OTOUTLINE_H

#include <vector>
#include <utility>
#include <cstdlib>
#include "../Util/loop.h"
#include "OTGlyph.h"
#include "OTGlyphToOutline.h"

namespace OpenType {

	template <typename coordinate_type>
		struct Point_conversion_trait {};

	template <>
		struct Point_conversion_trait <Short>
	{
		static Short toShort (Short i) { return i; }
	};

	template <typename Point> class QuadraticCurve;

	/// The type Point is a class or struct which defines attributes x and y
	/// of type Point::coordinate_type and bool onCurve.
	template <typename Point>
		class Curve 
	{
	public:
		typedef typename Point::coordinate_type T;
		typedef util::smart_ptr <QuadraticCurve <Point> > QuadraticCurvePtr;
		typedef util::smart_ptr <Curve <Point> > CurvePtr;
		typedef std::pair <CurvePtr, CurvePtr> TwoCurves;
	public:
		virtual const Point & getStartPoint() const = 0;
		virtual const Point & getEndPoint() const = 0;
		virtual bool hasMiddleXExtremum() const = 0;
		virtual bool hasMiddleYExtremum() const = 0;

		virtual void getBoundingBox (T & minX, T & minY, T & maxX, T & maxY) const = 0;
		virtual void getCBox (T & minX, T & minY, T & maxX, T & maxY) const = 0;

		virtual QuadraticCurvePtr asQuadratic() const = 0;
		virtual TwoCurves splitAtHalf() const = 0;
		virtual TwoCurves splitAtXExtremum() const = 0;
		virtual TwoCurves splitAtYExtremum() const = 0;

		/// Get contour points, skipping the last one.
		virtual std::vector <ContourPoint> getContourPoints() const = 0;

		virtual void transpose (T x, T y) = 0;
		virtual void transform (T xx, T xy, T yx, T yy) = 0;
		void scale (T x, T y) { transform (x, 0, 0, y); }

		virtual ~Curve() {}
	};

	/// PointCurve consists of just one point (usually an anchor point).
	/// It contains trivial implementations of the abstract members.
	template <typename Point>
		class PointCurve : public Curve <Point>
	{
	public:
		typedef typename Curve <Point>::T T;
		typedef typename Curve <Point>::QuadraticCurvePtr QuadraticCurvePtr;
		typedef typename Curve <Point>::CurvePtr CurvePtr;
		typedef typename Curve <Point>::TwoCurves TwoCurves;
	private:
		Point point;
	public:
		PointCurve (const Point & _point) : point (_point) {
			assert (point.onCurve);
		}
		PointCurve (const PointCurve & c) : point (c.point) {}

		virtual const Point & getStartPoint() const { return point; }
		virtual const Point & getEndPoint() const { return point; }
		virtual bool hasMiddleXExtremum() const { return false; }
		virtual bool hasMiddleYExtremum() const { return false; }

		virtual void getBoundingBox (T & minX, T & minY, T & maxX, T & maxY) const {
			minX = maxX = point.x;
			minY = maxY = point.y;
		}

		virtual void getCBox (T & minX, T & minY, T & maxX, T & maxY) const {
			minX = maxX = point.x;
			minY = maxY = point.y;
		}

		virtual QuadraticCurvePtr asQuadratic() const {
			Point off = point;
			off.onCurve = false;
			return new QuadraticCurve <Point> (point, off, point);
		}
		virtual TwoCurves splitAtHalf() const {
			return TwoCurves (new PointCurve (point), new PointCurve (point));
		}
		virtual TwoCurves splitAtXExtremum() const { assert (false); return TwoCurves(); }
		virtual TwoCurves splitAtYExtremum() const { assert (false); return TwoCurves(); }

		virtual std::vector <ContourPoint> getContourPoints() const {
			std::vector <ContourPoint> points;
			ContourPoint p;
			p.x = Point_conversion_trait <T>::toShort (point.x);
			p.y = Point_conversion_trait <T>::toShort (point.y);
			p.onCurve = true;
			points.push_back (p);
			return points;
		}


		virtual void transpose (T x, T y) {
			point.x += x;
			point.y += y;
		}

		virtual void transform (T xx, T xy, T yx, T yy) {
			T newX = xx * point.x + yx * point.y;
			T newY = xy * point.x + yy * point.y;
			point.x = newX;
			point.y = newY;
		}
	};

	/// PointCurve consists of just one line.
	template <typename Point>
		class LineCurve : public Curve <Point>
	{
	public:
		typedef typename Curve <Point>::T T;
		typedef typename Curve <Point>::QuadraticCurvePtr QuadraticCurvePtr;
		typedef typename Curve <Point>::CurvePtr CurvePtr;
		typedef typename Curve <Point>::TwoCurves TwoCurves;
	private:
		Point first, last;
	public:
		LineCurve (const Point & _first, const Point & _last)
			: first (_first), last (_last) {}

		LineCurve (const LineCurve & c)
			: first (c.first), last (c.last) {}

		virtual const Point & getStartPoint() const { return first; }
		virtual const Point & getEndPoint() const { return last; }
		virtual bool hasMiddleXExtremum() const { return false; }
		virtual bool hasMiddleYExtremum() const { return false; }

		virtual void getBoundingBox (T & minX, T & minY, T & maxX, T & maxY) const {
			minX = maxX = first.x;
			if (first.x < last.x)
				maxX = last.x;
			else
				minX = last.x;

			minY = maxY = first.y;
			if (first.y < last.y)
				maxY = last.y;
			else
				minY = last.y;
		}

		virtual void getCBox (T & minX, T & minY, T & maxX, T & maxY) const {
			minX = maxX = first.x;
			if (first.x < last.x)
				maxX = last.x;
			else
				minX = last.x;

			minY = maxY = first.y;
			if (first.y < last.y)
				maxY = last.y;
			else
				minY = last.y;
		}

		virtual QuadraticCurvePtr asQuadratic() const {
			Point off;
			off.x = (first.x + last.x) / 2;
			off.y = (first.y + last.y) / 2;
			off.onCurve = false;
			return new QuadraticCurve <Point> (first, off, last);
		}
		virtual TwoCurves splitAtHalf() const {
			Point middle;
			middle.x = (first.x + last.x) / 2;
			middle.y = (first.y + last.y) / 2;
			middle.onCurve = false;
			return TwoCurves (new LineCurve (first, middle), new LineCurve (middle, last));
		}
		virtual TwoCurves splitAtXExtremum() const { assert (false); return TwoCurves(); }
		virtual TwoCurves splitAtYExtremum() const { assert (false); return TwoCurves(); }

		virtual std::vector <ContourPoint> getContourPoints() const {
			std::vector <ContourPoint> points;
			ContourPoint p;
			p.x = Point_conversion_trait <T>::toShort (first.x);
			p.y = Point_conversion_trait <T>::toShort (first.y);
			p.onCurve = true;
			points.push_back (p);
			return points;
		}


		virtual void transpose (T x, T y) {
			first.x += x;
			first.y += y;
			last.x += x;
			last.y += y;
		}

		virtual void transform (T xx, T xy, T yx, T yy) {
			T newX = xx * first.x + yx * first.y;
			T newY = xy * first.x + yy * first.y;
			first.x = newX;
			first.y = newY;

			newX = xx * last.x + yx * last.y;
			newY = xy * last.x + yy * last.y;
			last.x = newX;
			last.y = newY;
		}
	};

	template <typename Point>
		class QuadraticCurve : public Curve <Point>
	{
	public:
		typedef typename Curve <Point>::T T;
		typedef typename Curve <Point>::QuadraticCurvePtr QuadraticCurvePtr;
		typedef typename Curve <Point>::CurvePtr CurvePtr;
		typedef typename Curve <Point>::TwoCurves TwoCurves;
	private:
		Point first, off, last;
	public:
		QuadraticCurve (const Point & _first, const Point & _off, const Point & _last)
			: first (_first), off (_off), last (_last)
		{
			assert (first.onCurve);
			assert (!off.onCurve);
			assert (last.onCurve);
		}

		QuadraticCurve (const QuadraticCurve & c)
			: first (c.first), off (c.off), last (c.last)
		{
			assert (first.onCurve);
			assert (!off.onCurve);
			assert (last.onCurve);
		}

		virtual const Point & getStartPoint() const { return first; }
		virtual const Point & getEndPoint() const { return last; }
		const Point & getOffPoint() const { return off; }

		virtual bool hasMiddleXExtremum() const {
			// We have an x extremum if 0 < t < 1 where t = (x0 - x1) / (x0 - 2x1 + x2)
			return ((first.x != off.x) && (first.x - 2 * off.x + last.x != 0) &&
				((first.x - off.x) < 0) == ((first.x - 2 * off.x + last.x) < 0) &&
				((first.x > off.x) == ((first.x - off.x) < (first.x - 2 * off.x + last.x))));
		}
		virtual bool hasMiddleYExtremum() const {
			// We have an y extremum if 0 < t < 1 where t = (y0 - y1) / (y0 - 2y1 + y2)
			return ((first.y != off.y) && (first.y - 2 * off.y + last.y != 0) &&
				((first.y - off.y) < 0) == ((first.y - 2 * off.y + last.y) < 0) &&
				((first.y > off.y) == ((first.y - off.y) < (first.y - 2 * off.y + last.y))));
		}

		virtual void getBoundingBox (T & minX, T & minY, T & maxX, T & maxY) const
		{
			minX = maxX = first.x;
			if (last.x > first.x)
				maxX = last.x;
			else
				maxX = first.x;

			// We have an x extremum if 0 < t < 1 where t = (x0 - x1) / (x0 - 2x1 + x2)
			if ((first.x != off.x) && (first.x - 2 * off.x + last.x != 0) &&
				((first.x - off.x) < 0) == ((first.x - 2 * off.x + last.x) < 0) &&
				((first.x > off.x) == ((first.x - off.x) < (first.x - 2 * off.x + last.x))))
			{
				T extremumX = (last.x * first.x - off.x * off.x) / (first.x - 2 * off.x + last.x);
				if (extremumX <= minX)
					minX = extremumX;
				else {
					assert (extremumX >= maxX);
					maxX = extremumX;
				}
			}

			// Exactly the same for y
			minY = maxY = first.y;
			if (last.y > first.y)
				maxY = last.y;
			else
				maxY = first.y;

			// We have an y extremum if 0 < t < 1 where t = (y0 - y1) / (y0 - 2y1 + y2)
			if ((first.y != off.y) && (first.y - 2 * off.y + last.y != 0) &&
				((first.y - off.y) < 0) == ((first.y - 2 * off.y + last.y) < 0) &&
				((first.y > off.y) == ((first.y - off.y) < (first.y - 2 * off.y + last.y))))
			{
				T extremumY = (last.y * first.y - off.y * off.y) / (first.y - 2 * off.y + last.y);
				if (extremumY <= minY)
					minY = extremumY;
				else {
					assert (extremumY >= maxY);
					maxY = extremumY;
				}
			}
		}

		virtual void getCBox (T & minX, T & minY, T & maxX, T & maxY) const {
			minX = maxX = first.x;
			if (first.x < last.x)
				maxX = last.x;
			else
				minX = last.x;

			if (off.x < minX)
				minX = off.x;
			if (off.x > maxX)
				maxX = off.x;

			minY = maxY = first.y;
			if (first.y < last.y)
				maxY = last.y;
			else
				minY = last.y;

			if (off.y < minY)
				minY = off.y;
			if (off.y > maxY)
				maxY = off.y;
		}

		virtual QuadraticCurvePtr asQuadratic() const {
			return new QuadraticCurve (*this);
		}

		virtual TwoCurves splitAtHalf() const {
			Point midPoint;
			midPoint.x = (first.x + 2 * off.x + last.x) / 4;
			midPoint.y = (first.y + 2 * off.y + last.y) / 4;
			midPoint.onCurve = true;
			Point oneOff;
			oneOff.x = (first.x + off.x) / 2;
			oneOff.y = (first.y + off.y) / 2;
			oneOff.onCurve = false;
			Point twoOff;
			twoOff.x = (off.x + last.x) / 2;
			twoOff.y = (off.y + last.y) / 2;
			twoOff.onCurve = false;

			return TwoCurves (new QuadraticCurve (first, oneOff, midPoint),
				new QuadraticCurve (midPoint, twoOff, last));
		}

		virtual TwoCurves splitAtXExtremum() const {
			assert (hasMiddleXExtremum());

			// Splitting at t = (x0 - x1) / (x0 - 2x1 + x2)

			Point oneOff;
			/*oneOff.x = first.x - first.x * (first.x - off.x) / (first.x - 2 * off.x + last.x) +
				off.x * (first.x - off.x) / (first.x - 2 * off.x + last.x);*/
			oneOff.x = first.x - ((first.x * first.x - first.x * off.x) / (first.x - 2 * off.x + last.x)) +
				((off.x * first.x - off.x * off.x) / (first.x - 2 * off.x + last.x));
			oneOff.y = first.y - ((first.y * first.x - first.y * off.x) / (first.x - 2 * off.x + last.x)) +
				((off.y * first.x - off.y * off.x) / (first.x - 2 * off.x + last.x));
			oneOff.onCurve = false;

			Point twoOff;
			twoOff.x = off.x - ((off.x * first.x - off.x * off.x) / (first.x - 2 * off.x + last.x)) +
				((last.x * first.x - last.x * off.x) / (first.x - 2 * off.x + last.x));
			twoOff.y = off.y - ((off.y * first.x - off.y * off.x) / (first.x - 2 * off.x + last.x)) +
				((last.y * first.x - last.y * off.x) / (first.x - 2 * off.x + last.x));
			twoOff.onCurve = false;

			Point midPoint;
			midPoint.x = oneOff.x - ((oneOff.x * first.x - oneOff.x * off.x) / (first.x - 2 * off.x + last.x)) +
				((twoOff.x * first.x - twoOff.x * off.x) / (first.x - 2 * off.x + last.x));
			midPoint.y = oneOff.y - ((oneOff.y * first.x - oneOff.y * off.x) / (first.x - 2 * off.x + last.x)) +
				((twoOff.y * first.x - twoOff.y * off.x) / (first.x - 2 * off.x + last.x));
			midPoint.onCurve = true;

			return TwoCurves (new QuadraticCurve (first, oneOff, midPoint),
				new QuadraticCurve (midPoint, twoOff, last));
		}

		virtual TwoCurves splitAtYExtremum() const {
			assert (hasMiddleYExtremum());

			// Splitting at t = (y0 - y1) / (y0 - 2y1 + y2)

			Point oneOff;
			oneOff.x = first.x - ((first.x * first.y - first.x * off.y) / (first.y - 2 * off.y + last.y)) +
				((off.x * first.y - off.x * off.y) / (first.y - 2 * off.y + last.y));
			oneOff.y = first.y - ((first.y * first.y - first.y * off.y) / (first.y - 2 * off.y + last.y)) +
				((off.y * first.y - off.y * off.y) / (first.y - 2 * off.y + last.y));
			oneOff.onCurve = false;

			Point twoOff;
			twoOff.x = off.x - ((off.x * first.y - off.x * off.y) / (first.y - 2 * off.y + last.y)) +
				((last.x * first.y - last.x * off.y) / (first.y - 2 * off.y + last.y));
			twoOff.y = off.y - ((off.y * first.y - off.y * off.y) / (first.y - 2 * off.y + last.y)) +
				((last.y * first.y - last.y * off.y) / (first.y - 2 * off.y + last.y));
			twoOff.onCurve = false;

			Point midPoint;
			midPoint.x = oneOff.x - ((oneOff.x * first.y - oneOff.x * off.y) / (first.y - 2 * off.y + last.y)) +
				((twoOff.x * first.y - twoOff.x * off.y) / (first.y - 2 * off.y + last.y));
			midPoint.y = oneOff.y - ((oneOff.y * first.y - oneOff.y * off.y) / (first.y - 2 * off.y + last.y)) +
				((twoOff.y * first.y - twoOff.y * off.y) / (first.y - 2 * off.y + last.y));
			midPoint.onCurve = true;

			return TwoCurves (new QuadraticCurve (first, oneOff, midPoint),
				new QuadraticCurve (midPoint, twoOff, last));
		}

		virtual std::vector <ContourPoint> getContourPoints() const {
			std::vector <ContourPoint> points;
			ContourPoint p;
			p.x = Point_conversion_trait <T>::toShort (first.x);
			p.y = Point_conversion_trait <T>::toShort (first.y);
			p.onCurve = true;
			points.push_back (p);
			p.x = Point_conversion_trait <T>::toShort (off.x);
			p.y = Point_conversion_trait <T>::toShort (off.y);
			p.onCurve = false;
			points.push_back (p);
			return points;
		}

		virtual void transpose (T x, T y) {
			first.x += x;
			first.y += y;
			off.x += x;
			off.y += y;
			last.x += x;
			last.y += y;
		}

		virtual void transform (T xx, T xy, T yx, T yy) {
			T newX = xx * first.x + yx * first.y;
			T newY = xy * first.x + yy * first.y;
			first.x = newX;
			first.y = newY;

			newX = xx * off.x + yx * off.y;
			newY = xy * off.x + yy * off.y;
			off.x = newX;
			off.y = newY;

			newX = xx * last.x + yx * last.y;
			newY = xy * last.x + yy * last.y;
			last.x = newX;
			last.y = newY;
		}
	};

	/*** Outline ***/

	template <class Point>
		class Outline : public util::loop <util::smart_ptr <Curve <Point> > >
	{
	public:
		typedef typename Point::coordinate_type coordinate_type;
		typedef typename util::loop <util::smart_ptr <Curve <Point> > >::const_iterator const_iterator;
		typedef typename util::loop <util::smart_ptr <Curve <Point> > >::iterator iterator;
	public:
		Outline() {}

		Outline (const Contour & contour) {
			ContourOutlineIterator begin (contour.begin());
			ContourOutlineIterator i = begin;
			do {
				push (i.get (util::return_type_specifier <Point>()));
				++ i;
			} while (i != begin);
		}

		Contour getContour() const {
			typedef std::vector <ContourPoint> ContourPoints;
			Contour contour;
			const_iterator i = this->begin();
			do {
				ContourPoints points = (*i)->getContourPoints();
				for (ContourPoints::const_iterator p = points.begin(); p != points.end(); p ++)
					contour.push (*p);
				++ i;
			} while (i != this->begin());

			// Do a bit of optimisation by deleting any on-curve points
			// right between two off-curve points

			if (contour.size() > 1) {
				Contour::iterator p = contour.begin();
				Contour::iterator previous = p;
				-- previous;
				Contour::iterator next = p;
				++ next;
				do {
					if (!previous->onCurve && p->onCurve && !next->onCurve &&
						abs (2 * p->x - previous->x - next->x) <= 1 &&
						abs (2 * p->y - previous->y - next->y) <= 1)
					{
						previous = contour.erase (p);
						p = previous;
						++ p;
						next = p;
						++ next;
					} else {
						previous = p;
						p = next;
						++ next;
					}
				} while (p != contour.begin());
			}

			return contour;
		}

		void getBoundingBox (coordinate_type & minX, coordinate_type & minY,
			coordinate_type & maxX, coordinate_type & maxY) const
		{
			const_iterator i = this->begin();
			(*i)->getBoundingBox (minX, minY, maxX, maxY);
			++ i;
			while (i != this->begin()) {
				coordinate_type localMinX, localMinY, localMaxX, localMaxY;
				(*i)->getBoundingBox (localMinX, localMinY, localMaxX, localMaxY);
				if (localMinX < minX)
					minX = localMinX;
				if (localMinY < minY)
					minY = localMinY;
				if (localMaxX > maxX)
					maxX = localMaxX;
				if (localMaxY > maxY)
					maxY = localMaxY;

				++ i;
			}
		}

		void transpose (coordinate_type x, coordinate_type y) {
			iterator i = this->begin();
			do {
				(*i)->transpose (x, y);
				++ i;
			} while (i != this->begin());
		}

		void transform (coordinate_type xx, coordinate_type xy,
			coordinate_type yx, coordinate_type yy)
		{
			iterator i = this->begin();
			do {
				(*i)->transform (xx, xy, yx, yy);
				++ i;
			} while (i != this->begin());
		}

		void addPointsAtExtremes() {
			coordinate_type minX, minY, maxX, maxY;
			getBoundingBox (minX, minY, maxX, maxY);
			iterator i = this->begin();
			do {
				coordinate_type localMinX, localMinY, localMaxX, localMaxY;
				(*i)->getCBox (localMinX, localMinY, localMaxX, localMaxY);
				if (localMinX < minX || localMaxX > maxX) {
					typename Curve <Point>::TwoCurves newCurves = (*i)->splitAtXExtremum();
					i = erase (i);
					i = insert (i, newCurves.second);
					i = insert (i, newCurves.first);
				} else {
					if (localMinY < minY || localMaxY > maxY) {
						typename Curve <Point>::TwoCurves newCurves = (*i)->splitAtYExtremum();
						i = erase (i);
						i = insert (i, newCurves.second);
						i = insert (i, newCurves.first);
					}
				}

				++ i;
			} while (i != this->begin());
		}
	};

	/*** Outlines ***/

	template <class Point>
		class Outlines : public std::vector <Outline <Point> >
	{
	public:
		typedef typename Point::coordinate_type coordinate_type;
		typedef typename std::vector <Outline <Point> >::const_iterator const_iterator;
		typedef typename std::vector <Outline <Point> >::iterator iterator;
	public:
		Outlines() {}
		
		Outlines (GlyphPtr glyph) {
			Contours contours = glyph->getContours();
			for (Contours::iterator contour = contours.begin(); contour != contours.end(); ++ contour) {
				push_back (Outline <Point> (*contour));
			}
		}

		Contours getContours() const {
			Contours contours;
			contours.reserve (this->size());
			for (const_iterator i = this->begin(); i != this->end(); ++ i)
				contours.push_back (i->getContour());
			return contours;
		}

		void transpose (coordinate_type x, coordinate_type y) {
			for (iterator i = this->begin(); i != this->end(); ++ i)
				i->transpose (x, y);
		}

		void transform (coordinate_type xx, coordinate_type xy,
			coordinate_type yx, coordinate_type yy)
		{
			for (iterator i = this->begin(); i != this->end(); ++ i)
				i->transform (xx, xy, yx, yy);
		}

		void addPointsAtExtremes() {
			for (iterator i = this->begin(); i != this->end(); ++ i)
				i->addPointsAtExtremes();
		}
	};

} // namespace OpenType

#endif	// OTOUTLINE_H
