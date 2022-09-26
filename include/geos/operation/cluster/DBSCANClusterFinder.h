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

#ifndef GEOS_OPERATION_CLUSTER_DBSCANCLUSTERFINDER
#define GEOS_OPERATION_CLUSTER_DBSCANCLUSTERFINDER

#include <geos/operation/cluster/AbstractClusterFinder.h>
#include <geos/geom/Geometry.h>
#include <stdexcept>

namespace geos {
namespace operation {
namespace cluster {

/** DBSCANClusterFinder clusters geometries according to the DBSCAN algorithm.
 *
 */
class GEOS_DLL DBSCANClusterFinder : public AbstractClusterFinder {
public:
    DBSCANClusterFinder(double eps, size_t minPoints) : m_eps(eps), m_minPoints(minPoints) {}

protected:

    const geom::Envelope& queryEnvelope(const geom::Geometry* a) override {
        m_envelope = *a->getEnvelopeInternal();
        m_envelope.expandBy(m_eps);
        return m_envelope;
    }

    Clusters process(const std::vector<const geom::Geometry*> & components,
             index::strtree::TemplateSTRtree<std::size_t> & index,
             UnionFind & uf) override;

    bool shouldJoin(const geom::Geometry*, const geom::Geometry*) override {
        throw std::runtime_error("Never get here.");
    }

private:
    double m_eps;
    size_t m_minPoints;
    geom::Envelope m_envelope;
};

}
}
}

#endif
