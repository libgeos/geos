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

#ifndef GEOS_OPERATION_CLUSTER_GEOMETRYINTERSECTSCLUSTERFINDER
#define GEOS_OPERATION_CLUSTER_GEOMETRYINTERSECTSCLUSTERFINDER

#include <geos/operation/cluster/AbstractClusterFinder.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>

namespace geos {
namespace operation {
namespace cluster {

/** GeometryIntersectsClusterFinder clusters geometries by intersection.
 * Any two geometries that intersect will be included in the same cluster.
 */
class GEOS_DLL GeometryIntersectsClusterFinder : public AbstractClusterFinder {
protected:
    const geom::Envelope& queryEnvelope(const geom::Geometry* a) override {
        return *(a->getEnvelopeInternal());
    }

    bool shouldJoin(const geom::Geometry* a, const geom::Geometry *b) override {
        if (m_prep == nullptr || &(m_prep->getGeometry()) != a) {
            m_prep = geom::prep::PreparedGeometryFactory::prepare(a);
        }

        return m_prep->intersects(b);
    }

private:
    std::unique_ptr<geom::prep::PreparedGeometry> m_prep;
};


}
}
}

#endif
