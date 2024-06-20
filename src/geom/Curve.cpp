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
 **********************************************************************/

#include <geos/geom/Curve.h>
#include <geos/geom/GeometryFilter.h>

namespace geos {
namespace geom {

void
Curve::apply_ro(GeometryComponentFilter* filter) const
{
    assert(filter);
    filter->filter_ro(this);
}

void
Curve::apply_ro(GeometryFilter* filter) const
{
    assert(filter);
    filter->filter_ro(this);
}

void
Curve::apply_rw(GeometryComponentFilter* filter)
{
    assert(filter);
    filter->filter_rw(this);
}

void
Curve::apply_rw(GeometryFilter* filter)
{
    assert(filter);
    filter->filter_rw(this);
}

bool
Curve::isRing() const
{
    return isClosed() && isSimple();
}


}
}
