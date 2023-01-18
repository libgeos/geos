/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2018 Daniel Baston <dbaston@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/


#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/util/LinearComponentExtracter.h>
#include <geos/index/strtree/Interval.h>
#include <geos/index/strtree/TemplateSTRtree.h>
#include <geos/util.h>
#include <geos/algorithm/RayCrossingCounter.h>
#include <geos/index/ItemVisitor.h>

#include <algorithm>
#include <typeinfo>

using geos::geom::CoordinateXY;

namespace geos {
namespace algorithm {
namespace locate {
//
// private:
//
IndexedPointInAreaLocator::IntervalIndexedGeometry::IntervalIndexedGeometry(const geom::Geometry& g)
{
    init(g);
}

void
IndexedPointInAreaLocator::IntervalIndexedGeometry::init(const geom::Geometry& g)
{
    geom::LineString::ConstVect lines;
    geom::util::LinearComponentExtracter::getLines(g, lines);

    // pre-compute size of segment vector
    std::size_t nsegs = 0;
    for(const geom::LineString* line : lines) {
        //-- only include rings of Polygons or LinearRings
        if (! line->isClosed())
          continue;

        nsegs += line->getCoordinatesRO()->size() - 1;
    }
    index = decltype(index)(10, nsegs);

    for(const geom::LineString* line : lines) {
        //-- only include rings of Polygons or LinearRings
        if (! line->isClosed())
          continue;

        addLine(line->getCoordinatesRO());
    }
}

void
IndexedPointInAreaLocator::IntervalIndexedGeometry::addLine(const geom::CoordinateSequence* pts)
{
    for(std::size_t i = 1, ni = pts->size(); i < ni; i++) {
        SegmentView seg(&pts->getAt<CoordinateXY>(i-1), &pts->getAt<CoordinateXY>(i));
        auto r = std::minmax(seg.p0().y, seg.p1().y);

        index.insert(index::strtree::Interval(r.first, r.second), seg);
    }
}


void
IndexedPointInAreaLocator::buildIndex(const geom::Geometry& g)
{
    index = detail::make_unique<IntervalIndexedGeometry>(g);
}


//
// protected:
//

//
// public:
//
IndexedPointInAreaLocator::IndexedPointInAreaLocator(const geom::Geometry& g)
    :	areaGeom(g)
{
}

geom::Location
IndexedPointInAreaLocator::locate(const geom::CoordinateXY* /*const*/ p)
{
    if (index == nullptr) {
        buildIndex(areaGeom);
    }

    algorithm::RayCrossingCounter rcc(*p);

    index->query(p->y, p->y, [&rcc](const SegmentView& ls) {
        rcc.countSegment(ls.p0(), ls.p1());
    });

    return rcc.getLocation();
}


} // geos::algorithm::locate
} // geos::algorithm
} // geos
