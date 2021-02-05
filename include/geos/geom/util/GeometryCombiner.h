/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006-2011 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/util/GeometryCombiner.java r320 (JTS-1.12)
 *
 **********************************************************************/

#ifndef GEOS_GEOM_UTIL_GEOMETRYCOMBINER_H
#define GEOS_GEOM_UTIL_GEOMETRYCOMBINER_H

#include <memory>
#include <vector>

#include <geos/export.h>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class GeometryFactory;
}
}

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

/** \brief
 * Combines [Geometrys](@ref Geometry) to produce a GeometryCollection
 * of the most appropriate type.
 *
 * Input geometries which are already collections will have their elements
 * extracted first.
 * No validation of the result geometry is performed.
 * (The only case where invalidity is possible is where polygonal geometries
 * are combined and result in a self-intersection).
 *
 * @see GeometryFactory#buildGeometry
 */
class GEOS_DLL GeometryCombiner {
public:
    /** \brief
     * Copies a collection of geometries and combines the result.
     *
     * @param geoms the geometries to combine (ownership left to caller)
     * @return the combined geometry
     */
    static std::unique_ptr<Geometry> combine(std::vector<const Geometry*> const& geoms);

    /** \brief
     * Combines a collection of geometries.
     *
     * @param geoms the geometries to combine (ownership transferred to combined geometry)
     * @return the combined geometry
     */
    static std::unique_ptr<Geometry> combine(std::vector<std::unique_ptr<Geometry>> && geoms);

    /** \brief
     * Copies two geometries and combines the result.
     *
     * @param g0 a geometry to combine (ownership left to caller)
     * @param g1 a geometry to combine (ownership left to caller)
     * @return the combined geometry
     */
    static std::unique_ptr<Geometry> combine(const Geometry* g0, const Geometry* g1);

    /** \brief
     * Combines two geometries.
     *
     * @param g0 a geometry to combine (ownership transferred to combined geometry)
     * @param g1 a geometry to combine (ownership transferred to combined geometry)
     * @return the combined geometry
     */
    static std::unique_ptr<Geometry> combine(std::unique_ptr<Geometry> && g0,
                                             std::unique_ptr<Geometry> && g1);

    /** \brief
     * Copies three geometries and combines the result.
     *
     * @param g0 a geometry to combine (ownership left to caller)
     * @param g1 a geometry to combine (ownership left to caller)
     * @param g2 a geometry to combine (ownership left to caller)
     * @return the combined geometry
     */
    static std::unique_ptr<Geometry> combine(const Geometry* g0, const Geometry* g1, const Geometry* g2);

    /** \brief
     * Combines three geometries.
     *
     * @param g0 a geometry to combine (ownership transferred to combined geometry)
     * @param g1 a geometry to combine (ownership transferred to combined geometry)
     * @param g2 a geometry to combine (ownership transferred to combined geometry)
     * @return the combined geometry
     */
    static std::unique_ptr<Geometry> combine(std::unique_ptr<Geometry> && g0,
                                             std::unique_ptr<Geometry> && g1,
                                             std::unique_ptr<Geometry> && g2);

private:
    std::vector<std::unique_ptr<Geometry>> inputGeoms;
    bool skipEmpty;

public:
    /** \brief
     * Creates a new combiner for a collection of geometries.
     *
     * @param geoms the geometries to combine
     */
    explicit GeometryCombiner(std::vector<const Geometry*> const& geoms);

    explicit GeometryCombiner(std::vector<std::unique_ptr<Geometry>> && geoms);

    /** \brief
     * Extracts the GeometryFactory used by the geometries in a collection.
     *
     * @return a GeometryFactory
     */
    GeometryFactory const* extractFactory() const;

    /** \brief
     * Computes the combination of the input geometries
     * to produce the most appropriate Geometry or GeometryCollection.
     *
     * @return a Geometry which is the combination of the inputs
     */
    std::unique_ptr<Geometry> combine();

    /** \brief
     * Set a flag indicating that empty geometries should be omitted from the result.
     */
    void setSkipEmpty(bool);

    // Declare type as noncopyable
    GeometryCombiner(const GeometryCombiner& other) = delete;
    GeometryCombiner& operator=(const GeometryCombiner& rhs) = delete;
};

} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

#endif
