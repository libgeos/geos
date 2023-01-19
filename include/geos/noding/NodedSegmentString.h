/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 *
 **********************************************************************
 *
 * Last port: noding/NodedSegmentString.java r320 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h> // for inlines
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/NodableSegmentString.h> // for inheritance
#include <geos/noding/SegmentNode.h>
#include <geos/noding/SegmentNodeList.h>
#include <geos/noding/SegmentString.h>
#include <geos/util/IllegalArgumentException.h>

#include <cstddef>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251 4355) // warning C4355: 'this' : used in base member initializer list
#endif

namespace geos {
namespace noding { // geos::noding

/** \brief
 * Represents a list of contiguous line segments,
 * and supports noding the segments.
 *
 * The line segments are represented by an array of [Coordinates](@ref geom::Coordinate).
 * Intended to optimize the noding of contiguous segments by
 * reducing the number of allocated objects.
 * SegmentStrings can carry a context object, which is useful
 * for preserving topological or parentage information.
 * All noded substrings are initialized with the same context object.
 *
 */
class GEOS_DLL NodedSegmentString : public NodableSegmentString {
public:

    // TODO: provide a templated method using an output iterator
    template <class II>
    static void
    getNodedSubstrings(II from, II too_far,
                       SegmentString::NonConstVect* resultEdgelist)
    {
        for(II i = from; i != too_far; ++i) {
            NodedSegmentString* nss = dynamic_cast<NodedSegmentString*>(*i);
            assert(nss);
            nss->getNodeList().addSplitEdges(resultEdgelist);
        }
    }

    template <class C>
    static void
    getNodedSubstrings(C* segStrings,
                       SegmentString::NonConstVect* resultEdgelist)
    {
        getNodedSubstrings(segStrings->begin(), segStrings->end(), resultEdgelist);
    }

    static void getNodedSubstrings(const SegmentString::NonConstVect& segStrings,
                                   SegmentString::NonConstVect* resultEdgeList);

    /// Returns allocated object
    static SegmentString::NonConstVect* getNodedSubstrings(
        const SegmentString::NonConstVect& segStrings);

    std::unique_ptr<geom::CoordinateSequence> getNodedCoordinates();

    bool hasNodes() const
    {
        return nodeList.size() > 0;
    }

    /** \brief
     * Creates a new segment string from a list of vertices.
     *
     * @param newPts CoordinateSequence representing the string,
     *               ownership transferred.
     * @param constructZ should newly-constructed coordinates store Z values?
     * @param constructM should newly-constructed coordinates store M values?
     * @param newContext the user-defined data of this segment string
     *                   (may be null)
     */
    NodedSegmentString(geom::CoordinateSequence* newPts, bool constructZ, bool constructM, const void* newContext)
        : NodableSegmentString(newContext, newPts)
        , nodeList(*this, constructZ, constructM)
    {}

    NodedSegmentString(SegmentString* ss, bool constructZ, bool constructM)
        : NodableSegmentString(ss->getData(), ss->getCoordinates()->clone().release())
        , nodeList(*this, constructZ, constructM)
    {}

    ~NodedSegmentString() override {
        delete seq;
    }

    SegmentNodeList& getNodeList();

    const SegmentNodeList& getNodeList() const;

    std::unique_ptr<geom::CoordinateSequence> releaseCoordinates();

    std::ostream& print(std::ostream& os) const override;


    /** \brief
     * Add {@link SegmentNode}s for one or both
     * intersections found for a segment of an edge to the edge
     * intersection list.
     */
    void addIntersections(algorithm::LineIntersector* li,
        std::size_t segmentIndex, std::size_t geomIndex)
    {
        for (std::size_t i = 0, n = li->getIntersectionNum(); i < n; ++i) {
            addIntersection(li, segmentIndex, geomIndex, i);
        }
    };

    /** \brief
     * Add an SegmentNode for intersection intIndex.
     *
     * An intersection that falls exactly on a vertex
     * of the SegmentString is normalized
     * to use the higher of the two possible segmentIndexes
     */
    void addIntersection(algorithm::LineIntersector* li,
        std::size_t segmentIndex,
        std::size_t geomIndex, std::size_t intIndex)
    {
        ::geos::ignore_unused_variable_warning(geomIndex);

        const auto& intPt = li->getIntersection(intIndex);
        addIntersection(intPt, segmentIndex);
    };

    /** \brief
     * Add an SegmentNode for intersection intIndex.
     *
     * An intersection that falls exactly on a vertex of the
     * edge is normalized
     * to use the higher of the two possible segmentIndexes
     */
    template<typename CoordType>
    void addIntersection(const CoordType& intPt,
        std::size_t segmentIndex)
    {
        std::size_t normalizedSegmentIndex = segmentIndex;

        if (segmentIndex > size() - 2) {
            throw util::IllegalArgumentException("SegmentString::addIntersection: SegmentIndex out of range");
        }

        // normalize the intersection point location
        auto nextSegIndex = normalizedSegmentIndex + 1;
        if (nextSegIndex < size()) {
            const auto& nextPt = getCoordinate<geom::CoordinateXY>(nextSegIndex);

            // Normalize segment index if intPt falls on vertex
            // The check for point equality is 2D only -
            // Z values are ignored
            if(intPt.equals2D(nextPt)) {
                normalizedSegmentIndex = nextSegIndex;
            }
        }

        /*
         * Add the intersection point to edge intersection list
         * (unless the node is already known)
         */
        nodeList.add(intPt, normalizedSegmentIndex);
    }

private:

    SegmentNodeList nodeList;

};

} // namespace geos::noding
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif











