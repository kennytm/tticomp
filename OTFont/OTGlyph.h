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
	\file OTGlyph.h Defines an OpenType glyph.
*/

#ifndef OTGLYPH_H
#define OTGLYPH_H

#ifdef _MSC_VER
// Disable "type name to long to fit in debug information file" warning on Visual C++
#pragma warning(disable:4786)
#endif

#include <vector>
#include "../Util/shared_vector.h"
#include "../Util/smart_ptr.h"
#include "../Util/loop.h"
#include "../Util/return_type_specifier.h"

#include "OpenType.h"
#include "OThmtxTable.h"

namespace OpenType {

	/// \brief Defines a smart_ptr to a Glyph.
	typedef util::smart_ptr <Glyph> GlyphPtr;
	/// \brief Defines a list of GlyphPtr objects.
	typedef std::vector <GlyphPtr> Glyphs;

	/*** Contour point ***/

	/// \brief Defines a TrueType glyph contour point.
	struct ContourPoint {
		typedef Short coordinate_type;
		Short x;
		Short y;
		bool onCurve;
	};

	typedef util::loop <ContourPoint> Contour;

	template <typename Point> class Curve;

	class Contours : public util::shared_vector <Contour> {
	public:
		Contours () {}
		Contours (size_t n) : util::shared_vector <Contour> (n) {}
		Contours (size_t n, const Contour & t) : util::shared_vector <Contour> (n, t) {}
		Contours (const Contours & c) : util::shared_vector <Contour> (c) {}

		size_t getPointNum() const;
		ContourPoint & getPoint (size_t index);
		const ContourPoint & getPoint (size_t index) const;
	};

	/*** Composite component ***/

	/**
		\brief A TrueType composite glyph component.
	*/
	class CompositeComponent {
	public:
		enum Flags {
			cfNone				= 0,
			cfArgsAreWords		= 0x0001,
			cfArgsAreXYValues	= 0x0002,
			cfRoundXYToGrid		= 0x0004,
			cfSimpleScale		= 0x0008,
			cfMoreComponents	= 0x0020,
			cfXYScale			= 0x0040,
			cfTwoByTwo			= 0x0080,
			cfInstructions		= 0x0100,
			cfUseMyMetrics		= 0x0200,
			cfOverlapCompound	= 0x0400,
			cfScaledOffset		= 0x0800,
			cfUnscaledOffset	= 0x1000,

			cfSave				= 0x1E04
		};
		struct Translation { Short x, y; };
		struct Scale { F2Dot14 xx, xy, yx, yy; };
	protected:
		OpenTypeFont &font;
		UShort glyphIndex;
		Flags flags;
		Scale scale;


		Contours getScaledContours() const;
		virtual void write (MemoryWritePen &pen, UShort extraFlags) const = 0;
		friend class CompositeGlyph;
	public:
		CompositeComponent (OpenTypeFont &aFont, UShort aGlyphIndex, Flags aFlags,
			Scale aScale);
		virtual ~CompositeComponent();

		/// \brief Return the scaled and translated contour points as they
		/// appear in the composite glyph
		virtual std::pair <Translation, Contours>
			getContours (const Contours & previous) const = 0;
		UShort getPointNum() const;
		UShort getContourNum() const;
		UShort getDepth() const;

		UShort getGlyphIndex() const { return glyphIndex; }
		Flags getFlags() const { return flags; }
		virtual UShort getAttachmentPoint1() const = 0;
		virtual UShort getAttachmentPoint2() const = 0;

		virtual Translation getTranslation() const = 0;
		Scale getScale() const { return scale; }
	};

	class PositionedCompositeComponent : public CompositeComponent {
		Translation translation;
	protected:
		virtual void write (MemoryWritePen &pen, UShort extraFlags) const;
	public:
		PositionedCompositeComponent (OpenTypeFont &aFont, UShort glyphIndex, Flags aFlags,
			Scale aScale, Translation aTranslation);
		virtual ~PositionedCompositeComponent();

		virtual std::pair <Translation, Contours>
			getContours (const Contours & previous) const;

		virtual UShort getAttachmentPoint1() const { return 0xFFFF; }
		virtual UShort getAttachmentPoint2() const { return 0xFFFF; }
		virtual Translation getTranslation() const { return translation; }
	};

	class AttachedCompositeComponent : public CompositeComponent {
		UShort p1, p2;
	protected:
		virtual void write (MemoryWritePen &pen, UShort extraFlags) const;
	public:
		AttachedCompositeComponent (OpenTypeFont &aFont, UShort glyphIndex, Flags aFlags,
			Scale aScale, UShort aP1, UShort aP2);
		virtual ~AttachedCompositeComponent();

		virtual std::pair <Translation, Contours>
			getContours (const Contours & previous) const;
		virtual UShort getAttachmentPoint1() const { return p1; }
		virtual UShort getAttachmentPoint2() const { return p2; }
		virtual Translation getTranslation() const;
	};

	typedef util::smart_ptr <CompositeComponent> ComponentPtr;
	typedef std::vector <ComponentPtr> Components;

	/*** Glyph ***/

	/**
		\brief An %OpenType glyph.
	*/
	class Glyph {
	public:
		struct BoundingBox {
			Short xMin, yMin, xMax, yMax;
		};

	private:
		util::String name;

	protected:
		friend class SimpleGlyph;
		mutable BoundingBox boundingBox;
		mutable bool dirtyBoundingBox;
		HorMetric horMetric;

		OpenTypeFont &font;

		// Non-empty glyph constructor
		Glyph (OpenTypeFont &aFont, util::String aName, HorMetric aHorMetric,
			MemoryPen &pen);

		MemoryBlockPtr instructions;

	protected:
		friend class OpenTypeFont;
		void recalculateBoundingBox () const;
		void recalculateBoundingBoxIfNecessary () const {
			if (dirtyBoundingBox)
				recalculateBoundingBox();
		}

		virtual void write (MemoryWritePen &pen) const;

		friend class maxpTable;
		virtual void getMaximumProfile (maxpTable *maxp) const;

		friend class headTable;
		virtual void getBoundingBox (headTable *head) const;

		friend class hheaTable;
		virtual void getHorizontalMaxima (hheaTable *hhea) const;

	public:
		// Empty glyph constructor
		Glyph (OpenTypeFont &aFont, util::String aName, HorMetric aHorMetric);
		virtual ~Glyph();

		/// \brief Return the postscript name for this glyph.
		util::String getName() const;
		/// \brief Return the horizontal metric for this glyph.
		HorMetric getHorMetric() const;
		virtual bool isEmpty() const;
		virtual bool isComposite() const;
		virtual UShort getPointNum() const;
		virtual UShort getContourNum() const;
		virtual const Components & getComponents() const;
		virtual Contours getContours() const;
		virtual MemoryBlockPtr getInstructions() const;
		void setInstructions (MemoryBlockPtr aInstructions);
		virtual UShort getCompositeDepth() const;

		// get xMin - lsb
		Short getDisplacement() const;
		const BoundingBox & getBoundingBox();

		void setHorMetric (HorMetric aHm);
	};

	/*** SimpleGlyph ***/

	class SimpleGlyph : public Glyph {
		Contours contours;

	protected:
		virtual void getMaximumProfile (maxpTable *maxp) const;

	public:
		SimpleGlyph (OpenTypeFont &aFont, util::String aName, HorMetric aHorMetric,
			UShort contourNum, MemoryPen &pen);
		SimpleGlyph (OpenTypeFont &aFont, util::String aName, HorMetric aHorMetric,
			const Contours & aPoints);
		SimpleGlyph (const Glyph & glyph);
		virtual ~SimpleGlyph();

		virtual void write (MemoryWritePen &pen) const;
		virtual bool isEmpty() const;
		virtual UShort getPointNum() const;
		virtual UShort getContourNum() const;
		virtual Contours getContours() const;
		void setContours (const Contours &aContours);
	};

	/*** CompositeGlyph ***/

	class CompositeGlyph : public Glyph {
	public:
		typedef util::shared_vector <CompositeComponent::Translation> Translations;

	private:
		Components components;

	protected:
		virtual void getMaximumProfile (maxpTable *maxp) const;

	public:
		CompositeGlyph (OpenTypeFont &aFont, util::String aName, HorMetric aHorMetric,
			MemoryPen &pen);
		CompositeGlyph (OpenTypeFont &aFont, util::String aName, HorMetric aHorMetric,
			const Components &components);
		virtual ~CompositeGlyph();

		virtual void write (MemoryWritePen &pen) const;
		virtual bool isEmpty() const;
		virtual bool isComposite() const;
		virtual UShort getPointNum() const;
		virtual UShort getContourNum() const;
		virtual Contours getContours() const;
		virtual UShort getCompositeDepth() const;

		virtual const Components & getComponents() const;
		void addComponent (ComponentPtr newComponent);
		void setComponents (const Components &_components);
		Translations getTranslations() const;
	};
}
#endif // OTGLYPH_H
