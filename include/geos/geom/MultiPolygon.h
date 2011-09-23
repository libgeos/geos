/**********************************************************************
 * $Id: MultiPolygon.h 3211 2011-02-13 19:54:01Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2011 Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/MultiPolygon.java r320 (JTS-1.12)
 *
 **********************************************************************/

#ifndef GEOS_GEOS_MULTIPOLYGON_H
#define GEOS_GEOS_MULTIPOLYGON_H

#include <geos/export.h>
#include <string>
#include <vector>
#include <geos/platform.h>
#include <geos/geom/GeometryCollection.h> // for inheritance
#include <geos/geom/Polygonal.h> // for inheritance
#include <geos/geom/Dimension.h> // for Dimension::DimensionType

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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4250) // T1 inherits T2 via dominance
#endif

/// Models a collection of {@link Polygon}s.
//
/// As per the OGC SFS specification,
/// the Polygons in a MultiPolygon may not overlap,
/// and may only touch at single points.
/// This allows the topological point-set semantics
/// to be well-defined.
///
class GEOS_DLL MultiPolygon: public GeometryCollection, public Polygonal
{
public:

	friend class GeometryFactory;

	virtual ~MultiPolygon();

	/// Returns surface dimension (2)
	Dimension::DimensionType getDimension() const;

	/// Returns 1 (MultiPolygon boundary is MultiLineString)
	int getBoundaryDimension() const;

	/** \brief
	 * Computes the boundary of this geometry
	 *
	 * @return a lineal geometry (which may be empty)
	 * @see Geometry#getBoundary
	 */
	Geometry* getBoundary() const;

	std::string getGeometryType() const;

	virtual GeometryTypeId getGeometryTypeId() const;

	bool isSimple() const;

	bool equalsExact(const Geometry *other, double tolerance=0) const;

	Geometry *clone() const;

protected:

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

	MultiPolygon(const MultiPolygon &mp);
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace geos::geom
} // namespace geos

#ifdef GEOS_INLINE
# include "geos/geom/MultiPolygon.inl"
#endif

#endif // ndef GEOS_GEOS_MULTIPOLYGON_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/05/04 15:49:39  strk
 * updated all Geometry::getDimension() methods to return Dimension::DimensionType (closes bug#93)
 *
 * Revision 1.3  2006/04/28 10:55:39  strk
 * Geometry constructors made protected, to ensure all constructions use GeometryFactory,
 * which has been made friend of all Geometry derivates. getNumPoints() changed to return
 * size_t.
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
