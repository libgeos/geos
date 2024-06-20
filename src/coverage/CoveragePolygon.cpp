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
using geos::geom::CoordinateXY;
using geos::geom::Envelope;
using geos::geom::Location;
using geos::geom::Polygon;

namespace geos {     // geos
namespace coverage { // geos.coverage

/* public */
CoveragePolygon::CoveragePolygon(const Polygon* poly)
    : m_polygon(poly)
{
    //-- cache polygon envelope for maximum performance
    polyEnv = *(poly->getEnvelopeInternal());
}

/* public */
bool 
CoveragePolygon::intersectsEnv(const Envelope& env) const
{
    return polyEnv.intersects(env);
}

/* public */
bool 
CoveragePolygon::intersectsEnv(const CoordinateXY& p) const
{
    return polyEnv.intersects(p);
}

/* public */
bool 
CoveragePolygon::contains(const CoordinateXY& p) const
{
    if (! intersectsEnv(p))
        return false;
    auto& pia = getLocator();
    return Location::INTERIOR == pia.locate(&p);
}

/* private */
IndexedPointInAreaLocator&
CoveragePolygon::getLocator() const
{
    if (m_locator == nullptr) {
        m_locator = std::make_unique<IndexedPointInAreaLocator>(*m_polygon);
    }
    return *m_locator;
}

} // namespace geos.coverage
} // namespace geos


