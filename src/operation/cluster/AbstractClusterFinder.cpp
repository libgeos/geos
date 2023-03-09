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

#include <geos/operation/cluster/AbstractClusterFinder.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/prep/PreparedGeometry.h>

#include <geos/util.h>
#include <geos/index/strtree/TemplateSTRtree.h>
#include <geos/operation/cluster/UnionFind.h>

namespace geos {
namespace operation {
namespace cluster {

using geom::Geometry;

std::vector<std::unique_ptr<geom::Geometry>>
AbstractClusterFinder::clusterToVector(const geom::Geometry& g) {
    return clusterToVector(g.clone());
}

std::unique_ptr<geom::Geometry>
AbstractClusterFinder::clusterToCollection(std::unique_ptr<geom::Geometry> && g) {
    auto gfact = g->getFactory();

    return gfact->createGeometryCollection(clusterToVector(std::move(g)));
}

std::unique_ptr<geom::Geometry>
AbstractClusterFinder::clusterToCollection(const geom::Geometry & g) {
    return clusterToCollection(g.clone());
}


Clusters
AbstractClusterFinder::cluster(const std::vector<const geom::Geometry*> & components) {
    index::strtree::TemplateSTRtree<std::size_t> tree;

    for (std::size_t i = 0; i < components.size(); i++) {
        tree.insert(*components[i]->getEnvelopeInternal(), i);
    }

    UnionFind uf(components.size());
    return process(components, tree, uf);
}

std::vector<std::unique_ptr<geom::Geometry>>
AbstractClusterFinder::clusterToVector(std::unique_ptr<geom::Geometry> && g) {
    const geom::GeometryFactory& gfact = *g->getFactory();

    std::vector<const Geometry*> components(g->getNumGeometries());
    for (size_t i = 0; i < g->getNumGeometries(); i++) {
        components[i]= g->getGeometryN(i);
    }

    const auto& clusters = cluster(components);

    std::vector<std::unique_ptr<Geometry>> component_geoms = getComponents(std::move(g));

    std::vector<std::unique_ptr<Geometry>> cluster_geoms;

    for (size_t i = 0; i < clusters.getNumClusters(); i++) {
        std::vector<std::unique_ptr<Geometry>> cluster_component_geoms;
        cluster_component_geoms.reserve(clusters.getSize(i));
        for (auto it = clusters.begin(i); it != clusters.end(i); ++it) {
            cluster_component_geoms.push_back(std::move(component_geoms[*it]));
        }
        cluster_geoms.push_back(gfact.buildGeometry(std::move(cluster_component_geoms)));
    }

    return cluster_geoms;
}

Clusters
AbstractClusterFinder::process(const std::vector<const Geometry*> & components,
                   index::strtree::TemplateSTRtree<std::size_t> & tree,
                   UnionFind & uf) {

    std::vector<size_t> hits;

    for (size_t i = 0; i < components.size(); i++) {
        const geom::Geometry* gi = components[i];

        hits.clear();

        // Sort candidates based on envelope area to try and perform simpler intersection tests instead of
        // complex ones. This seems to perform better than sorting on the number of points.
        tree.query(queryEnvelope(gi), hits);
        std::sort(hits.begin(), hits.end(), [&components](std::size_t a, std::size_t b) {
            return components[a]->getEnvelopeInternal()->getArea() < components[b]->getEnvelopeInternal()->getArea();
        });

        for (std::size_t j : hits) {
            if (uf.different(i, j)) {
                const geom::Geometry* gj = components[j];

                // Only call shouldJoin with the more complex geometry in the LHS, where it will become
                // the prepared geometry.
                // TODO move point check to subclasses that benefit to avoid for those that don't?
                if (gi->getNumPoints() >= gj->getNumPoints() && shouldJoin(gi, gj)) {
                    uf.join(i, j);
                }
            }
        }

    }

    return uf.getClusters();
}

std::vector<std::unique_ptr<Geometry>>
AbstractClusterFinder::getComponents(std::unique_ptr<Geometry>&& g)
{
    if (g->isCollection()) {
        return detail::down_cast<geom::GeometryCollection*>(g.get())->releaseGeometries();
    } else {
        std::vector<std::unique_ptr<Geometry>> ret(1);
        ret[0] = std::move(g);
        return ret;
    }
}



}
}
}
