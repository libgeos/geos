/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 ISciences LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/cluster/GeometryFlattener.h>

namespace geos  {
namespace operation {
namespace cluster {

std::unique_ptr<geom::Geometry>
GeometryFlattener::flatten(std::unique_ptr<geom::Geometry>&& g) {
    if (!g->isCollection()) {
        return std::move(g);
    }

    if (g->isEmpty()) {
        return std::move(g);
    }

    const geom::GeometryFactory* factory = g->getFactory();

    std::vector<std::unique_ptr<geom::Geometry>> components;
    flatten(std::move(g), components);

    return factory->buildGeometry(std::move(components));
}

void
GeometryFlattener::flatten(std::unique_ptr<geom::Geometry>&& g, std::vector<std::unique_ptr<geom::Geometry>>& components)
{
    if (g->isCollection()) {
        auto gc = static_cast<geom::GeometryCollection*>(g.get());

        for (auto& gi : gc->releaseGeometries()) {
            flatten(std::move(gi), components);
        }
    } else {
        components.push_back(std::move(g));
    }
}


}
}
}
