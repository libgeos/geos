/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) Martin Davis 2022
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/noding/Noder.h>     // for composition
#include <geos/noding/SegmentString.h>     // for composition
#include <geos/geom/LineSegment.h> // for composition
#include <geos/noding/BasicSegmentString.h>

#include <unordered_set>

// Forward declarations
namespace geos {
namespace geom {
class CoordinateSequence;
class Coordinate;
}
namespace noding {
class NodedSegmentString;
}
}

namespace geos {   // geos
namespace noding { // geos::noding

/**
 * A noder which extracts chains of boundary segments
 * as SegmentStrings from a polygonal coverage.
 * Boundary segments are those which are not duplicated in the input polygonal coverage.
 * Extracting chains of segments minimize the number of segment strings created,
 * which produces a more efficient topological graph structure.
 * <p>
 * This enables fast overlay of polygonal coverages in CoverageUnion.
 * Using this noder is faster than SegmentExtractingNoder
 * and BoundarySegmentNoder.
 * <p>
 * No precision reduction is carried out.
 * If that is required, another noder must be used (such as a snap-rounding noder),
 * or the input must be precision-reduced beforehand.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL BoundaryChainNoder : public Noder {
    using Coordinate = geos::geom::Coordinate;

private:

    class BoundaryChainMap {

        private:

            // Members
            SegmentString* segString;
            std::vector<bool> isBoundary;

            static SegmentString* createChain(
                const SegmentString* segString,
                std::size_t startIndex,
                std::size_t endIndex,
                bool constructZ,
                bool constructM);

            std::size_t findChainStart(std::size_t index) const;
            std::size_t findChainEnd(std::size_t index) const;

        public:

            BoundaryChainMap(SegmentString* ss)
                : segString(ss) {
                    isBoundary.resize(ss->size()-1, false);
                };

            void setBoundarySegment(std::size_t index);
            void createChains(std::vector<SegmentString*>& chains, bool constructZ, bool constructM);
    };

    class Segment {
    public:
        Segment(const geom::CoordinateSequence& seq,
            BoundaryChainMap& segMap,
            std::size_t index)
            : m_seq(seq)
            , m_segMap(segMap)
            , m_index(index)
            , m_flip(seq.getAt<geom::CoordinateXY>(index).compareTo(seq.getAt<geom::CoordinateXY>(index + 1)) < 0)
        {}

        const geom::CoordinateXY& p0() const {
            return m_seq.getAt<geom::CoordinateXY>(m_flip ? m_index : m_index + 1);
        }

        const geom::CoordinateXY& p1() const {
            return m_seq.getAt<geom::CoordinateXY>(m_flip ? m_index + 1 : m_index);
        }

        void markBoundary() const {
            m_segMap.setBoundarySegment(m_index);
        };

        bool operator==(const Segment& other) const {
            return p0().equals2D(other.p0()) && p1().equals2D(other.p1());
        }

        struct HashCode {
            std::size_t operator()(const Segment& s) const {
                std::size_t h = std::hash<double>{}(s.p0().x);
                h ^= (std::hash<double>{}(s.p0().y) << 1);
                h ^= (std::hash<double>{}(s.p1().x) << 1);
                h ^= (std::hash<double>{}(s.p1().y) << 1);
                return h;
            }
        };

    private:
        const geom::CoordinateSequence& m_seq;
        BoundaryChainMap& m_segMap;
        std::size_t m_index;
        bool m_flip;
    };


public:

    using SegmentSet = std::unordered_set<Segment, Segment::HashCode>;

    BoundaryChainNoder()
        : m_chainList(nullptr)
        , m_constructZ(false)
        , m_constructM(false)
        {};

    // Noder virtual methods
    std::vector<SegmentString*>* getNodedSubstrings() const override;
    void computeNodes(std::vector<SegmentString*>* inputSegStrings) override;


private:

    // Members
    std::vector<SegmentString*>* m_chainList;
    std::vector<std::unique_ptr<geom::CoordinateSequence>> m_substrings;
    bool m_constructZ;
    bool m_constructM;

    // Methods
    void addSegments(std::vector<SegmentString*>* segStrings,
        SegmentSet& segSet,
        std::vector<BoundaryChainMap>& includedSegs);

    static void addSegments(SegmentString* segString,
        BoundaryChainMap& segInclude,
        SegmentSet& segSet);

    static bool segSetContains(
        SegmentSet& segSet, Segment& seg);

    static void markBoundarySegments(SegmentSet& segSet);

    std::vector<SegmentString*>* extractChains(std::vector<BoundaryChainMap>& sections) const;

    Coordinate::UnorderedSet findNodePts(
        const std::vector<SegmentString*>* segStrings) const;

    std::vector<SegmentString*>* nodeChains(
        const std::vector<SegmentString*>* chains,
        const Coordinate::UnorderedSet& nodePts);

    void nodeChain(
        SegmentString* chain,
        const Coordinate::UnorderedSet& nodePts,
        std::vector<SegmentString*>* nodedChains);

    std::size_t findNodeIndex(
        const SegmentString* chain,
        std::size_t start,
        const Coordinate::UnorderedSet& nodePts) const;

    noding::BasicSegmentString* substring(
        const SegmentString* segString,
        std::size_t start, std::size_t end);

    // Declared as non-copyable
    BoundaryChainNoder(const BoundaryChainNoder& other);
    BoundaryChainNoder& operator=(const BoundaryChainNoder& rhs);

};

} // namespace geos::noding
} // namespace geos
