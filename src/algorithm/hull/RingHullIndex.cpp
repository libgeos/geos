/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/hull/RingHullIndex.h>
#include <geos/algorithm/hull/RingHull.h>
#include <geos/geom/Envelope.h>


using geos::geom::Envelope;


namespace geos {
namespace algorithm { // geos.algorithm
namespace hull {      // geos.algorithm.hulll


/* public */
void
RingHullIndex::add(const RingHull* ringHull)
{
    hulls.push_back(ringHull);
}

std::size_t
RingHullIndex::size() const
{
    return hulls.size();
}

/* public */
std::vector<const RingHull*>
RingHullIndex::query(const Envelope& queryEnv) const
{
    std::vector<const RingHull*> result;
    for (auto hull : hulls) {
        const Envelope* envHull = hull->getEnvelope();
        if (queryEnv.intersects(*envHull)) {
            result.push_back(hull);
        }
    }
    return result;
}


} // namespace geos.algorithm.hull
} // namespace geos.algorithm
} // namespace geos

