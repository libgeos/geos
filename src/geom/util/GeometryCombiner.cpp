/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006-2011 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/util/GeometryCombiner.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/geom/util/GeometryCombiner.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

std::unique_ptr<Geometry>
GeometryCombiner::combine(std::vector<std::unique_ptr<Geometry>>&& geoms)
{
    GeometryCombiner combiner(std::move(geoms));
    return combiner.combine();
}

std::unique_ptr<Geometry>
GeometryCombiner::combine(std::vector<const Geometry*> const& geoms)
{
    GeometryCombiner combiner(geoms);
    return combiner.combine();
}

std::unique_ptr<Geometry>
GeometryCombiner::combine(const Geometry* g0, const Geometry* g1)
{
    std::vector<const Geometry*> geoms;
    geoms.push_back(g0);
    geoms.push_back(g1);

    GeometryCombiner combiner(geoms);
    return combiner.combine();
}

std::unique_ptr<Geometry>
GeometryCombiner::combine(std::unique_ptr<Geometry> && g0,
                          std::unique_ptr<Geometry> && g1)
{
    std::vector<std::unique_ptr<Geometry>> geoms(2);
    geoms[0] = std::move(g0);
    geoms[1] = std::move(g1);
    GeometryCombiner combiner(std::move(geoms));
    return combiner.combine();
}

std::unique_ptr<Geometry>
GeometryCombiner::combine(std::unique_ptr<Geometry> && g0,
                          std::unique_ptr<Geometry> && g1,
                          std::unique_ptr<Geometry> && g2)
{
    std::vector<std::unique_ptr<Geometry>> geoms(3);
    geoms[0] = std::move(g0);
    geoms[1] = std::move(g1);
    geoms[2] = std::move(g2);
    GeometryCombiner combiner(std::move(geoms));
    return combiner.combine();
}

std::unique_ptr<Geometry>
GeometryCombiner::combine(const Geometry* g0, const Geometry* g1,
                          const Geometry* g2)
{
    std::vector<const Geometry*> geoms;
    geoms.push_back(g0);
    geoms.push_back(g1);
    geoms.push_back(g2);

    GeometryCombiner combiner(geoms);
    return combiner.combine();
}

GeometryCombiner::GeometryCombiner(std::vector<const Geometry*> const& geoms) : skipEmpty(false)
{
    for(const auto& geom : geoms) {
        for (std::size_t i = 0; i < geom->getNumGeometries(); i++) {
            auto part = geom->getGeometryN(i);
            inputGeoms.push_back(part->clone());
        }
    }
}

GeometryCombiner::GeometryCombiner(std::vector<std::unique_ptr<Geometry>> && geoms) : skipEmpty(false)
{
    for(auto& geom : geoms) {
        auto coll = dynamic_cast<GeometryCollection *>(geom.get());
        if (coll) {
            for (auto &part : coll->releaseGeometries()) {
                inputGeoms.push_back(std::move(part));
            }
        } else {
            inputGeoms.push_back(std::move(geom));
        }
    }
}

std::unique_ptr<Geometry>
GeometryCombiner::combine()
{
    auto geomFactory = inputGeoms.empty() ? GeometryFactory::getDefaultInstance() : inputGeoms.front()->getFactory();

    if (skipEmpty) {
        inputGeoms.erase(std::remove_if(inputGeoms.begin(), inputGeoms.end(), [](std::unique_ptr<Geometry> & g) {
            return g->isEmpty();
        }), inputGeoms.end());
    }

    // return the "simplest possible" geometry
    return geomFactory->buildGeometry(std::move(inputGeoms));
}

void GeometryCombiner::setSkipEmpty(bool b) {
    skipEmpty = b;
}

} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

