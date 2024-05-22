/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 Martin Davis <mtnclimb@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CoveragePolygon.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Location.h>
#include <geos/geom/Polygon.h>

using geos::algorithm::locate::IndexedPointInAreaLocator;
using geos::geom::Coordinate;
using geos::geom::Envelope;
using geos::geom::Location;
using geos::geom::Polygon;

namespace geos {     // geos
namespace coverage { // geos.coverage

/* public */
CoveragePolygon::CoveragePolygon(const Polygon* poly)
    : polygon(poly)
{
    polyEnv = *(poly->getEnvelopeInternal());
}

/* public */
bool 
CoveragePolygon::intersectsEnv(const Envelope env)
{
    return polyEnv.intersects(env);
}

/* public */
bool 
CoveragePolygon::intersectsEnv(const Coordinate p)
{
    return polyEnv.intersects(p);
}

/* public */
bool 
CoveragePolygon::contains(const Coordinate p)
{
    if (! intersectsEnv(p))
        return false;
    IndexedPointInAreaLocator* pia = getLocator();
    return Location::INTERIOR == pia->locate(&p);
}

/* private */
IndexedPointInAreaLocator*
CoveragePolygon::getLocator()
{
    if (locator == nullptr) {
        //locator = std::make_unique<IndexedPointInAreaLocator>(new IndexedPointInAreaLocator(*polygon));
        locator = std::unique_ptr<IndexedPointInAreaLocator>(new IndexedPointInAreaLocator(*polygon));
    }
    return locator.get();
}

} // namespace geos.coverage
} // namespace geos


