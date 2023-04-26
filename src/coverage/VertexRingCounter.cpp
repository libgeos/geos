/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (c) 2022 Martin Davis.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/VertexRingCounter.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/Geometry.h>


using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateSequenceFilter;
using geos::geom::Geometry;


namespace geos {
namespace coverage { // geos.coverage

/* public static  */
void
VertexRingCounter::count(
    std::vector<const Geometry*>& geoms,
    std::map<Coordinate, std::size_t>& counts)
{
    VertexRingCounter vertextCounter(counts);
    for (const Geometry* geom : geoms) {
        geom->apply_ro(vertextCounter);
    }
}


/* public */
void
VertexRingCounter::filter_ro(const CoordinateSequence& seq, std::size_t i)
{
    //-- for rings don't double-count duplicate endpoint
    if (seq.isRing() && i == 0)
        return;

    const Coordinate& v = seq.getAt(i);
    auto search = vertexCounts.find(v);
    std::size_t count = 0;
    if (search != vertexCounts.end()) {
        count = search->second;
    }
    count++;
    vertexCounts[v] = count;
}


} // geos.coverage
} // geos
