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

#include <string>
#include <vector>
#include <geos/platform.h>
#include <geos/geom/Geometry.h>

#include <geos/inline.h>

namespace geos {
	namespace geom { // geos::geom
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

	/// Constructs a LineString taking ownership the given CoordinateSequence.
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
	virtual Point* getStartPoint() const;
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

	/// Normalize a LineString.  
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
	virtual Envelope* computeEnvelopeInternal() const;
	CoordinateSequence* points;
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
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
