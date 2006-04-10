/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOM_GEOMETRYFACTORY_H
#define GEOS_GEOM_GEOMETRYFACTORY_H

#include <geos/inline.h>
#include <vector>

namespace geos {
	namespace geom { 
		class CoordinateSequenceFactory;
		class Coordinate;
		class CoordinateSequence;
		class Envelope;
		class Geometry;
		class GeometryCollection;
		class LineString;
		class LinearRing;
		class MultiLineString;
		class MultiPoint;
		class MultiPolygon;
		class Point;
		class Polygon;
		class PrecisionModel;
	}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \brief Supplies a set of utility methods for building Geometry objects
 * from CoordinateSequence or other Geometry objects.
 */
class GeometryFactory {
public:
	/**
	 * \brief 
	 * Constructs a GeometryFactory that generates Geometries having a
	 * floating PrecisionModel and a spatial-reference ID of 0.
	 */
	GeometryFactory();

	/**
	 * \brief
	 * Constructs a GeometryFactory that generates Geometries having
	 * the given PrecisionModel, spatial-reference ID, and
	 * CoordinateSequence implementation.
	 *
	 * NOTES:
	 * (1) the given PrecisionModel is COPIED
	 * (2) the CoordinateSequenceFactory is NOT COPIED
	 *     and must be available for the whole lifetime
	 *     of the GeometryFactory
	 */
	GeometryFactory(const PrecisionModel *pm, int newSRID,
		CoordinateSequenceFactory *nCoordinateSequenceFactory);

	/**
	 * \brief
	 * Constructs a GeometryFactory that generates Geometries having the
	 * given CoordinateSequence implementation, a double-precision floating
	 * PrecisionModel and a spatial-reference ID of 0.
	 */
	GeometryFactory(CoordinateSequenceFactory *nCoordinateSequenceFactory);

	/**
	 * \brief
	 * Constructs a GeometryFactory that generates Geometries having
	 * the given PrecisionModel and the default CoordinateSequence
	 * implementation.
	 *
	 * @param pm the PrecisionModel to use
	 */
	GeometryFactory(const PrecisionModel *pm);

	/**
	 * \brief
	 * Constructs a GeometryFactory that generates Geometries having
	 * the given {@link PrecisionModel} and spatial-reference ID,
	 * and the default CoordinateSequence implementation.
	 *
	 * @param pm the PrecisionModel to use
	 * @param newSRID the SRID to use
	 */
	GeometryFactory(const PrecisionModel* pm, int newSRID);

	/**
	 * \brief Copy constructor
	 *
	 * @param gf the GeometryFactory to clone from
	 */
	GeometryFactory(const GeometryFactory &gf);

	/**
	 * \brief 
	 * Return a pointer to the default GeometryFactory.
	 * This is a global shared object instantiated
	 * using default constructor.
	 */
	static const GeometryFactory*
	getDefaultInstance();

	/// Destructor
	virtual ~GeometryFactory();

//Skipped a lot of list to array convertors

	Point* createPointFromInternalCoord(const Coordinate* coord,
			const Geometry *exemplar) const;

	/// Converts an Envelope to a Geometry.
	//
	/// Returned Geometry can be a Point, a Polygon or an EMPTY geom.
	///
	Geometry* toGeometry(const Envelope* envelope) const;

	/// \brief
	/// Returns the PrecisionModel that Geometries created by this
	/// factory will be associated with.
	const PrecisionModel* getPrecisionModel() const;

	/// Creates an EMPTY Point
	Point* createPoint() const;

	/// Creates a Point using the given Coordinate
	Point* createPoint(const Coordinate& coordinate) const;

	/// Creates a Point taking ownership of the given CoordinateSequence
	Point* createPoint(CoordinateSequence *coordinates) const;

	/// Creates a Point with a deep-copy of the given CoordinateSequence.
	Point* createPoint(const CoordinateSequence &coordinates) const;

	/// Construct an EMPTY GeometryCollection
	GeometryCollection* createGeometryCollection() const;

	/// Construct the EMPTY Geometry
	Geometry* createEmptyGeometry() const;

	/// Construct a GeometryCollection taking ownership of given arguments
	GeometryCollection* createGeometryCollection(
			std::vector<Geometry *> *newGeoms) const;

	/// Constructs a GeometryCollection with a deep-copy of args
	GeometryCollection* createGeometryCollection(
			const std::vector<Geometry *> &newGeoms) const;

	/// Construct an EMPTY MultiLineString 
	MultiLineString* createMultiLineString() const;

	/// Construct a MultiLineString taking ownership of given arguments
	MultiLineString* createMultiLineString(
			std::vector<Geometry *> *newLines) const;

	/// Construct a MultiLineString with a deep-copy of given arguments
	MultiLineString* createMultiLineString(
			const std::vector<Geometry *> &fromLines) const;

	/// Construct an EMPTY MultiPolygon 
	MultiPolygon* createMultiPolygon() const;

	/// Construct a MultiPolygon taking ownership of given arguments
	MultiPolygon* createMultiPolygon(std::vector<Geometry *> *newPolys) const;

	/// Construct a MultiPolygon with a deep-copy of given arguments
	MultiPolygon* createMultiPolygon(
			const std::vector<Geometry *> &fromPolys) const;

	/// Construct an EMPTY LinearRing 
	LinearRing* createLinearRing() const;

	/// Construct a LinearRing taking ownership of given arguments
	LinearRing* createLinearRing(CoordinateSequence* newCoords) const;

	/// Construct a LinearRing with a deep-copy of given arguments
	LinearRing* createLinearRing(
			const CoordinateSequence& coordinates) const;

	/// Constructs an EMPTY <code>MultiPoint</code>.
	MultiPoint* createMultiPoint() const;

	/// Construct a MultiPoint taking ownership of given arguments
	MultiPoint* createMultiPoint(std::vector<Geometry *> *newPoints) const;

	/// Construct a MultiPoint with a deep-copy of given arguments
	MultiPoint* createMultiPoint(
			const std::vector<Geometry *> &fromPoints) const;

	/// \brief
	/// Construct a MultiPoint containing a Point geometry
	/// for each Coordinate in the given list.
	MultiPoint* createMultiPoint(
			const CoordinateSequence &fromCoords) const;

	/// Construct an EMPTY Polygon 
	Polygon* createPolygon() const;

	/// Construct a Polygon taking ownership of given arguments
	Polygon* createPolygon(LinearRing *shell,
			std::vector<Geometry *> *holes) const;

	/// Construct a Polygon with a deep-copy of given arguments
	Polygon* createPolygon(const LinearRing &shell,
			const std::vector<Geometry *> &holes) const;

	/// Construct an EMPTY LineString 
	LineString* createLineString() const;

	/// Construct a LineString taking ownership of given argument
	LineString* createLineString(CoordinateSequence* coordinates) const;

	/// Construct a LineString with a deep-copy of given argument
	LineString* createLineString(
			const CoordinateSequence& coordinates) const;

	/**
	 *  Build an appropriate <code>Geometry</code>, <code>MultiGeometry</code>, or
	 *  <code>GeometryCollection</code> to contain the <code>Geometry</code>s in
	 *  it.
	 *
	 *  For example:
	 *
	 *    - If <code>geomList</code> contains a single <code>Polygon</code>,
	 *      the <code>Polygon</code> is returned.
	 *    - If <code>geomList</code> contains several <code>Polygon</code>s, a
	 *      <code>MultiPolygon</code> is returned.
	 *    - If <code>geomList</code> contains some <code>Polygon</code>s and
	 *      some <code>LineString</code>s, a <code>GeometryCollection</code> is
	 *      returned.
	 *    - If <code>geomList</code> is empty, an empty
	 *      <code>GeometryCollection</code> is returned
	 *    .
	 *
	 * Note that this method does not "flatten" Geometries in the input,
	 * and hence if any MultiGeometries are contained in the input a
	 * GeometryCollection containing them will be returned.
	 *
	 * @param  newGeoms  the <code>Geometry</code>s to combine
	 *
	 * @return
	 *	A <code>Geometry</code> of the "smallest", "most type-specific"
	 *	class that can contain the elements of <code>geomList</code>.
	 *
	 * NOTE: the returned Geometry will take ownership of the
	 * 	given vector AND its elements 
	 */
	Geometry* buildGeometry(std::vector<Geometry *> *geoms) const;

	/** \brief
	 * This function does the same thing of the omonimouse function
	 * taking vector pointer instead of reference. 
	 *
	 * The difference is that this version will copy needed data
	 * leaving ownership to the caller.
	 */
	Geometry* buildGeometry(const std::vector<Geometry *> &geoms) const;
	
	int getSRID() const;

	/// \brief
	/// Returns the CoordinateSequenceFactory associated
	/// with this GeometryFactory
	const CoordinateSequenceFactory* getCoordinateSequenceFactory() const;

	/// Returns a clone of given Geometry.
	Geometry* createGeometry(const Geometry *g) const;

	/// Destroy a Geometry, or release it
	void destroyGeometry(Geometry *g) const;

private:
	const PrecisionModel* precisionModel;
	int SRID;
	const CoordinateSequenceFactory *coordinateListFactory;
};

} // namespace geos::geom
} // namespace geos

#ifdef GEOS_INLINE
# include "geos/geom/GeometryFactory.inl"
#endif

#endif // ndef GEOS_GEOM_GEOMETRYFACTORY_H

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/04/10 13:09:49  strk
 * Added GeometryFactory::defaultInstance()
 * Made Geometry::INTERNAL_GEOMETRY_FACTORY an alias for it
 * removed last deletion from Unload::Release class
 *
 * Revision 1.5  2006/03/31 17:51:26  strk
 * A few assertion checking, comments cleanup, use of initialization lists
 * in constructors, handled NULL parameters.
 *
 * Revision 1.4  2006/03/28 16:33:14  strk
 * Added note about args responsibility in GeometryFactory constructor
 *
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/20 17:27:03  strk
 * Bug #72 - Missing <vector> header
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

