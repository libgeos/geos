/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/MultiLineString.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/geomgraph/GeometryGraph.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiLineString.h>
#include <geos/operation/BoundaryOp.h>
#include <geos/util.h>

#include <vector>
#include <cassert>

using geos::geom::GeometryFactory;
using geos::geom::MultiLineString;
using geos::geomgraph::GeometryGraph;

namespace geos {
namespace geom { // geos::geom

/*protected*/
MultiLineString::MultiLineString(std::vector<std::unique_ptr<LineString>> && newLines,
        const GeometryFactory& factory)
        : GeometryCollection(std::move(newLines), factory)
{}

MultiLineString::MultiLineString(std::vector<std::unique_ptr<Geometry>> && newLines,
                                 const GeometryFactory& factory)
        : GeometryCollection(std::move(newLines), factory)
{}

Dimension::DimensionType
MultiLineString::getDimension() const
{
    return Dimension::L; // line
}

int
MultiLineString::getBoundaryDimension() const
{
    if(isClosed()) {
        return Dimension::False;
    }
    return 0;
}

std::string
MultiLineString::getGeometryType() const
{
    return "MultiLineString";
}

bool
MultiLineString::isClosed() const
{
    if(isEmpty()) {
        return false;
    }
    for(const auto& g : geometries) {
        const LineString* ls = detail::down_cast<const LineString*>(g.get());
        if(! ls->isClosed()) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<Geometry>
MultiLineString::getBoundary() const
{
    operation::BoundaryOp bop(*this);
    return bop.getBoundary();
}

GeometryTypeId
MultiLineString::getGeometryTypeId() const
{
    return GEOS_MULTILINESTRING;
}

MultiLineString*
MultiLineString::reverseImpl() const
{
    if(isEmpty()) {
        return clone().release();
    }

    std::vector<std::unique_ptr<Geometry>> reversed(geometries.size());

    std::transform(geometries.begin(),
                   geometries.end(),
                   reversed.begin(),
                   [](const std::unique_ptr<Geometry> & g) {
                       return g->reverse();
                   });

    return getFactory()->createMultiLineString(std::move(reversed)).release();
}

const LineString*
MultiLineString::getGeometryN(std::size_t i) const
{
    return static_cast<const LineString*>(geometries[i].get());
}

} // namespace geos::geom
} // namespace geos
