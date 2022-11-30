/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: precision/SimpleGeometryPrecisionReducer.cpp rev. 1.10 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/precision/PointwisePrecisionReducerTransformer.h>
#include <geos/util.h>


using namespace geos::geom;
using namespace geos::geom::util;

namespace geos {
namespace precision { // geos.precision


/* public static */
std::unique_ptr<Geometry>
PointwisePrecisionReducerTransformer::reduce(const Geometry& geom, const PrecisionModel& targetPM)
{
    PointwisePrecisionReducerTransformer trans(targetPM);
    return trans.transform(&geom);
}

/* protected */
std::unique_ptr<CoordinateSequence>
PointwisePrecisionReducerTransformer::transformCoordinates(const CoordinateSequence* coords, const Geometry* parent)
{
    (void)(parent); // ignore unused variable

    if (coords->isEmpty()) {
        return detail::make_unique<CoordinateSequence>(0u, coords->getDimension());
    }

    return reducePointwise(coords);
}


/* private */
std::unique_ptr<CoordinateSequence>
PointwisePrecisionReducerTransformer::reducePointwise(const CoordinateSequence* coordinates)
{
    auto coordReduce = detail::make_unique<CoordinateSequence>();
    coordReduce->reserve(coordinates->size());

    // copy coordinates and reduce
    coordinates->forEach<Coordinate>([&coordReduce, this](Coordinate coord) {
        targetPM.makePrecise(coord);
        coordReduce->add(coord);
    });
    return coordReduce;
}



} // namespace geos.precision
} // namespace geos

