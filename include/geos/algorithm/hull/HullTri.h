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

#include <geos/geom/Triangle.h>
#include <geos/triangulate/tri/Tri.h>
#include <geos/triangulate/tri/TriList.h>
#include <geos/triangulate/quadedge/TriangleVisitor.h>

#include <queue>
#include <deque>


namespace geos {
namespace geom {
class Coordinate;
}
namespace triangulate {
namespace quadedge {
}
}
}

using geos::geom::Coordinate;
using geos::geom::Triangle;
using geos::triangulate::tri::Tri;
using geos::triangulate::tri::TriList;

namespace geos {
namespace algorithm { // geos::algorithm
namespace hull {      // geos::algorithm::hull



class HullTri : public Tri
{
    private:

        double m_size;
        bool m_isMarked = false;

        bool isBoundaryTouch(TriIndex index) const;


    public:

        HullTri(const Coordinate& c0, const Coordinate& c1, const Coordinate& c2)
            : Tri(c0, c1, c2)
            , m_size(Triangle::longestSideLength(c0, c1, c2))
            {};

        class HullTriCompare {
            public:
                HullTriCompare() {};
                bool operator()(const HullTri* a, const HullTri* b)
                {
                    if (a->getSize() == b->getSize())
                        return a->getArea() < b->getArea();
                    else
                        return a->getSize() < b->getSize();
                }
        };


        double getSize() const;

        /**
        * Sets the size to be the length of the boundary edges.
        * This is used when constructing hull without holes,
        * by erosion from the triangulation border.
        */
        void setSizeToBoundary();

        void setSizeToLongestEdge();
        void setSizeToCircumradius();


        bool isMarked() const;
        void setMarked(bool marked);
        bool isRemoved();
        TriIndex boundaryIndex() const;
        TriIndex boundaryIndexCCW() const;
        TriIndex boundaryIndexCW() const;

        /**
        * Tests if a tri is the only one connecting its 2 adjacents.
        * Assumes that the tri is on the border of the triangulation
        * and that the triangulation does not contain holes
        *
        * @return true if the tri is the only connection
        */
        bool isConnecting() const;

        /**
        * Gets the index of a vertex which is adjacent to two other tris (if any).
        *
        * @return the vertex index, or -1
        */
        int adjacent2VertexIndex() const;

        /**
        * Tests whether some vertex of this Tri has degree = 1.
        * In this case it is not in any other Tris.
        *
        * @param tri
        * @param triList
        * @return true if a vertex has degree 1
        */
        TriIndex isolatedVertexIndex(TriList<HullTri>& triList) const;

        double lengthOfLongestEdge() const;

        /**
        * Tests if this tri has a vertex which is in the boundary,
        * but not in a boundary edge.
        *
        * @return true if the tri touches the boundary at a vertex
        */
        bool hasBoundaryTouch() const;

        static HullTri* findTri(TriList<HullTri>& triList, Tri* exceptTri);
        static bool isAllMarked(TriList<HullTri>& triList);
        static void clearMarks(TriList<HullTri>& triList);
        static void markConnected(HullTri* triStart, HullTri* exceptTri);
        static bool isConnected(TriList<HullTri>& triList, HullTri* exceptTri);

        friend std::ostream& operator<<(std::ostream& os, const HullTri& ht);

        double lengthOfBoundary() const;

        void remove(TriList<HullTri>& triList);


}; // HullTri






} // geos::algorithm::hull
} // geos::algorithm
} // geos

