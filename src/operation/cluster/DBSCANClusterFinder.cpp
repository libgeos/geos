/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2015-2021 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/cluster/DBSCANClusterFinder.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/operation/cluster/UnionFind.h>

namespace geos {
namespace operation {
namespace cluster {

static inline void unionIfAvailable(UnionFind & uf,
                                    size_t p,
                                    size_t q,
                                    std::vector<bool> & is_in_core,
                                    std::vector<bool> & in_a_cluster) {
    if (in_a_cluster[q]) {
        // Can we merge p's cluster with q's cluster? We can do this only
        // if both p and q are considered _core_ points of their respective
        // clusters.
        if (is_in_core[q]) {
            uf.join(p, q);
        }
    } else {
        uf.join(p, q);
        in_a_cluster[q] = true;
    }
}

Clusters DBSCANClusterFinder::process(const std::vector<const geom::Geometry*> & components,
                      index::strtree::TemplateSTRtree<std::size_t> & tree,
                      UnionFind & uf) {

    std::vector<bool> in_a_cluster(components.size());
    std::vector<bool> is_in_core(components.size());

    std::vector<size_t> hits;
    std::vector<size_t> neighbors;
    for (size_t p = 0; p < components.size(); p++) {
        hits.clear();
        neighbors.clear();
        const geom::Geometry *gp = components[p];

        tree.query(queryEnvelope(gp), [&hits](std::size_t hit) {
            hits.push_back(hit);
        });

        if (hits.size() < m_minPoints) {
            // We didn't find enough points do do anything even if they're all within eps.
            continue;
        }

        std::unique_ptr<geom::prep::PreparedGeometry> prep;

        for (size_t q : hits) {
            if (neighbors.size() >= m_minPoints) {
                // If we've already identified p as a core point, and it's already in the same cluster
                // as q, then there's nothing to learn by computing the distance.
                if (uf.same(p, q)) {
                    continue;
                }

                // Similarly, if q is already identified as a border point of another cluster, there's
                // no point figuring out what the distance is.
                if (in_a_cluster[q] && !is_in_core[q]) {
                    continue;
                }
            }

            double dist;
            if (p == q) {
                dist = 0;
            } else {
                if (!prep) {
                    prep = geom::prep::PreparedGeometryFactory::prepare(components[p]);
                }

                dist = prep->distance(components[q]);
            }

            if (dist <= m_eps) {
                // If we haven't hit minPoints yet, we don't know if we can union p and q.
                // Just set q aside for now.
                if (neighbors.size() < m_minPoints) {
                    neighbors.push_back(q);

                    // If we just hit minPoints, we can now union all of the neighbor geometries
                    // we've been saving.
                    if (neighbors.size() == m_minPoints) {
                        is_in_core[p] = true;
                        in_a_cluster[p] = true;
                        for (auto& n : neighbors) {
                            unionIfAvailable(uf, p, n, is_in_core, in_a_cluster);
                        }
                    }
                } else {
                    // If we're above minPoints, no need to store our neighbors, just go ahead
                    // and union them now.
                    unionIfAvailable(uf, p, q, is_in_core, in_a_cluster);
                }
            }
        }
    }


    std::vector<size_t> includedInCluster;
    includedInCluster.reserve(components.size());
    for (size_t p = 0; p < components.size(); p++) {
        if (in_a_cluster[p]) {
            includedInCluster.push_back(p);
        }
    }

    return uf.getClusters(includedInCluster);
}


}
}
}
