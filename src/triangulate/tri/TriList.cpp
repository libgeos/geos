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

#include <geos/geom/Coordinate.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/triangulate/tri/TriList.h>
#include <geos/export.h>


namespace geos {        // geos
namespace triangulate { // geos.triangulate
namespace tri {         // geos.triangulate.tri


/* private */
Tri*
TriList::create(const Coordinate& c0, const Coordinate& c1, const Coordinate& c2)
{
    triStore.emplace_back(c0, c1, c2);
    Tri* newTri = &triStore.back();
    return newTri;
}


/* public */
void
TriList::add(const Coordinate& c0, const Coordinate& c1, const Coordinate& c2)
{
    Tri* newTri = create(c0, c1, c2);
    tris.push_back(newTri);
}

/* public */
std::unique_ptr<Geometry>
TriList::toGeometry(const geom::GeometryFactory* geomFact) const
{
    std::vector<std::unique_ptr<Geometry>> geoms;
    for (auto* tri: tris) {
        std::unique_ptr<Geometry> geom = tri->toPolygon(geomFact);
        geoms.emplace_back(geom.release());
    }
    return geomFact->createGeometryCollection(std::move(geoms));
}

/* public static */
std::unique_ptr<Geometry>
TriList::toGeometry(
    const geom::GeometryFactory* geomFact,
    const std::vector<std::unique_ptr<TriList>>& allTriLists)
{
    std::vector<std::unique_ptr<Geometry>> geoms;
    for (const std::unique_ptr<TriList>& triList: allTriLists) {
        for (const Tri* tri: *triList) {
            std::unique_ptr<Geometry> geom = tri->toPolygon(geomFact);
            geoms.emplace_back(geom.release());
        }
    }
    return geomFact->createGeometryCollection(std::move(geoms));
}

std::ostream&
operator<<(std::ostream& os, TriList& triList)
{
    os << "TRILIST ";
    os << "[" << triList.size() << "] (";
    for (auto* tri: triList) {
        os << "  " << *tri << "," << std::endl;
    }
    os << ")";
    return os;
}


} // namespace geos.triangulate.tri
} // namespace geos.triangulate
} // namespace geos

