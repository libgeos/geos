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

#ifndef GEOS_OPERATION_CLUSTER_ENVELOPEDISTANCECLUSTERFINDER
#define GEOS_OPERATION_CLUSTER_ENVELOPEDISTANCECLUSTERFINDER

#include <geos/operation/cluster/AbstractClusterFinder.h>
#include <geos/geom/Envelope.h>

namespace geos {
namespace operation {
namespace cluster {

/** EnvelopeDistanceClusterFinder clusters geometries by the distance between their envelopes.
 * Any two geometries whose envelopes are within the specified distance will be included in the same cluster.
 */
class GEOS_DLL EnvelopeDistanceClusterFinder : public AbstractClusterFinder {
public:
    explicit EnvelopeDistanceClusterFinder(double d) : m_distance(d), m_distance_squared(d*d) {}

protected:
    const geom::Envelope& queryEnvelope(const geom::Geometry* a) override {
        m_envelope = *a->getEnvelopeInternal();
        m_envelope.expandBy(m_distance);
        return m_envelope;
    }

    bool shouldJoin(const geom::Geometry* a, const geom::Geometry *b) override {
        return a->getEnvelopeInternal()->distanceSquared(*b->getEnvelopeInternal()) <= m_distance_squared;
    }

private:
    geom::Envelope m_envelope;
    double m_distance;
    double m_distance_squared;
};

}
}
}

#endif
