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

#ifndef GEOS_OPERATION_CLUSTER_CLUSTERS
#define GEOS_OPERATION_CLUSTER_CLUSTERS

#include <geos/export.h>
#include <limits>
#include <vector>

namespace geos {
namespace operation {
namespace cluster {

class UnionFind;

class GEOS_DLL Clusters {
private:
    std::vector<std::size_t> m_elemsInCluster; // The IDs of elements that are included in a cluster
    std::vector<std::size_t> m_starts; // Start position of each cluster in m_elemsInCluster
    std::size_t m_numElems; // The number of elements from which clusters were generated

public:
    using const_iterator = decltype(m_elemsInCluster)::const_iterator;

    explicit Clusters(UnionFind & uf, std::vector<std::size_t> elemsInCluster, std::size_t numElems);

    // Get the number of clusters available
    std::size_t getNumClusters() const {
        return m_starts.size();
    }

    // Get the size of a given cluster
    std::size_t getSize(std::size_t cluster) const {
        return static_cast<std::size_t>(std::distance(begin(cluster), end(cluster)));
    }

    // Get a vector containing the cluster ID for each item in `elems`
    std::vector<std::size_t> getClusterIds(std::size_t noClusterValue = std::numeric_limits<std::size_t>::max()) const;

    // Get an iterator to the first element in a given cluster
    const_iterator begin(std::size_t cluster) const {
        return std::next(m_elemsInCluster.begin(), static_cast<std::ptrdiff_t>(m_starts[cluster]));
    }

    // Get an iterator beyond the last element in a given cluster
    const_iterator end(std::size_t cluster) const {
        if (cluster == static_cast<std::size_t>(m_starts.size() - 1)) {
            return m_elemsInCluster.end();
        }

        return begin(cluster + 1);
    }

};

}
}
}

#endif
