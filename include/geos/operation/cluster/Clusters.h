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

#ifndef GEOS_OPERATION_CLUSTER_CLUSTERS
#define GEOS_OPERATION_CLUSTER_CLUSTERS

#include <geos/export.h>
#include <vector>

namespace geos {
namespace operation {
namespace cluster {

class UnionFind;

class GEOS_DLL Clusters {
private:
    std::vector<std::size_t> starts;
    std::vector<std::size_t> components;

public:
    using const_iterator = decltype(components)::const_iterator;

    explicit Clusters(UnionFind & uf, std::vector<std::size_t> elems);

    // Get the number of clusters available
    std::size_t getNumClusters() const {
        return starts.size();
    }

    // Get the size of a given cluster
    std::size_t getSize(std::size_t cluster) const {
        return static_cast<std::size_t>(std::distance(begin(cluster), end(cluster)));
    }

    // Get an iterator to the first element in a given cluster
    const_iterator begin(std::size_t cluster) const {
        return std::next(components.begin(), static_cast<std::ptrdiff_t>(starts[cluster]));
    }

    // Get an iterator beyond the last element in a given cluster
    const_iterator end(std::size_t cluster) const {
        if (cluster == static_cast<std::size_t>(starts.size() - 1)) {
            return components.end();
        }

        return begin(cluster + 1);
    }


};

}
}
}

#endif
