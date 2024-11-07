/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Martin Davis <mtnclimb@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/construct/IndexedDistanceToPoint.java
 * https://github.com/locationtech/jts/commit/d92f783163d9440fcc10c729143787bf7b9fe8f9
 *
 **********************************************************************/

#include <geos/algorithm/construct/IndexedPointInPolygonsLocator.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/util/PolygonalExtracter.h>
#include <vector>

using geos::algorithm::locate::IndexedPointInAreaLocator;
using geos::geom::Envelope;
using geos::geom::Location;
using geos::geom::util::PolygonalExtracter;

namespace geos {
namespace algorithm { // geos.algorithm
namespace construct { // geos.algorithm.construct

/* public */
IndexedPointInPolygonsLocator::IndexedPointInPolygonsLocator(const Geometry& g)
    : geom(g), isInitialized(false)
{   
}

/* private */
void IndexedPointInPolygonsLocator::init()
{
    if (isInitialized) {
        return;
    }
    isInitialized = true;
    std::vector<const Geometry*> polys;
    PolygonalExtracter::getPolygonals(geom, polys);
    for (const Geometry* poly : polys) {
        IndexedPointInAreaLocator* ptLocator = new IndexedPointInAreaLocator(*poly);
        locators.emplace_back(ptLocator);
        index.insert(poly->getEnvelopeInternal(), ptLocator);
    }
}

/* public */
Location IndexedPointInPolygonsLocator::locate(const CoordinateXY* /*const*/ pt)
{
    init();
    Envelope queryEnv(*pt);
    std::vector<IndexedPointInAreaLocator*> result;
    index.query(queryEnv, result);
    for (IndexedPointInAreaLocator* ptLocater : result) {
      Location loc = ptLocater->locate(pt);
      if (loc != Location::EXTERIOR)
        return loc;
    }
    return Location::EXTERIOR;
}

}}}