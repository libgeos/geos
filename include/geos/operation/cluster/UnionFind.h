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

#ifndef GEOS_OPERATION_CLUSTER_UNIONFIND
#define GEOS_OPERATION_CLUSTER_UNIONFIND

#include <algorithm>
#include <numeric>
#include <vector>

#include <geos/export.h>
#include <geos/operation/cluster/Clusters.h>

namespace geos {
namespace operation {
namespace cluster {

/** UnionFind provides an implementation of a disjoint set data structure that
 * is useful in clustering. Elements to be clustered are referred to according
 * to a numeric index.
 */
class GEOS_DLL UnionFind {

public:
    /** Create a UnionFind object
     *
     * @param n the number of elements to be clustered (fixed size)
     */
    explicit UnionFind(size_t n) :
            clusters(n),
            sizes(n),
            num_clusters(n) {
        std::iota(clusters.begin(), clusters.end(), 0);
        std::fill(sizes.begin(), sizes.end(), 1);
    }

    // Are two elements in the same cluster?
    bool same(size_t i, size_t j) {
        return i == j || (find(i) == find(j));
    }

    // Are two elements in a different cluster?
    bool different(size_t i, size_t j) {
        return !same(i, j);
    }

    /**
     * Return the ID of the cluster associated with an item
     * @param i index of the item to lookup
     * @return a numeric cluster ID
     */
    size_t find(size_t i) {
        size_t root = i;

        while(clusters[root] != root) {
            root = clusters[root];
        }

        while (i != root) {
            size_t next = clusters[i];
            clusters[i] = root;
            i = next;
        }

        return i;
    }

    /**
     * Merge the clusters associated with two items
     * @param i ID of an item associated with the first cluster
     * @param j ID of an item associated with the second cluster
     */
    void join(size_t i, size_t j) {
        auto a = find(i);
        auto b = find(j);

        if (a == b) {
            return;
        }

        if ((sizes[b] > sizes[a]) || (sizes[a] == sizes[b] && b <= a)) {
            std::swap(a, b);
        }

        clusters[a] = clusters[b];
        sizes[b] += sizes[a];
        sizes[a] = 0;

        num_clusters--;
    }

    size_t getNumClusters() const {
        return num_clusters;
    }

    template<typename T>
    void sortByCluster(T begin, T end) {
        std::sort(begin, end, [this](size_t a, size_t b) {
            return find(a) < find(b);
        });
    }

    /**
     * Return the clusters associated with all elements
     * @return an object that allows iteration over the elements of each cluster
     */
    Clusters getClusters();

    /**
     * Return the clusters associated with the given elements
     * @param elems a vector of element ids
     * @return an object that allows iteration over the elements of each cluster
     */
    Clusters getClusters(std::vector<size_t> elems);

private:
    std::vector<size_t> clusters;
    std::vector<size_t> sizes;
    size_t num_clusters;
};

}
}
}

#endif
