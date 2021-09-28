/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/triangulate/tri/Tri.h>

#include <geos/export.h>
#include <iostream>
#include <deque>
#include <array>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class Geometry;
}
}

using geos::geom::Coordinate;
using geos::geom::Geometry;

namespace geos {        // geos.
namespace triangulate { // geos.triangulate
namespace tri {         // geos.triangulate.tri


/**
 * A utility class that holds the ownership of
 * the Tris being passed between different processing
 * operations, and also provides access to those
 * Tris as a vector of pointers, to match up
 * to the JTS API.
 *
 */
class GEOS_DLL TriList {

private:

    // Members
    std::deque<Tri> triStore;
    std::vector<Tri*> tris;

    // Methods
    Tri* create(const Coordinate& c0, const Coordinate& c1, const Coordinate& c2);


public:

    TriList() {};

    void add(const Coordinate& c0, const Coordinate& c1, const Coordinate& c2);

    void add(std::array<Coordinate, 3>& corner)
    {
        add(corner[0], corner[1], corner[2]);
    }

    std::unique_ptr<Geometry> toGeometry(
        const GeometryFactory* geomFact) const;

    static std::unique_ptr<Geometry> toGeometry(
        const geom::GeometryFactory* geomFact,
        const std::vector<std::unique_ptr<TriList>>& allTriLists);

    friend std::ostream& operator << (std::ostream& os, TriList& te);

    // Support for iterating on TriList
    typedef std::vector<Tri*>::iterator iterator;
    typedef std::vector<Tri*>::const_iterator const_iterator;
    size_t size() const { return tris.size(); }
    bool empty() const { return tris.empty(); }
    iterator begin() { return tris.begin(); }
    iterator end() { return tris.end(); }
    const_iterator begin() const { return tris.begin(); }
    const_iterator end() const { return tris.end(); }
    Tri* operator [] (std::size_t index) { return tris[index]; }

};


} // namespace geos.triangulate.tri
} // namespace geos.triangulate
} // namespace geos

