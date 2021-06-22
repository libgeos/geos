/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/index/strtree/TemplateSTRtree.h>

#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class Polygon;
class LinearRing;
}
}


namespace geos {      // geos.
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using geos::geom::Polygon;
using geos::geom::LinearRing;
using geos::geom::Coordinate;


class GEOS_DLL IndexedNestedHoleTester {

private:

    const Polygon* polygon;
    index::strtree::TemplateSTRtree<const LinearRing*> index;
    const Coordinate* nestedPt = nullptr;

    void loadIndex();

    bool isHoleInsideHole(const LinearRing* hole, const LinearRing* testHole);


public:

    IndexedNestedHoleTester(const Polygon* p_polygon)
        : polygon(p_polygon)
    {
        loadIndex();
    }

    const Coordinate* getNestedPoint() { return nestedPt; }

    bool isNested();

};



} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

