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

using geos::geom::Coordinate;

namespace geos {   // geos
namespace noding { // geos::noding

class GEOS_DLL BoundaryChainNoder : public Noder {

private:

    class BoundarySegmentMap {

        private:

            // Members
            SegmentString* segString;
            std::vector<bool> isBoundary;

            static SegmentString* createChain(
                const SegmentString* segString,
                std::size_t startIndex,
                std::size_t endIndex);

            std::size_t findChainStart(std::size_t index) const;
            std::size_t findChainEnd(std::size_t index) const;

        public:

            BoundarySegmentMap(SegmentString* ss)
                : segString(ss) {
                    isBoundary.resize(ss->size()-1, false);
                };

            void setBoundarySegment(std::size_t index);
            void createChains(std::vector<SegmentString*>& chainList);
    };


    class Segment : public geom::LineSegment {

        private:

            BoundarySegmentMap& segMap;
            std::size_t index;

        public:

            Segment(const Coordinate& c0, const Coordinate& c1,
                BoundarySegmentMap& p_segMap, std::size_t p_index)
                : LineSegment(c0, c1)
                , segMap(p_segMap)
                , index(p_index)
            {
                normalize();
            };

            void markInBoundary() const {
                segMap.setBoundarySegment(index);
            };

            // struct HashCode {
            //     std::size_t operator()(const Segment& s) const {
            //         std::size_t h = std::hash<double>{}(s.p0.x);
            //         h ^= (std::hash<double>{}(s.p0.y) << 1);
            //         h ^= (std::hash<double>{}(s.p1.x) << 1);
            //         h ^= (std::hash<double>{}(s.p1.y) << 1);
            //         return h ^ (std::hash<std::size_t>{}(s.index) << 1);
            //     }
            // };
    };


public:

    // typedef std::unordered_set<Segment, Segment::HashCode> SegmentSet;
    typedef std::unordered_set<Segment, geos::geom::LineSegment::HashCode> SegmentSet;

    BoundaryChainNoder() : chainList(nullptr) {};

    // Noder virtual methods
    std::vector<SegmentString*>* getNodedSubstrings() const override;
    void computeNodes(std::vector<SegmentString*>* inputSegStrings) override;


private:

    // Members
    std::vector<SegmentString*>* chainList;

    // Methods
    static void addSegments(std::vector<SegmentString*>* segStrings,
        SegmentSet& segSet,
        std::vector<BoundarySegmentMap>& includedSegs);

    static void addSegments(SegmentString* segString,
        BoundarySegmentMap& segInclude,
        SegmentSet& segSet);

    static void markBoundarySegments(SegmentSet& segSet);

    static std::vector<SegmentString*>* extractChains(std::vector<BoundarySegmentMap>& sections);

    static bool segSetContains(SegmentSet& segSet, Segment& seg);

};

} // namespace geos::noding
} // namespace geos


