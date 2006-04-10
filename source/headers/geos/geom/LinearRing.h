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

#ifndef GEOS_GEOS_LINEARRING_H
#define GEOS_GEOS_LINEARRING_H

#include <string>
#include <vector>
#include <geos/platform.h>
#include <geos/geom/LineString.h>

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom { // geos::geom
		class Coordinate;
		class CoordinateArraySequence;
	}
}

namespace geos {
namespace geom { // geos::geom

/**
 *
 * \brief Basic implementation of <code>LinearRing</code>.
 *
 * The first and last point in the coordinate sequence must be equal.
 * Either orientation of the ring is allowed.
 * A valid ring must not self-intersect.
 *
 */
class LinearRing : public LineString {

public:

	LinearRing(const LinearRing &lr);

	/**
	 * \brief Constructs a <code>LinearRing</code> with the given points.
	 *
	 * @param  points  points forming a closed and simple linestring, or
	 *      <code>null</code> or an empty array to create the empty
	 *      geometry.
	 *      This array must not contain <code>null</code> elements.
	 *	If not null LinearRing will take ownership of points.
	 *
	 * @param newFactory the GeometryFactory used to create this geometry
	 *
	 */
	LinearRing(CoordinateSequence* points, const GeometryFactory *newFactory);

	virtual Geometry *clone() const { return new LinearRing(*this); }
	virtual ~LinearRing();
	bool isSimple() const;
	std::string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
	bool isClosed() const;
	void setPoints(CoordinateSequence* cl);
private:
	void validateConstruction();
};


} // namespace geos::geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/LinearRing.inl"
//#endif

#endif // ndef GEOS_GEOS_LINEARRING_H

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/04/10 17:35:44  strk
 * Changed LineString::points and Point::coordinates to be wrapped
 * in an auto_ptr<>. This should close bugs #86 and #89
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
