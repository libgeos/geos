/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/cluster/Clusters.h>
#include <geos/operation/cluster/UnionFind.h>

namespace geos {
namespace operation {
namespace cluster {

Clusters::Clusters(UnionFind & uf, std::vector<std::size_t> elems) {
    components = std::move(elems);
    if (!components.empty()) {
        uf.sortByCluster(components.begin(), components.end());

        starts.reserve(uf.getNumClusters());
        starts.push_back(0);

        for (std::size_t i = 1; i < components.size(); i++) {
            if (uf.find(components[i]) != uf.find(components[i - 1])) {
                starts.push_back(i);
            }
        }
    }
}

}
}
}

