/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2025 Martin Davis
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

// #include <geos/noding/BasicSegmentString.h>


// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Geometry;
class GeometryFactory;
class LineString;
class LinearRing;
class Polygon;
}
}

namespace geos {     // geos.
namespace coverage { // geos.coverage

class GEOS_DLL CleanCoverage {

    using Coordinate = geos::geom::Coordinate;
    using CoordinateSequence = geos::geom::CoordinateSequence;
    using Geometry = geos::geom::Geometry;
    using GeometryFactory = geos::geom::GeometryFactory;
    using Polygon = geos::geom::Polygon;
    using LineString = geos::geom::LineString;
    using LinearRing = geos::geom::LinearRing;



public:

    CleanCoverage();


    class CleanArea {

    private:

        // Members
        std::vector<const Polygon*> polys;
        Envelope env;

    public:

        // Methods
        void add(const Polygon* poly);
        const Envelope* getEnvelope();
        double getBorderLength(const Polygon* adjPoly);
        double getArea();
        bool isAdjacent(RelateNG& rel);
        std::unique_ptr<Geometry> union();

    }; // CleanArea


    class MergeStrategy {

    public:

        virtual std::size_t getTarget() const = 0;

        virtual void checkMergeTarget(
            std::size_t areaIndex,
            CleanArea* cleanArea,
            const Polygon* poly) = 0;

    }; // MergeStrategy


    class BorderMergeStrategy : public MergeStrategy {

    private:

        std::size_t m_targetIndex = INDEX_UNKNOWN;
        double m_targetBorderLen;

    public:

        std::size_t getTarget()  const override {
            return m_targetIndex;
        }

        void checkMergeTarget(std::size_t areaIndex, CleanArea* area, const Polygon* poly) override {
            double borderLen = area == nullptr ? 0.0 : area->getBorderLength(poly);
            if (m_targetIndex == INDEX_UNKNOWN || borderLen > m_targetBorderLen) {
                m_targetIndex = areaIndex;
                m_targetBorderLen = borderLen;
            }
        }

    }; // BorderStrategy


    class AreaMergeStrategy : public MergeStrategy {

    private:

        std::size_t m_targetIndex = INDEX_UNKNOWN;
        double m_targetArea;
        bool m_isMax;

    public:

        AreaMergeStrategy(bool isMax) : m_isMax(isMax) {};

        std::size_t getTarget()  const override {
            return m_targetIndex;
        }

        void checkMergeTarget(std::size_t areaIndex, CleanArea* area, const Polygon* poly) override {
            double areaVal = area == nullptr ? 0.0 : area->getArea();
            bool isBetter = m_isMax
                ? areaVal > m_targetArea
                : areaVal < m_targetArea;
            if (m_targetIndex == INDEX_UNKNOWN || isBetter) {
                m_targetIndex = areaIndex;
                m_targetArea = areaVal;
            }
        }

    }; // AreaMergeStrategy


    class IndexMergeStrategy : public MergeStrategy {

    private:

        std::size_t m_targetIndex = INDEX_UNKNOWN;
        bool m_isMax;

    public:

        IndexMergeStrategy(bool isMax) : m_isMax(isMax) {};

        std::size_t getTarget()  const override {
            return m_targetIndex;
        }

        void checkMergeTarget(std::size_t areaIndex, CleanArea* area, const Polygon* poly) override {
            bool isBetter = m_isMax
                ? areaIndex > m_targetIndex
                : areaIndex < m_targetIndex;
            if (m_targetIndex < 0 || isBetter) {
                m_targetIndex = areaIndex;
            }
        }
    }; // MergeStrategy


private:

    // Members

    /**
     * The areas in the clean coverage.
     * Entries may be null, if no resultant corresponded to the input area.
     */
    std::vector<str::unique_ptr<CleanArea>> cov;
    //-- used for finding areas to merge gaps
    std::unique_ptr<Quadtree> covIndex = nullptr;


};

} // namespace geos.coverage
} // namespace geos





