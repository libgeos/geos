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
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/precision/PointwisePrecisionReducerTransformer.h>


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
        CoordinateArraySequence* cas = new CoordinateArraySequence(std::size_t(0u), coords->getDimension());
        return std::unique_ptr<CoordinateSequence>(static_cast<CoordinateSequence*>(cas));
    }

    std::vector<Coordinate> coordsReduce = reducePointwise(coords);
    CoordinateArraySequence* cas = new CoordinateArraySequence(std::move(coordsReduce));
    return std::unique_ptr<CoordinateSequence>(static_cast<CoordinateSequence*>(cas));
}


/* private */
std::vector<Coordinate>
PointwisePrecisionReducerTransformer::reducePointwise(const CoordinateSequence* coordinates)
{
    std::vector<Coordinate> coordReduce;
    coordReduce.reserve(coordinates->size());

    // copy coordinates and reduce
    for (std::size_t i = 0; i < coordinates->size(); i++) {
        Coordinate coord = coordinates->getAt(i);
        targetPM.makePrecise(coord);
        coordReduce.emplace_back(coord);
    }
    return coordReduce;
}



} // namespace geos.precision
} // namespace geos

