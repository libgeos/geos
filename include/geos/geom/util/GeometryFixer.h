/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <memory>
#include <vector>

#include <geos/geom/Geometry.h>
#include <geos/export.h>

// Forward declarations
namespace geos {
namespace geom {
class GeometryFactory;
class Point;
class Polygon;
class LineString;
class LinearRing;
class MultiPoint;
class MultiPolygon;
class MultiLineString;
class GeometryCollection;
}
}

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

/**
 * Fixes a geometry to be a valid geometry, while preserving as much as
 * possible of the shape and location of the input.
 * Validity is determined according to {@link Geometry#isValid()}.
 *
 * Input geometries are always processed, so even valid inputs may
 * have some minor alterations.  The output is always a new geometry object.
 *
 * <h2>Semantic Rules</h2>
 *
 * * Vertices with non-finite X or Y ordinates are removed (as per Coordinate::isValid() ).
 *  * Repeated points are reduced to a single point
 *  * Empty atomic geometries are valid and are returned unchanged
 *  * Empty elements are removed from collections
 *  * <code>Point</code>: keep valid coordinate, or EMPTY
 *  * <code>LineString</code>: coordinates are fixed
 *  * <code>LinearRing</code>: coordinates are fixed, Keep valid ring or else convert into <code>LineString</code>
 *  * <code>Polygon</code>: transform into a valid polygon,
 *  * preserving as much of the extent and vertices as possible.
 *    * Rings are fixed to ensure they are valid<
 *    * Holes intersecting the shell are subtracted from the shell
 *    * Holes outside the shell are converted into polygons
 *  * <code>MultiPolygon</code>: each polygon is fixed,
 *    then result made non-overlapping (via union)
 *  * <code>GeometryCollection</code>: each element is fixed
 *  * Collapsed lines and polygons are handled as follows,
 *    depending on the <code>keepCollapsed</code> setting:
 *    * <code>false</code>: (default) collapses are converted to empty geometries
 *    * <code>true</code>: collapses are converted to a valid geometry of lower dimension
 *
 * @author Martin Davis
*/
class GEOS_DLL GeometryFixer {

private:

    const geom::Geometry* geom;
    const geom::GeometryFactory* factory;
    bool isKeepCollapsed; // false

public:

    GeometryFixer(const geom::Geometry* p_geom)
        : geom(p_geom)
        , factory(p_geom->getFactory())
        , isKeepCollapsed(false)
        {};

    static std::unique_ptr<geom::Geometry> fix(const geom::Geometry* geom);

    /**
    * Sets whether collapsed geometries are converted to empty,
    * (which will be removed from collections),
    * or to a valid geom::Geometry of lower dimension.
    * The default is to convert collapses to empty geometries.
    *
    * @param p_isKeepCollapsed whether collapses should be converted to a lower dimension geometry
    */
    void setKeepCollapsed(bool p_isKeepCollapsed);

    /**
    * Gets the fixed geometry.
    *
    * @return the fixed geometry
    */
    std::unique_ptr<geom::Geometry> getResult() const;

private:

    std::unique_ptr<geom::Point> fixPoint(const geom::Point* geom) const;
    std::unique_ptr<geom::Point> fixPointElement(const geom::Point* geom) const;
    bool isValidPoint(const geom::Point* pt) const;
    std::unique_ptr<geom::Geometry> fixMultiPoint(const geom::MultiPoint* geom) const;
    std::unique_ptr<geom::Geometry> fixLinearRing(const geom::LinearRing* geom) const;
    std::unique_ptr<geom::Geometry> fixLinearRingElement(const geom::LinearRing* geom) const;
    std::unique_ptr<geom::Geometry> fixLineString(const geom::LineString* geom) const;
    std::unique_ptr<geom::Geometry> fixLineStringElement(const geom::LineString* geom) const;

    /**
    * Returns a clean copy of the input coordinate array.
    *
    * @param pts coordinates to clean
    * @return an array of clean coordinates
    */
    std::unique_ptr<geom::Geometry> fixMultiLineString(const MultiLineString* geom) const;
    std::unique_ptr<geom::Geometry> fixPolygon(const geom::Polygon* geom) const;
    std::unique_ptr<geom::Geometry> fixPolygonElement(const geom::Polygon* geom) const;
    std::vector<std::unique_ptr<Geometry>> fixHoles(const geom::Polygon* geom) const;
    std::unique_ptr<geom::Geometry> removeHoles(const geom::Geometry* shell, const geom::Geometry* holes) const;
    std::unique_ptr<geom::Geometry> fixRing(const geom::LinearRing* ring) const;
    std::unique_ptr<geom::Geometry> fixMultiPolygon(const geom::MultiPolygon* geom) const;
    std::unique_ptr<geom::Geometry> fixCollection(const geom::GeometryCollection* geom) const;

    void classifyHoles(
        const Geometry* shell,
        std::vector<std::unique_ptr<Geometry>>& holesFixed,
        std::vector<const Geometry*>& holes,
        std::vector<const Geometry*>& shells) const;

    /**
    * Subtracts a list of polygonal geometries from a polygonal geometry.
    *
    * @param shell polygonal geometry for shell
    * @param holes polygonal geometries to subtract
    * @return the result geometry
    */
    std::unique_ptr<Geometry> difference(
        const Geometry* shell,
        std::vector<const Geometry*>& holes) const;

    /**
    * Unions a list of polygonal geometries.
    * Optimizes case of zero or one input geometries.
    * Requires that the inputs are net new objects.
    *
    * @param polys the polygonal geometries to union
    * @return the union of the inputs
    */
    std::unique_ptr<Geometry> unionGeometry(
        std::vector<const Geometry*>& polys) const;

};

} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos


