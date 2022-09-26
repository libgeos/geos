/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020-2022 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_OPERATION_CLUSTER_ABSTRACTCLUSTERFINDER
#define GEOS_OPERATION_CLUSTER_ABSTRACTCLUSTERFINDER

#include <functional>
#include <memory>
#include <vector>

#include <geos/export.h>
#include <geos/index/strtree/TemplateSTRtree.h>
#include <geos/operation/cluster/Clusters.h>

// Forward declarations
namespace geos {
namespace geom {
    class Envelope;
    class Geometry;
}
namespace operation {
namespace cluster {
    class UnionFind;
}
}
}

namespace geos {
namespace operation {
namespace cluster {

/** AbstractClusterFinder defines an interface for bottom-up clustering algorithms,
 * where spatial index queries can be used to identify geometries that should be
 * clustered together.
 */
class GEOS_DLL AbstractClusterFinder {

public:
    /**
     * Cluster the provided geometries, returning an object that provides access
     * to the components of each cluster.
     *
     * @param g A vector of geometries to cluster
     */
    Clusters cluster(const std::vector<const geom::Geometry*>& g);

    /**
     * Cluster the components of the provided geometry, returning a vector of clusters.
     * This function will take ownership of the provided geometry. Any components that
     * are included in a cluster will be returned. Components that are not included in
     * any cluster will be destroyed.
     *
     * @param g A geometry whose components should be clustered.
     * @return a Geometry vector, with each entry representing a single cluster.
     */
    std::vector<std::unique_ptr<geom::Geometry>> clusterToVector(std::unique_ptr<geom::Geometry> && g);

    /**
     * Cluster the components of the provided geometry, returning a vector of clusters.
     * The input geometry will not be modified.
     *
     * @param g A geometry whose components should be clustered.
     * @return a Geometry vector, with each entry representing a single cluster.
     */
    std::vector<std::unique_ptr<geom::Geometry>> clusterToVector(const geom::Geometry& g);

    /**
     * Cluster the components of the provided geometry, returning a GeometryCollection.
     * This function will take ownership of the provided geometry. Any components that
     * are included in a cluster will be returned. Components that are not included in
     * any cluster will be destroyed.
     *
     * @param g A geometry whose components should be clustered.
     * @return a GeometryCollection, with each sub-geometry representing a single cluster.
     */
    std::unique_ptr<geom::Geometry> clusterToCollection(std::unique_ptr<geom::Geometry> && g);

    /**
     * Cluster the components of the provided geometry, returning a GeometryCollection.
     * The input geometry will not be modified.
     *
     * @param g A geometry whose components should be clustered.
     * @return a GeometryCollection, with each sub-geometry representing a single cluster.
     */
    std::unique_ptr<geom::Geometry> clusterToCollection(const geom::Geometry & g);

protected:
    /**
     * Determine whether two geometries should be considered in the same cluster.
     * @param a Geometry
     * @param b Geometry
     * @return `true` if the clusters associated with `a` and `b` should be merged.
     */
    virtual bool shouldJoin(const geom::Geometry* a, const geom::Geometry* b) = 0;

    /**
     * Provide an query Envelope that can be used to find all geometries possibly
     * in the same cluster as the input.
     * @param a Geometry
     * @return an Envelope suitable for querying
     */
    virtual const geom::Envelope& queryEnvelope(const geom::Geometry* a) = 0;

    /**
     * Given a vector and index of components,
     * @param components a vector of Geometry components
     * @param index a spatial index storing pointers to those components
     * @param uf a UnionFind
     * @return a vector of with the indices of all components that should be included in a cluster
     */
    virtual Clusters process(const std::vector<const geom::Geometry*> & components,
                 index::strtree::TemplateSTRtree<std::size_t> & index,
                 UnionFind & uf);

private:
    static std::vector<std::unique_ptr<geom::Geometry>> getComponents(std::unique_ptr<geom::Geometry>&& g);
};



}
}
}

#endif
