/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/simplify/LinkedRing.h>
#include <geos/index/VertexSequencePackedRtree.h>

#include <queue>

namespace geos {
namespace geom {
class Envelope;
class LinearRing;
class LineString;
class Polygon;
}
namespace simplify {
class RingHullIndex;
}
}


using geos::geom::Coordinate;
using geos::geom::Envelope;
using geos::geom::LinearRing;
using geos::geom::LineString;
using geos::geom::Polygon;
using geos::index::VertexSequencePackedRtree;


namespace geos {
namespace simplify { // geos::simplify


class RingHull
{

public:

    /*
    * Creates a new instance.
    *
    * @param p_ring the ring vertices to process
    * @param p_isOuter whether the hull is outer or inner
    */
    RingHull(const LinearRing* p_ring, bool p_isOuter);

    void setMinVertexNum(std::size_t minVertexNum);

    void setMaxAreaDelta(double maxAreaDelta);

    const Envelope* getEnvelope() const;

    std::unique_ptr<LinearRing> getHull(RingHullIndex& hullIndex);

    static bool isConvex(const LinkedRing& vertexRing, std::size_t index);

    static double area(const LinkedRing& vertexRing, std::size_t index);

    void compute(RingHullIndex& hullIndex);

    std::unique_ptr<Polygon> toGeometry() const;


private:


    class Corner
    {

    private:

        std::size_t index;
        std::size_t prev;
        std::size_t next;
        double area;

    public:

        Corner(std::size_t p_idx, std::size_t p_prev, std::size_t p_next, double p_area)
            : index(p_idx)
            , prev(p_prev)
            , next(p_next)
            , area(p_area)
            {};

        inline int compareTo(const Corner& rhs) const {
            if (area == rhs.getArea()) {
                if (index == rhs.getIndex())
                    return 0;
                else return index < rhs.getIndex() ? -1 : 1;
            }
            else return area < rhs.getArea() ? -1 : 1;
        }

        inline bool operator< (const Corner& rhs) const {
            return compareTo(rhs) < 0;
        };

        inline bool operator> (const Corner& rhs) const {
            return compareTo(rhs) > 0;
        };

        inline bool operator==(const Corner& rhs) const {
            return compareTo(rhs) == 0;
        };

        bool isVertex(std::size_t p_index) const;
        std::size_t getIndex() const;
        double getArea() const;
        void envelope(const LinkedRing& ring, Envelope& env) const;
        bool intersects(const Coordinate& v, const LinkedRing& ring) const;
        bool isRemoved(const LinkedRing& ring) const;
        std::unique_ptr<LineString> toLineString(const LinkedRing& ring);

        struct Greater {
            inline bool operator()(const Corner & a, const Corner & b) const {
                return a > b;
            }
        };

        using PriorityQueue = std::priority_queue<Corner, std::vector<Corner>, Corner::Greater>;

    }; // class Corner



    const LinearRing* inputRing;
    double targetVertexNum = -1.0;
    double targetAreaDelta = -1.0;

    /**
    * The polygon vertices are provided in CW orientation.
    * Thus for convex interior angles
    * the vertices forming the angle are in CW orientation.
    */
    std::unique_ptr<CoordinateSequence> vertex;
    std::unique_ptr<LinkedRing> vertexRing;
    double areaDelta = 0;

    /**
    * Indexing vertices improves corner intersection testing performance.
    * The ring vertices are contiguous, so are suitable for a
    * {@link VertexSequencePackedRtree}.
    */
    std::unique_ptr<VertexSequencePackedRtree> vertexIndex;

    Corner::PriorityQueue cornerQueue;


    void init(CoordinateSequence& ring, bool isOuter);
    void addCorner(std::size_t i, Corner::PriorityQueue& cornerQueue);
    bool isAtTarget(const Corner& corner);

    /**
    * Removes a corner by removing the apex vertex from the ring.
    * Two new corners are created with apexes
    * at the other vertices of the corner
    * (if they are non-convex and thus removable).
    *
    * @param corner the corner to remove
    * @param cornerQueue the corner queue
    */
    void removeCorner(const Corner& corner, Corner::PriorityQueue& cornerQueue);
    bool isRemovable(const Corner& corner, const RingHullIndex& hullIndex) const;

    /**
    * Tests if any vertices in a hull intersect the corner triangle.
    * Uses the vertex spatial index for efficiency.
    *
    * @param corner the corner vertices
    * @param cornerEnv the envelope of the corner
    * @param hull the hull to test
    * @return true if there is an intersecting vertex
    */
    bool hasIntersectingVertex(
        const Corner& corner,
        const Envelope& cornerEnv,
        const RingHull* hull) const;

    const Coordinate& getCoordinate(std::size_t index) const;

    void query(
        const Envelope& cornerEnv,
        std::vector<std::size_t>& result) const;

    void queryHull(const Envelope& queryEnv, std::vector<Coordinate>& pts);




}; // RingHull


} // geos::simplify
} // geos
