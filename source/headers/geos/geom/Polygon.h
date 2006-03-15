/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/Polygon.java rev. 1.47
 *
 **********************************************************************/

#ifndef GEOS_GEOM_POLYGON_H
#define GEOS_GEOM_POLYGON_H

#include <string>
#include <vector>
#include <geos/platform.h>
#include <geos/geom/Geometry.h>

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom { // geos::geom
		class Coordinate;
		class CoordinateArraySequence;
		class LinearRing;
		class LineString;
	}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \class Polygon geom.h geos.h
 *
 * \brief Represents a linear polygon, which may include holes.
 *
 * The shell and holes of the polygon are represented by {@link LinearRing}s.
 * In a valid polygon, holes may touch the shell or other holes at a single point.
 * However, no sequence of touching holes may split the polygon into two pieces.
 * The orientation of the rings in the polygon does not matter.
 * <p>
 *  The shell and holes must conform to the assertions specified in the <A
 *  HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
 *  Specification for SQL</A> .
 *
 */
class Polygon: public Geometry{
public:

	/// A vector of const Polygon pointers
	typedef std::vector<const Polygon *> ConstVect;

	Polygon(const Polygon &p);
	virtual ~Polygon();

	/**
	 * Constructs a <code>Polygon</code> with the given exterior 
	 * and interior boundaries.
	 *
	 * @param  newShell  the outer boundary of the new Polygon,
	 *                   or <code>null</code> or an empty
	 *		     LinearRing if the empty geometry
	 *                   is to be created.
	 *
	 * @param  newHoles  the LinearRings defining the inner
	 *                   boundaries of the new Polygon, or
	 *                   null or empty LinearRing 
	 *                   if the empty  geometry is to be created.
	 *
	 * @param newFactory the GeometryFactory used to create this geometry
	 *
	 * Polygon will take ownership of Shell and Holes LinearRings 
	 */
	Polygon(LinearRing *newShell, std::vector<Geometry *> *newHoles,
		const GeometryFactory *newFactory);

	virtual Geometry *clone() const { return new Polygon(*this); }
	CoordinateSequence* getCoordinates() const;
	int getNumPoints() const;

	/// Returns surface dimension (2)
	int getDimension() const;

	/// Returns 1 (Polygon boundary is a MultiLineString)
	int getBoundaryDimension() const;

	/**
	 * \brief
	 * Returns a MultiLineString.
	 * One LineString for the shell and one for each hole.
	 * Empty for an empty Polygon.
	 */
	Geometry* getBoundary() const;

	bool isEmpty() const;

	/** \brief
	 * Tests if a valid polygon is simple.
	 * This method always returns true, since a valid polygon is always simple
	 *
	 * @return <code>true</code>
	 */
	bool isSimple() const;
	
	/// Returns the exterior ring (shell)
	const LineString* getExteriorRing() const;

	/// Returns number of interior rings (hole)
	int getNumInteriorRing() const;

	/// Get nth interior ring (hole)
	const LineString* getInteriorRingN(int n) const;

	std::string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
	bool equalsExact(const Geometry *other, double tolerance=0) const;
	void apply_rw(const CoordinateFilter *filter);
	void apply_ro(CoordinateFilter *filter) const;
	void apply_rw(GeometryFilter *filter);
	void apply_ro(GeometryFilter *filter) const;

	Geometry* convexHull() const;
	void normalize();
	int compareToSameClass(const Geometry *p) const; //was protected
	const Coordinate* getCoordinate() const;

	double getArea() const;

 	/// Returns the perimeter of this <code>Polygon</code>
	double getLength() const;

	void apply_rw(GeometryComponentFilter *filter);
	void apply_ro(GeometryComponentFilter *filter) const;

	bool isRectangle() const;

protected:

	LinearRing *shell;
	std::vector<Geometry *> *holes; //Actually vector<LinearRing *>
	Envelope* computeEnvelopeInternal() const;

private:

	void normalize(LinearRing *ring, bool clockwise);
};

} // namespace geos::geom
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/geom/Polygon.inl"
//#endif

#endif // ndef GEOS_GEOM_POLYGON_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/15 09:12:45  strk
 * isSimple doc
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
