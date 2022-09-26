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

#pragma once

#include <geos/export.h>
#include <geos/operation/cluster/AbstractClusterFinder.h>
#include <geos/operation/cluster/GeometryFlattener.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>

namespace geos {
namespace operation {
namespace cluster {

class GEOS_DLL DisjointOperation {
public:
    DisjointOperation(AbstractClusterFinder& finder) : m_finder(finder) {}

    /** Decompose a geometry into disjoint subsets using the provided `ClusterFinder`,
     *  process each subset using `f`, and combine the results. It is assumed that
     *  the processed results of each subset will also be disjoint; therefore, this
     *  algorithm may not be suitable for operations such as buffering.
     *
     * @brief process
     * @param g a geometry to be processed
     * @param f function taking a single argument of `const Geometry&`
     */
    template<typename Function>
    std::unique_ptr<geom::Geometry> processDisjointSubsets(const geom::Geometry& g, Function&& f) {
        if (g.getNumGeometries() == 1) {
            return f(g);
        }

        auto flattened = operation::cluster::GeometryFlattener::flatten(g.clone());
        auto clustered = m_finder.cluster(std::move(flattened));

        for (auto& subset : clustered) {
            subset = f(*subset);
        }

        auto coll = g.getFactory()->createGeometryCollection(std::move(clustered));

        return operation::cluster::GeometryFlattener::flatten(std::move(coll));
    }

private:
    AbstractClusterFinder& m_finder;
};


}
}
}
