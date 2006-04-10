/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOS_LINESTRING_H
#define GEOS_GEOS_LINESTRING_H

#include <geos/platform.h> // do we need this ?
#include <geos/geom/Geometry.h> // for inheritance
#include <geos/geom/CoordinateSequence.h> // for proper use of auto_ptr<>
#include <geos/geom/Envelope.h> // for proper use of auto_ptr<>

#include <string>
#include <vector>
#include <memory> // for auto_ptr

#include <geos/inline.h>

namespace geos {
	namespace geom {
		class Coordinate;
		class CoordinateArraySequence;
	}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \class LineString geom.h geos.h
 * \brief Basic implementation of LineString.
 */
class LineString: public Geometry {
public:

	/// A vector of const LineString pointers
	typedef std::vector<const LineString *> ConstVect;

	LineString(const LineString &ls);

	/// \brief
	/// Constructs a LineString taking ownership the
	/// given CoordinateSequence.
	LineString(CoordinateSequence *pts, const GeometryFactory *newFactory);

	virtual ~LineString();

	virtual Geometry *clone() const { return new LineString(*this); }

	virtual CoordinateSequence* getCoordinates() const;

	/// Returns a read-only pointer to internal CoordinateSequence
	const CoordinateSequence* getCoordinatesRO() const;

	virtual const Coordinate& getCoordinateN(int n) const;

	/// Returns line dimension (1)
	virtual int getDimension() const;

	/**
	 * \brief
	 * Returns Dimension::False for a closed LineString,
	 * 0 otherwise (LineString boundary is a MultiPoint)
	 */
	virtual int getBoundaryDimension() const;

	/**
	 * \brief
	 * Returns a MultiPoint.
	 * Empty for closed LineString, a Point for each vertex otherwise.
	 */
	virtual Geometry* getBoundary() const;

	virtual bool isEmpty() const;

	virtual int getNumPoints() const;

	virtual Point* getPointN(int n) const;

	/// \brief
	/// Return the start point of the LineString
	/// or NULL if this is an EMPTY LineString.
	///
	virtual Point* getStartPoint() const;

	/// \brief
	/// Return the end point of the LineString
	/// or NULL if this is an EMPTY LineString.
	///
	virtual Point* getEndPoint() const;

	virtual bool isClosed() const;

	virtual bool isRing() const;

	virtual std::string getGeometryType() const;

	virtual GeometryTypeId getGeometryTypeId() const;

	virtual bool isSimple() const;

	virtual bool isCoordinate(Coordinate& pt) const;

	virtual bool equalsExact(const Geometry *other, double tolerance=0)
		const;

	virtual void apply_rw(const CoordinateFilter *filter);

	virtual void apply_ro(CoordinateFilter *filter) const;

	virtual void apply_rw(GeometryFilter *filter);

	virtual void apply_ro(GeometryFilter *filter) const;

	virtual void apply_rw(GeometryComponentFilter *filter);

	virtual void apply_ro(GeometryComponentFilter *filter) const;

	/** \brief
	 * Normalizes a LineString. 
	 *
	 * A normalized linestring
	 * has the first point which is not equal to it's reflected point
	 * less than the reflected point.
	 */
	virtual void normalize();

	//was protected
	virtual int compareToSameClass(const Geometry *ls) const;

	virtual const Coordinate* getCoordinate() const;

	virtual double getLength() const;

	/**
	 * Creates a LineString whose coordinates are in the reverse
	 * order of this objects
	 *
	 * @return a LineString with coordinates in the reverse order
	 */
  	LineString* reverse() const;

protected:

	Envelope::AutoPtr computeEnvelopeInternal() const;

	std::auto_ptr<CoordinateSequence> points;
};

struct LineStringLT {
	bool operator()(const LineString *ls1, const LineString *ls2) const {
		return ls1->compareTo(ls2)<0;
	}
};


} // namespace geos::geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/LineString.inl"
//#endif

#endif // ndef GEOS_GEOS_LINESTRING_H

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/04/10 18:15:09  strk
 * Changed Geometry::envelope member to be of type auto_ptr<Envelope>.
 * Changed computeEnvelopeInternal() signater to return auto_ptr<Envelope>
 *
 * Revision 1.5  2006/04/10 17:35:44  strk
 * Changed LineString::points and Point::coordinates to be wrapped
 * in an auto_ptr<>. This should close bugs #86 and #89
 *
 * Revision 1.4  2006/04/05 10:25:21  strk
 * Fixed LineString constructor to ensure deletion of CoordinateSequence
 * argument on exception throw
 *
 * Revision 1.3  2006/03/31 16:55:17  strk
 * Added many assertions checking in LineString implementation.
 * Changed ::getCoordinate() to return NULL on empty geom.
 * Changed ::get{Start,End}Point() to return NULL on empty geom.
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
