/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021-2022 Daniel Baston
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

Clusters::Clusters(UnionFind & uf, std::vector<std::size_t> elemsInCluster, size_t numElems) {
    m_elemsInCluster = std::move(elemsInCluster);
    m_numElems = numElems;

    if (!m_elemsInCluster.empty()) {
        uf.sortByCluster(m_elemsInCluster.begin(), m_elemsInCluster.end());

        m_starts.reserve(uf.getNumClusters());
        m_starts.push_back(0);

        for (std::size_t i = 1; i < m_elemsInCluster.size(); i++) {
            if (uf.find(m_elemsInCluster[i]) != uf.find(m_elemsInCluster[i - 1])) {
                m_starts.push_back(i);
            }
        }
    }
}

std::vector<std::size_t>
Clusters::getClusterIds(std::size_t noClusterValue) const {
    std::vector<std::size_t> cluster_ids(m_numElems, noClusterValue);

    for (std::size_t i = 0; i < getNumClusters(); i++) {
        for (auto it = begin(i); it != end(i); ++it) {
            cluster_ids[*it] = i;
        }
    }

    return cluster_ids;
}

}
}
}

