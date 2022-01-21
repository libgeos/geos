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

#include <geos/geom/GeometryFactory.h>

#include <geos/export.h>
#include <iostream>
#include <iterator>
#include <deque>
#include <array>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class Geometry;
}
}

using geos::geom::Coordinate;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;

typedef int TriIndex;

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
template<typename TriType>
class TriList {

private:

    // Members
    std::deque<TriType> triStore;
    std::vector<TriType*> tris;

    // Methods
    TriType* create(const Coordinate& c0, const Coordinate& c1, const Coordinate& c2)
    {
        triStore.emplace_back(c0, c1, c2);
        TriType* newTri = &triStore.back();
        return newTri;
    }


public:

    TriList() {};

    std::vector<TriType*>& getTris()
    {
        return tris;
    }

    void remove(TriType* tri)
    {
        // We can leave triStore untouched, just remove
        // the pointer from tris.
        for (auto it = tris.begin(); it != tris.end(); ++it) {
            if (*it == tri) {
                tris.erase(it);
                return;
            }
        }
    }

    void add(const Coordinate& c0, const Coordinate& c1, const Coordinate& c2)
    {
        auto* newTri = create(c0, c1, c2);
        tris.push_back(newTri);
    };

    void add(std::array<Coordinate, 3>& corner)
    {
        add(corner[0], corner[1], corner[2]);
    };

    double area()
    {
        double dArea = 0.0;
        for (const auto* tri : tris) {
          dArea += tri->getArea();
        }
        return dArea;
    };

    double length()
    {
        double dLength = 0.0;
        for (const auto* tri : tris) {
          dLength += tri->getLength();
        }
        return dLength;
    };

    /* public */
    std::size_t
    degree(const TriType* tri, TriIndex index)
    {
        const Coordinate& v = tri->getCoordinate(index);
        std::size_t szDegree = 0;
        for (auto* t : *this) {
            for (TriIndex i = 0; i < 3; i++) {
                if (v.equals2D(t->getCoordinate(i)))
                    szDegree++;
            }
        }
        return szDegree;
    }

    void validate()
    {
        for (auto* tri : *this) {
            tri->validate();
        }
    }

    std::unique_ptr<Geometry> toGeometry(
        const GeometryFactory* geomFact) const
    {
        std::vector<std::unique_ptr<Geometry>> geoms;
        for (auto* tri: tris) {
            std::unique_ptr<Geometry> geom = tri->toPolygon(geomFact);
            geoms.emplace_back(geom.release());
        }
        return geomFact->createGeometryCollection(std::move(geoms));
    }

    friend std::ostream& operator << (std::ostream& os, TriList& triList)
    {
        os << "TRILIST ";
        os << "[" << triList.size() << "] (";
        for (auto* tri: triList) {
            os << "  " << *tri << "," << std::endl;
        }
        os << ")";
        return os;
    }

    // Support for iterating on TriList
    typedef typename std::vector<TriType*>::iterator iterator;
    typedef typename std::vector<TriType*>::const_iterator const_iterator;
    size_t size() const { return tris.size(); }
    bool empty() const { return tris.empty(); }
    iterator begin() { return tris.begin(); }
    iterator end() { return tris.end(); }
    const_iterator begin() const { return tris.begin(); }
    const_iterator end() const { return tris.end(); }
    TriType* operator [] (std::size_t index) { return tris[index]; }

};


} // namespace geos.triangulate.tri
} // namespace geos.triangulate
} // namespace geos

