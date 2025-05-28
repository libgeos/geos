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

#include <geos/geom/Envelope.h>
#include <geos/constants.h>
#include <geos/index/quadtree/Quadtree.h>

#include <vector>
#include <memory>

// Forward declarations
namespace geos {
namespace geom {
    class Geometry;
    class GeometryFactory;
    class LineString;
    class LinearRing;
    class Polygon;
}
namespace operation {
namespace relateng {
    class RelateNG;
}
}
namespace index {
namespace quadtree {
}
}
}


namespace geos {     // geos.
namespace coverage { // geos.coverage

class CleanCoverage {

    using Envelope = geos::geom::Envelope;
    using Geometry = geos::geom::Geometry;
    using GeometryFactory = geos::geom::GeometryFactory;
    using LineString = geos::geom::LineString;
    using LinearRing = geos::geom::LinearRing;
    using Polygon = geos::geom::Polygon;
    using RelateNG = geos::operation::relateng::RelateNG;
    using Quadtree = geos::index::quadtree::Quadtree;


public:

    // Classes

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
        std::unique_ptr<Geometry> getUnion();

    }; // CleanArea


    class MergeStrategy {

    public:

        virtual ~MergeStrategy() = default;

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

        BorderMergeStrategy() {};

        std::size_t getTarget()  const override {
            return m_targetIndex;
        };

        void checkMergeTarget(std::size_t areaIndex, CleanArea* area, const Polygon* poly) override {
            double borderLen = area == nullptr ? 0.0 : area->getBorderLength(poly);
            if (m_targetIndex == INDEX_UNKNOWN || borderLen > m_targetBorderLen) {
                m_targetIndex = areaIndex;
                m_targetBorderLen = borderLen;
            }
        };

    }; // BorderStrategy


    class AreaMergeStrategy : public MergeStrategy {

    private:

        std::size_t m_targetIndex = INDEX_UNKNOWN;
        double m_targetArea;
        bool m_isMax;

    public:

        AreaMergeStrategy(bool isMax) : m_isMax(isMax) {};

        std::size_t getTarget() const override {
            return m_targetIndex;
        }

        void checkMergeTarget(std::size_t areaIndex, CleanArea* area, const Polygon* poly) override {
            (void)poly;
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
            (void)area;
            (void)poly;
            bool isBetter = m_isMax
                ? areaIndex > m_targetIndex
                : areaIndex < m_targetIndex;
            if (isBetter) {
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
    std::vector<std::unique_ptr<CleanArea>> cov;
    //-- used for finding areas to merge gaps
    std::unique_ptr<Quadtree> covIndex = nullptr;

    void mergeGap(const Polygon* gap);

    CleanArea* findMaxBorderLength(const Polygon* poly, std::vector<CleanArea*>& areas);

    std::vector<CleanArea*> findAdjacentAreas(const Geometry* poly);

    void createIndex();


public:

    // Methods

    CleanCoverage(std::size_t size);

    void add(std::size_t i, const Polygon* poly);

    void mergeOverlap(const Polygon* overlap,
        MergeStrategy& mergeStrategy,
        std::vector<std::size_t>& parentIndexes);

    static std::size_t findMergeTarget(const Polygon* poly,
        MergeStrategy& strategy,
        std::vector<std::size_t>& parentIndexes,
        std::vector<std::unique_ptr<CleanArea>>& cov);

    void mergeGaps(std::vector<const Polygon*>& gaps);

    std::vector<std::unique_ptr<Geometry>> toCoverage(const GeometryFactory* geomFactory);

    /**
     * Disable copy construction and assignment. Apparently needed to make this
     * class compile under MSVC. (See https://stackoverflow.com/q/29565299)
     */
    CleanCoverage(const CleanCoverage&) = delete;
    CleanCoverage& operator=(const CleanCoverage&) = delete;


};

} // namespace geos.coverage
} // namespace geos





