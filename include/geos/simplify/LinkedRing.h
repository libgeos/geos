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

#include <memory>

#include <geos/geom/Coordinate.h>

namespace geos {
namespace geom {
class Coordinate;
class CoordinateArraySequence;
}
namespace triangulate {
namespace quadedge {
}
}
}

using geos::geom::Coordinate;
using geos::geom::CoordinateArraySequence;

namespace geos {
namespace simplify { // geos::simplify



class LinkedRing
{
    private:

        static constexpr std::size_t
            NO_COORD_INDEX = std::numeric_limits<std::size_t>::max();

        const std::vector<Coordinate>& m_coord;
        std::size_t m_size;
        std::vector<std::size_t> m_next;
        std::vector<std::size_t> m_prev;

        static std::vector<std::size_t> createNextLinks(std::size_t size);
        static std::vector<std::size_t> createPrevLinks(std::size_t size);


    public:

        LinkedRing(const std::vector<Coordinate>& cs)
            : m_coord(cs)
            , m_size(cs.size()-1)
            , m_next(createNextLinks(m_size))
            , m_prev(createPrevLinks(m_size))
            {};

        std::size_t size() const;
        std::size_t next(std::size_t i) const;
        std::size_t prev(std::size_t i) const;
        const Coordinate& getCoordinate(std::size_t index) const;
        const Coordinate& prevCoordinate(std::size_t index) const;
        const Coordinate& nextCoordinate(std::size_t index) const;
        bool hasCoordinate(std::size_t index) const;
        void remove(std::size_t index);
        std::unique_ptr<CoordinateArraySequence> getCoordinates() const;


}; // LinkedRing


} // geos::simplify
} // geos

