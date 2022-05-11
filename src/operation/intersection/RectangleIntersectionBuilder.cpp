/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2014 Mika Heiskanen <mika.heiskanen@fmi.fi>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/intersection/Rectangle.h>
#include <geos/operation/intersection/RectangleIntersectionBuilder.h>
#include <geos/operation/valid/RepeatedPointRemover.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/algorithm/PointLocation.h>
#include <geos/util/IllegalArgumentException.h>

#include <cmath> // for fabs()

namespace geos {
namespace operation { // geos::operation
namespace intersection { // geos::operation::intersection

using namespace geos::geom;

RectangleIntersectionBuilder::~RectangleIntersectionBuilder()
{
    for(std::list<geom::Polygon*>::iterator i = polygons.begin(), e = polygons.end(); i != e; ++i) {
        delete *i;
    }
    for(std::list<geom::LineString*>::iterator i = lines.begin(), e = lines.end(); i != e; ++i) {
        delete *i;
    }
    for(std::list<geom::Point*>::iterator i = points.begin(), e = points.end(); i != e; ++i) {
        delete *i;
    }
}

void
RectangleIntersectionBuilder::reconnect()
{
    // Nothing to reconnect if there aren't at least two lines
    if(lines.size() < 2) {
        return;
    }

    geom::LineString* line1 = lines.front();
    const geom::CoordinateSequence& cs1 = *line1->getCoordinatesRO();

    geom::LineString* line2 = lines.back();
    const geom::CoordinateSequence& cs2 = *line2->getCoordinatesRO();

    const auto n1 = cs1.size();
    const auto n2 = cs2.size();

    // Safety check against bad input to prevent segfaults
    if(n1 == 0 || n2 == 0) {
        return;
    }

    if(cs1[0] != cs2[n2 - 1]) {
        return;
    }

    // Merge the two linestrings
    auto ncs = valid::RepeatedPointRemover::removeRepeatedPoints(&cs2);
    ncs->add(&cs1, false, true);

    delete line1;
    delete line2;

    LineString* nline = _gf.createLineString(ncs.release());
    lines.pop_front();
    lines.pop_back();

    lines.push_front(nline);
}


void
RectangleIntersectionBuilder::release(RectangleIntersectionBuilder& theParts)
{
    for(std::list<geom::Polygon*>::iterator i = polygons.begin(), e = polygons.end(); i != e; ++i) {
        theParts.add(*i);
    }

    for(std::list<geom::LineString*>::iterator i = lines.begin(), e = lines.end(); i != e; ++i) {
        theParts.add(*i);
    }

    for(std::list<geom::Point*>::iterator i = points.begin(), e = points.end(); i != e; ++i) {
        theParts.add(*i);
    }

    clear();
}

/**
 * \brief Clear the parts having transferred ownership elsewhere
 */

void
RectangleIntersectionBuilder::clear()
{
    polygons.clear();
    lines.clear();
    points.clear();
}

/**
 * \brief Test if there are no parts at all
 */

bool
RectangleIntersectionBuilder::empty() const
{
    return polygons.empty() && lines.empty() && points.empty();
}

/**
 * \brief Add intermediate Polygon
 */

void
RectangleIntersectionBuilder::add(geom::Polygon* thePolygon)
{
    polygons.push_back(thePolygon);
}

/**
 * \brief Add intermediate LineString
 */

void
RectangleIntersectionBuilder::add(geom::LineString* theLine)
{
    lines.push_back(theLine);
}

/**
 * \brief Add intermediate Point
 */

void
RectangleIntersectionBuilder::add(geom::Point* thePoint)
{
    points.push_back(thePoint);
}

std::unique_ptr<geom::Geometry>
RectangleIntersectionBuilder::build()
{
    // Total number of objects

    std::size_t n = polygons.size() + lines.size() + points.size();

    if(n == 0) {
        return std::unique_ptr<Geometry>(_gf.createGeometryCollection());
    }

    std::vector<Geometry*>* geoms = new std::vector<Geometry*>;
    geoms->reserve(n);

    for(std::list<geom::Polygon*>::iterator i = polygons.begin(), e = polygons.end(); i != e; ++i) {
        geoms->push_back(*i);
    }
    polygons.clear();

    for(std::list<geom::LineString*>::iterator i = lines.begin(), e = lines.end(); i != e; ++i) {
        geoms->push_back(*i);
    }
    lines.clear();

    for(std::list<geom::Point*>::iterator i = points.begin(), e = points.end(); i != e; ++i) {
        geoms->push_back(*i);
    }
    points.clear();

    return std::unique_ptr<Geometry>(
               (*geoms)[0]->getFactory()->buildGeometry(geoms)
           );
}

} // namespace geos::operation::intersection
} // namespace geos::operation
} // namespace geos
