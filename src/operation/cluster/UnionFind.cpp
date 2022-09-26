/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020-2021 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/cluster/UnionFind.h>

namespace geos {
namespace operation {
namespace cluster {

Clusters UnionFind::getClusters() {
    std::vector<size_t> elems(clusters.size());
    std::iota(elems.begin(), elems.end(), 0);

    return Clusters(*this, std::move(elems), clusters.size());
}

Clusters UnionFind::getClusters(std::vector<size_t> elems) {
    return Clusters(*this, std::move(elems), clusters.size());
}

}
}
}
