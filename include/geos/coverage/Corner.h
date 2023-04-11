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

#include <geos/geom/Envelope.h>
#include <geos/simplify/LinkedLine.h>
#include <geos/export.h>

#include <vector>
#include <memory>
#include <queue>

namespace geos {
namespace simplify {
class LinkedLine;
}
namespace geom {
class Coordinate;
class LineString;
}
}


using geos::geom::Coordinate;
using geos::geom::Envelope;
using geos::geom::LineString;
using geos::simplify::LinkedLine;


namespace geos {
namespace coverage { // geos::coverage


class Corner
{

public:

    Corner(const LinkedLine* edge, std::size_t i);

    bool isVertex(std::size_t index) const;

    inline std::size_t getIndex() const {
        return m_index;
    }

    inline double getArea() const {
        return m_area;
    };

    const Coordinate& prev() const;
    const Coordinate& next() const;

    Envelope envelope() const;

    bool isVertex(const Coordinate& v) const;
    bool isBaseline(const Coordinate& p0, const Coordinate& p1) const;
    bool intersects(const Coordinate& v) const;
    bool isRemoved() const;

    const Coordinate& getCoordinate() {
        return m_edge->getCoordinate(m_index);
    }

    std::unique_ptr<LineString> toLineString() const;

    inline int compareTo(const Corner& rhs) const {
        double area_lhs = getArea();
        double area_rhs = rhs.getArea();

        if (area_lhs == area_rhs) {
            std::size_t index_lhs = getIndex();
            std::size_t index_rhs = rhs.getIndex();
            if (index_lhs == index_rhs) return 0;
            else return index_lhs < index_rhs ? -1 : 1;
        }
        else
            return area_lhs < area_rhs ? -1 : 1;
    }

    bool operator< (const Corner& rhs) const {
        return compareTo(rhs) < 0;
    };

    bool operator> (const Corner& rhs) const {
        return compareTo(rhs) > 0;
    };

    bool operator==(const Corner& rhs) const {
        return compareTo(rhs) == 0;
    };

    struct Greater {
        inline bool operator()(const Corner & a, const Corner & b) const {
            return a.compareTo(b) > 0;
        }
    };

    // Order using greater for compatibility with the Java PriorityQueue
    // implementation, which returns the smallest item off the top of the
    // queue
    using PriorityQueue = std::priority_queue<Corner, std::vector<Corner>, Corner::Greater>;


private:

    // members
    const LinkedLine* m_edge;
    std::size_t m_index;
    std::size_t m_prev;
    std::size_t m_next;
    double m_area;

    // methods
    static double area(const LinkedLine& edge, std::size_t index);

}; // Corner



GEOS_DLL std::ostream& operator<< (std::ostream& os, const Corner& c);


} // geos::coverage
} // geos
