/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <memory>
#include <utility>

// Forward declarations
namespace geos::geom {
class CircularString;
class CompoundCurve;
class CoordinateXY;
class Curve;
class LineString;
class SimpleCurve;
}

namespace geos::operation::split {

class GEOS_DLL SplitLinealAtPoint {

public:
    /** Split a curve into two sections at a specified vertex.
     *
     * If the vertex is the first or last vertex of the geometry, one of the returned
     * geometries will be empty.
     *
     * If the input geometry is a CircularString, the vertex must be an endpoint of an arc.
     *
     * @param sc the curve to split
     * @param i the index of the vertex to split at.
     * @return a pair of split geometries.
     */
    static std::pair<std::unique_ptr<geom::SimpleCurve>, std::unique_ptr<geom::SimpleCurve>>
    splitSimpleCurveAtVertex(const geom::SimpleCurve& sc, std::size_t i);

    /** Split a SimpleCurve into two sections at an arbitrary point.
     *
     * @param sc the curve to split
     * @param i the index of the vertex of the section (segment or arc) that contains pt
     * @param pt the splitting point
     * @return a pair of split geometries.
     */
    static std::pair<std::unique_ptr<geom::SimpleCurve>, std::unique_ptr<geom::SimpleCurve>>
    splitSimpleCurveAtPoint(const geom::SimpleCurve &sc, std::size_t i, const geom::CoordinateXY &pt);

    /** Split a CompoundCurve into two sections at an arbitrary point.
     *
     * @param sc the curve to split
     * @param i the component that contains the splitting vertex
     * @param j the index of the vertex of the section (segment or arc) that contains pt
     * @param pt the splitting point
     * @return a pair of split geometries.
     */
    static std::pair<std::unique_ptr<geom::Curve>, std::unique_ptr<geom::Curve>>
    splitCompoundCurveAtPoint(const geom::CompoundCurve &sc, std::size_t i, std::size_t j, const geom::CoordinateXY &pt);

    /** Split a CircularString into two sections at an arbitrary point. */
    static std::pair<std::unique_ptr<geom::CircularString>, std::unique_ptr<geom::CircularString>>
    splitCircularStringAtPoint(const geom::CircularString &ls, std::size_t i, const geom::CoordinateXY &pt);

    /** Split CircularString into two sections at vertex i.
     *  The vertex must be an endpoint of an arc. */
    static std::pair<std::unique_ptr<geom::CircularString>, std::unique_ptr<geom::CircularString>>
    splitCircularStringAtVertex(const geom::CircularString &cs, std::size_t i);

    /** Split a LineString into two sections at an arbitrary point. */
    static std::pair<std::unique_ptr<geom::LineString>, std::unique_ptr<geom::LineString>>
    splitLineStringAtPoint(const geom::LineString &ls, std::size_t i, const geom::CoordinateXY &pt);

    /** Split LineString into two sections at vertex i */
    static std::pair<std::unique_ptr<geom::LineString>, std::unique_ptr<geom::LineString>>
    splitLineStringAtVertex(const geom::LineString &ls, std::size_t i);

};

}
