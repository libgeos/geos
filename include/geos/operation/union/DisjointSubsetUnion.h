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
#include <geos/geom/Geometry.h>
#include <geos/operation/cluster/DisjointOperation.h>
#include <geos/operation/cluster/GeometryIntersectsClusterFinder.h>
#include <geos/operation/overlayng/OverlayNGRobust.h>

namespace geos {
namespace operation {
namespace geounion {

class GEOS_DLL DisjointSubsetUnion {
public:
    /** Perform a unary union on a geometry by combining the results of
     *  unary unions performed on its disjoint subsets.
     *
     * @brief Union
     * @param g the geometry to union
     * @return result geometry
     */
    static std::unique_ptr<geom::Geometry> Union(const geom::Geometry* g) {
        operation::cluster::GeometryIntersectsClusterFinder f;
        operation::cluster::DisjointOperation op(f);
        op.setSplitInputs(true);

        return op.processDisjointSubsets(*g, [](const geom::Geometry& subset) {
            return subset.Union();
        });
    }
};

}
}
}
