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

#ifndef GEOS_OPERATION_CLUSTER_ENVELOPEINTERSECTSCLUSTERFINDER
#define GEOS_OPERATION_CLUSTER_ENVELOPEINTERSECTSCLUSTERFINDER

#include <geos/operation/cluster/AbstractClusterFinder.h>

namespace geos {
namespace operation {
namespace cluster {

/** EnvelopeIntersectsClusterFinder clusters geometries by envelope intersection.
 * Any two geometries whose envelopes intersect will be included in the same cluster.
 */
class GEOS_DLL EnvelopeIntersectsClusterFinder : public AbstractClusterFinder {
protected:

    const geom::Envelope& queryEnvelope(const geom::Geometry* a) override {
        return *(a->getEnvelopeInternal());
    }

    bool shouldJoin(const geom::Geometry* a, const geom::Geometry *b) override {
        return a->getEnvelopeInternal()->intersects(b->getEnvelopeInternal());
    }

};

}
}
}

#endif