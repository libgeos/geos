/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2018 Daniel Baston <dbaston@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_LOCATE_INDEXEDPOINTINAREALOCATOR_H
#define GEOS_ALGORITHM_LOCATE_INDEXEDPOINTINAREALOCATOR_H

#include <geos/geom/LineSegment.h>
#include <geos/algorithm/locate/PointOnGeometryLocator.h> // inherited
#include <geos/index/ItemVisitor.h> // inherited
#include <geos/index/strtree/TemplateSTRtree.h>

#include <memory>
#include <vector> // composition

namespace geos {
namespace algorithm {
class RayCrossingCounter;
}
namespace geom {
class Geometry;
class Coordinate;
class CoordinateSequence;
}
}

namespace geos {
namespace algorithm { // geos::algorithm
namespace locate { // geos::algorithm::locate

/** \brief
 * Determines the location of [Coordinates](@ref geom::Coordinate) relative to
 * an areal geometry, using indexing for efficiency.
 *
 * The Location is computed precisely, in that points located on the geometry boundary
 * or segments will return [geom::Location::BOUNDARY](@ref geom::Location).
 *
 * Polygonal and [LinearRing](@ref geom::LinearRing) geometries are supported.
 *
 * The index is lazy-loaded, which allows creating instances even if they are not used.
 *
 */
class IndexedPointInAreaLocator : public PointOnGeometryLocator {
private:
    struct SegmentView {
        SegmentView(const geom::Coordinate* p_p0, const geom::Coordinate* p_p1) : m_p0(p_p0) {
            // All GEOS CoordinateSequences store their coordinates sequentially.
            // Should that ever change, this assert will fail.
            (void) p_p1;
            assert(p_p0 + 1 == p_p1);
        }

        const geom::Coordinate& p0() const {
            return *m_p0;
        }

        const geom::Coordinate& p1() const {
            return *(m_p0 + 1);
        }

        const geom::Coordinate* m_p0;
    };

    class IntervalIndexedGeometry {
    private:

        index::strtree::TemplateSTRtree<SegmentView, index::strtree::IntervalTraits> index;

        void init(const geom::Geometry& g);
        void addLine(const geom::CoordinateSequence* pts);

    public:
        IntervalIndexedGeometry(const geom::Geometry& g);

        template<typename Visitor>
        void query(double min, double max, Visitor&& f) {
            index.query(index::strtree::Interval(min, max), f);
        }
    };

    const geom::Geometry& areaGeom;
    std::unique_ptr<IntervalIndexedGeometry> index;

    void buildIndex(const geom::Geometry& g);

    // Declare type as noncopyable
    IndexedPointInAreaLocator(const IndexedPointInAreaLocator& other) = delete;
    IndexedPointInAreaLocator& operator=(const IndexedPointInAreaLocator& rhs) = delete;

public:
    /** \brief
     * Creates a new locator for a given [Geometry](@ref geom::Geometry).
     *
     * Polygonal and [LinearRing](@ref geom::LinearRing) geometries are supported.
     *
     * @param g the Geometry to locate in
     */
    IndexedPointInAreaLocator(const geom::Geometry& g);

    const geom::Geometry&  getGeometry() const {
        return areaGeom;
    }

    /** \brief
     * Determines the [Location](@ref geom::Location) of a point in an areal
     * [Geometry](@ref geom::Geometry).
     *
     * @param p the point to test
     * @return the location of the point in the geometry
     */
    geom::Location locate(const geom::Coordinate* /*const*/ p) override;

};

} // geos::algorithm::locate
} // geos::algorithm
} // geos

#endif // GEOS_ALGORITHM_LOCATE_INDEXEDPOINTINAREALOCATOR_H
