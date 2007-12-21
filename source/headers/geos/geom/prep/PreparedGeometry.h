/**********************************************************************
 * $Id
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

#ifndef GEOS_GEOM_PREP_PREPAREDGEOMETRY_H
#define GEOS_GEOM_PREP_PREPAREDGEOMETRY_H


// Forward declarations
namespace geos {
	namespace geom { 
		class Geometry;
	}
}


namespace geos {
namespace geom { // geos::geom
namespace prep { // geos::geom::prep

/**
 * \class PreparedGeometry
 *
 * \brief
 * An interface for classes which prepare {@link Geometry}s 
 * in order to optimize the performance 
 * of repeated calls to specific geometric operations.
 * <p>
 * A given implementation may provide optimized implementations
 * for only some of the specified methods, 
 * and delegate the remaining methods to the original {@link Geometry} operations.
 * An implementation may also only optimize certain situations,
 * and delegate others. 
 * See the implementing classes for documentation about which methods and situations
 * they optimize.
 * 
 * Last port: index/SpatialIndex.java (JTS-1.9) *
 */
class PreparedGeometry {
public:
	virtual ~PreparedGeometry() {};
	
	/**
	 * Gets the original {@link Geometry} which has been prepared.
	 * 
	 * @return the base geometry
	 */
	virtual const geom::Geometry & getGeometry() const =0;

	/**
	 * Tests whether the base {@link Geometry} contains a given geometry.
	 * 
	 * @param geom the Geometry to test
	 * @return true if this Geometry contains the given Geometry
	 * 
	 * @see Geometry#contains(Geometry)
	 */
	virtual bool contains(const geom::Geometry *geom) const =0;

	/**
	 * Tests whether the base {@link Geometry} contains a given geometry.
	 * <p>
	 * The <code>containsProperly</code> predicate has the following equivalent definitions:
	 * <ul>
	 * <li>Every point of the other geometry is a point of this geometry's interior.
	 * <li>The DE-9IM Intersection Matrix for the two geometries matches 
	 * <code>[T**FF*FF*]</code>
	 * </ul>
	 * The advantage to using this predicate is that it can be computed
	 * efficiently, with no need to compute topology at individual points.
	 * 
	 * @param geom the Geometry to test
	 * @return true if this Geometry containsProperly the given Geometry
	 * 
	 */
	virtual bool containsProperly(const geom::Geometry *geom) const =0;

	/**
	 * Tests whether the base {@link Geometry} is covered by a given geometry.
	 * 
	 * @param geom the Geometry to test
	 * @return true if this Geometry is covered by the given Geometry
	 * 
	 * @see Geometry#coveredBy(Geometry)
	 */
	virtual bool coveredBy(const geom::Geometry *geom) const =0;

	/**
	 * Tests whether the base {@link Geometry} covers a given geometry.
	 * 
	 * @param geom the Geometry to test
	 * @return true if this Geometry covers the given Geometry
	 * 
	 * @see Geometry#covers(Geometry)
	 */
	virtual bool covers(const geom::Geometry *geom) const =0;

	/**
	 * Tests whether the base {@link Geometry} crosses a given geometry.
	 * 
	 * @param geom the Geometry to test
	 * @return true if this Geometry crosses the given Geometry
	 * 
	 * @see Geometry#crosses(Geometry)
	 */
	virtual bool crosses(const geom::Geometry *geom) const =0;

	/**
	 * Tests whether the base {@link Geometry} is disjoint from a given geometry.
	 * 
	 * @param geom the Geometry to test
	 * @return true if this Geometry is disjoint from the given Geometry
	 * 
	 * @see Geometry#disjoint(Geometry)
	 */
	virtual bool disjoint(const geom::Geometry *geom) const =0;

	/**
	 * Tests whether the base {@link Geometry} intersects a given geometry.
	 * 
	 * @param geom the Geometry to test
	 * @return true if this Geometry intersects the given Geometry
	 * 
	 * @see Geometry#intersects(Geometry)
	 */
	virtual bool intersects(const geom::Geometry *geom) const =0;

	/**
	 * Tests whether the base {@link Geometry} overlaps a given geometry.
	 * 
	 * @param geom the Geometry to test
	 * @return true if this Geometry overlaps the given Geometry
	 * 
	 * @see Geometry#overlaps(Geometry)
	 */
	virtual bool overlaps(const geom::Geometry *geom) const =0;

	/**
	 * Tests whether the base {@link Geometry} touches a given geometry.
	 * 
	 * @param geom the Geometry to test
	 * @return true if this Geometry touches the given Geometry
	 * 
	 * @see Geometry#touches(Geometry)
	 */
	virtual bool touches(const geom::Geometry *geom) const =0;

	/**
	 * Tests whether the base {@link Geometry} is within a given geometry.
	 * 
	 * @param geom the Geometry to test
	 * @return true if this Geometry is within the given Geometry
	 * 
	 * @see Geometry#within(Geometry)
	 */
	virtual bool within(const geom::Geometry *geom) const =0;
};


} // namespace geos::geom::prep
} // namespace geos::geom
} // namespace geos


#endif // ndef GEOS_GEOM_PREP_PREPAREDGEOMETRY_H

/**********************************************************************
 * $Log$
 **********************************************************************/
