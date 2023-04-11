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

// #include <geos/geom/Coordinate.h>

#include <geos/export.h>

#include <vector>
#include <memory>


namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
}
}

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;


namespace geos {
namespace simplify { // geos::simplify

class LinkedLine
{

public:

    LinkedLine(const CoordinateSequence& pts);

    bool isRing() const;
    bool isCorner(std::size_t i) const;

    std::size_t size() const;
    std::size_t next(std::size_t i) const;
    std::size_t prev(std::size_t i) const;

    const Coordinate& getCoordinate(std::size_t index) const;
    const Coordinate& prevCoordinate(std::size_t index) const;
    const Coordinate& nextCoordinate(std::size_t index) const;

    bool hasCoordinate(std::size_t index) const;

    void remove(std::size_t index);

    std::unique_ptr<CoordinateSequence> getCoordinates() const;


private:

    // Members
    const CoordinateSequence&  m_coord;
    bool m_isRing;
    std::size_t m_size;
    std::vector<std::size_t> m_next;
    std::vector<std::size_t> m_prev;

    void createNextLinks(std::size_t size);

    void createPrevLinks(std::size_t size);


}; // LinkedLine

GEOS_DLL std::ostream& operator<< (std::ostream& os, const LinkedLine& ll);


} // geos::simplify
} // geos
