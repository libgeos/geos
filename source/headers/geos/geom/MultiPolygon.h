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

#ifndef GEOS_GEOS_MULTIPOLYGON_H
#define GEOS_GEOS_MULTIPOLYGON_H

#include <string>
#include <vector>
#include <geos/platform.h>
#include <geos/geom/GeometryCollection.h>

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom { // geos::geom
		class Coordinate;
		class CoordinateArraySequence;
		class MultiPoint;
	}
}


namespace geos {
namespace geom { // geos::geom

/// Basic implementation of <code>MultiPolygon</code>.
class MultiPolygon: public GeometryCollection {

public:

	/**
	 * \brief Construct a MultiPolygon
	 *
	 * @param newPolys
	 *	the <code>Polygon</code>s for this <code>MultiPolygon</code>,
	 *	or <code>null</code> or an empty array to create the empty
	 *	geometry. Elements may be empty <code>Polygon</code>s, but
	 *	not <code>null</code>s.
	 *	The polygons must conform to the assertions specified in the
	 *	<A HREF="http://www.opengis.org/techno/specs.htm">
	 *	OpenGIS Simple Features Specification for SQL
	 *	</A>.
	 *
	 *	Constructed object will take ownership of
	 *	the vector and its elements.
	 *
	 * @param newFactory
	 * 	The GeometryFactory used to create this geometry
	 *	Caller must keep the factory alive for the life-time
	 *	of the constructed MultiPolygon.
	 */
	MultiPolygon(std::vector<Geometry *> *newPolys, const GeometryFactory *newFactory);

	virtual ~MultiPolygon();

	/// Returns surface dimension (2)
	int getDimension() const;

	/// Returns 1 (MultiPolygon boundary is MultiLineString)
	int getBoundaryDimension() const;

	/**
	 * \brief
	 * Returns a MultiLineString composed of one LineString for
	 * each of the composing Polygon's shells and holes.
	 */
	Geometry* getBoundary() const;

	std::string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
	bool isSimple() const;
	bool equalsExact(const Geometry *other, double tolerance=0) const;

	MultiPolygon(const MultiPolygon &mp);
	Geometry *clone() const;

};


} // namespace geos::geom
} // namespace geos

#ifdef GEOS_INLINE
# include "geos/geom/MultiPolygon.inl"
#endif

#endif // ndef GEOS_GEOS_MULTIPOLYGON_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
