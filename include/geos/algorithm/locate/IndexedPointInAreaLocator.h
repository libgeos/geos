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

#pragma once

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
class GEOS_DLL IndexedPointInAreaLocator : public PointOnGeometryLocator {
private:
    struct SegmentView {
        SegmentView(const geom::CoordinateXY* p0, const geom::CoordinateXY* p1) {
            // There is a significant performance benefit in fitting our
            // line segment into 8 bytes (about 15-20%). Because we know that
            // p1 follows p0 in a CoordinateSequence, we know that the address
            // of p1 is 16, 24, or 32 bytes greater than the address of p0.
            // By packing this offset into the least significant bits of p0,
            // we can retrieve both p0 and p1 while only using 8 byytes.
            std::size_t os = static_cast<std::size_t>(reinterpret_cast<const double*>(p1) - reinterpret_cast<const double*>(p0)) - 2u;
            m_p0 = reinterpret_cast<std::size_t>(p0) | os;

            assert(&this->p0() == p0);
            assert(&this->p1() == p1);
        }

        const geom::CoordinateXY& p0() const {
            auto ret = reinterpret_cast<const geom::CoordinateXY*>(m_p0 >> 2 << 2);
            return *ret;
        }

        const geom::CoordinateXY& p1() const {
            auto offset = (m_p0 & 0x03) + 2;
            auto ret = reinterpret_cast<const geom::CoordinateXY*>(reinterpret_cast<double*>(m_p0 >> 2 << 2) + offset);
            return *ret;
        }

        std::size_t m_p0;
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
    geom::Location locate(const geom::CoordinateXY* /*const*/ p) override;

};

} // geos::algorithm::locate
} // geos::algorithm
} // geos

