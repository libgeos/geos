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
 **********************************************************************
 *
 * Last port: geom/util/GeometryMapper.java
 *
 **********************************************************************/

#include <geos/geom/util/GeometryMapper.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util


/* public static */
std::unique_ptr<Geometry>
GeometryMapper::map(const Geometry& geom, mapOp op)
{
    std::vector<std::unique_ptr<Geometry>> mapped;
    for (std::size_t i = 0; i < geom.getNumGeometries(); i++) {
        const Geometry* subgeom = geom.getGeometryN(i);
        std::unique_ptr<Geometry> g = op(*subgeom);
        if (g != nullptr)
            mapped.push_back(std::move(g));
    }
    return geom.getFactory()->buildGeometry(std::move(mapped));
}



/* public static */
std::unique_ptr<Geometry>
GeometryMapper::flatMap(const Geometry& geom, int emptyDim, mapOp op)
{
    std::vector<std::unique_ptr<Geometry>> mapped;
    flatMap(geom, op, mapped);

    if (mapped.empty()) {
        return geom.getFactory()->createEmpty(emptyDim);
    }
    if (mapped.size() == 1) {
        std::unique_ptr<Geometry> rg(mapped.at(0).release());
        return rg;
    }
    return geom.getFactory()->buildGeometry(std::move(mapped));
}

/* private static */
void
GeometryMapper::flatMap(const Geometry& geom, mapOp op, std::vector<std::unique_ptr<Geometry>>& mapped)
{
    for (std::size_t i = 0; i < geom.getNumGeometries(); i++) {
        const Geometry* g = geom.getGeometryN(i);
        if (g->isCollection()) {
            flatMap(*g, op, mapped);
        }
        else {
            std::unique_ptr<Geometry> res = op(*g);
            if (res != nullptr && ! res->isEmpty()) {
                addFlat(res, mapped);
            }
        }
    }
}

/* private static */
void
GeometryMapper::addFlat(std::unique_ptr<Geometry>& geom, std::vector<std::unique_ptr<Geometry>>& geomList)
{
    if (geom->isEmpty()) return;
    if (geom->isCollection()) {
        auto col = static_cast<GeometryCollection*>(geom.get());
        std::vector<std::unique_ptr<Geometry>> geoms = col->releaseGeometries();
        for (auto& subgeom: geoms) {
            addFlat(subgeom, geomList);
        }
    }
    else {
        geomList.emplace_back(geom.release());
    }
}



} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

