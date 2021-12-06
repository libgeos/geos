/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

namespace geos {
namespace geom {

class Coordinate;

class SegmentInspector {

public:
    virtual void inspect(const Coordinate& p0, const Coordinate& p1) = 0;

};

}
}
