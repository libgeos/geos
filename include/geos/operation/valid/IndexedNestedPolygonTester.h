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
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>

#include <memory>
#include <map>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class Polygon;
class LinearRing;
class MultiPolygon;
}
}


namespace geos {      // geos.
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using geos::geom::Polygon;
using geos::geom::MultiPolygon;
using geos::geom::LinearRing;
using geos::geom::CoordinateXY;
using algorithm::locate::IndexedPointInAreaLocator;
using index::strtree::TemplateSTRtree;

class GEOS_DLL IndexedNestedPolygonTester {

private:

    const MultiPolygon* multiPoly;
    TemplateSTRtree<const Polygon*> index;
    // std::vector<IndexedPointInAreaLocator> locators;
    std::map<const Polygon*, IndexedPointInAreaLocator> locators;
    CoordinateXY nestedPt;

    void loadIndex();

    IndexedPointInAreaLocator& getLocator(const Polygon* poly);

    bool findNestedPoint(const LinearRing* shell,
        const Polygon* possibleOuterPoly,
        IndexedPointInAreaLocator& locator,
        CoordinateXY& coordNested);

    /**
    * Finds a point of a shell segment which lies inside a polygon, if any.
    * The shell is assume to touch the polyon only at shell vertices,
    * and does not cross the polygon.
    *
    * @param the shell to test
    * @param the polygon to test against
    * @param coordNested return parametr for found coordinate
    * @return an interior segment point, or null if the shell is nested correctly
    */
    static bool findIncidentSegmentNestedPoint(
        const LinearRing* shell,
        const Polygon* poly,
        CoordinateXY& coordNested);

    // Declare type as noncopyable
    IndexedNestedPolygonTester(const IndexedNestedPolygonTester& other) = delete;
    IndexedNestedPolygonTester& operator=(const IndexedNestedPolygonTester& rhs) = delete;

public:

    IndexedNestedPolygonTester(const MultiPolygon* p_multiPoly);

    /**
    * Gets a point on a nested polygon, if one exists.
    *
    * @return a point on a nested polygon, or null if none are nested
    */
    const CoordinateXY& getNestedPoint() const { return nestedPt; }

    /**
    * Tests if any polygon is nested (contained) within another polygon.
    * This is invalid.
    * The nested point will be set to reflect this.
    * @return true if some polygon is nested
    */
    bool isNested();


};



} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
