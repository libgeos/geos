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
#include <geos/geom/MultiLineString.h>
#include <geos/geom/LineString.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Dimension.h>

#include <vector>
#include <cassert>

#ifndef GEOS_INLINE
# include "geos/geom/MultiLineString.inl"
#endif

using namespace std;
using namespace geos::algorithm;
//using namespace geos::operation;
using namespace geos::geomgraph;

namespace geos {
namespace geom { // geos::geom

/*protected*/
MultiLineString::MultiLineString(vector<Geometry*>* newLines,
                                 const GeometryFactory* factory)
    :
    Geometry(factory),
    GeometryCollection(newLines, factory)
{
}

MultiLineString::~MultiLineString() {}

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

string
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
        LineString* ls = dynamic_cast<LineString*>(g.get());
        if(! ls->isClosed()) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<Geometry>
MultiLineString::getBoundary() const
{
    if(isEmpty()) {
        return std::unique_ptr<Geometry>(getFactory()->createGeometryCollection(nullptr));
    }

    GeometryGraph gg(0, this);
    CoordinateSequence* pts = gg.getBoundaryPoints();
    return std::unique_ptr<Geometry>(getFactory()->createMultiPoint(*pts));
}

bool
MultiLineString::equalsExact(const Geometry* other, double tolerance) const
{
    if(!isEquivalentClass(other)) {
        return false;
    }
    return GeometryCollection::equalsExact(other, tolerance);
}
GeometryTypeId
MultiLineString::getGeometryTypeId() const
{
    return GEOS_MULTILINESTRING;
}

std::unique_ptr<Geometry>
MultiLineString::reverse() const
{
    if(isEmpty()) {
        return clone();
    }

    size_t nLines = geometries.size();
    Geometry::NonConstVect* revLines = new Geometry::NonConstVect(nLines);
    for(size_t i = 0; i < nLines; ++i) {
        LineString* iLS = dynamic_cast<LineString*>(geometries[i].get());
        assert(iLS);
        (*revLines)[nLines - 1 - i] = iLS->reverse().release();
    }
    return std::unique_ptr<Geometry>(getFactory()->createMultiLineString(revLines));
}

} // namespace geos::geom
} // namespace geos
