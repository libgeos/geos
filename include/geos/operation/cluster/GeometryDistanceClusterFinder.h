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

#ifndef GEOS_OPERATION_CLUSTER_GEOMETRYDISTANCECLUSTERFINDER
#define GEOS_OPERATION_CLUSTER_GEOMETRYDISTANCECLUSTERFINDER

#include <geos/operation/cluster/AbstractClusterFinder.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>

namespace geos {
namespace operation {
namespace cluster {

/** GeometryDistanceClusterFinder clusters geometries according to the distance between
 *  them. Any two geometries that are within the specified threshold distance will be
 *  included in the same cluster.
 */
class GEOS_DLL GeometryDistanceClusterFinder : public AbstractClusterFinder {
public:
    explicit GeometryDistanceClusterFinder(double distance) : m_distance(distance) {}

protected:
    bool shouldJoin(const geom::Geometry* a, const geom::Geometry *b) override {
        if (m_prep == nullptr || &(m_prep->getGeometry()) != a) {
            m_prep = geom::prep::PreparedGeometryFactory::prepare(a);
        }

        return m_prep->isWithinDistance(b, m_distance);
    }

    const geom::Envelope& queryEnvelope(const geom::Geometry* a) override {
        m_envelope = *a->getEnvelopeInternal();
        m_envelope.expandBy(m_distance);
        return m_envelope;
    }

private:
    std::unique_ptr<geom::prep::PreparedGeometry> m_prep;
    double m_distance;
    geom::Envelope m_envelope;
};

}
}
}

#endif
