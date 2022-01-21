/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/triangulate/tri/TriList.h>
#include <geos/triangulate/tri/Tri.h>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class GeometryFactory;
class Polygon;
}
}

using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::Polygon;
using geos::triangulate::tri::TriList;
using geos::triangulate::tri::Tri;


namespace geos {
namespace triangulate {
namespace polygon {


/**
 * Computes a triangulation of each polygon in a {@link geos::geom::Geometry}.
 * A polygon triangulation is a non-overlapping set of triangles which
 * cover the polygon and have the same vertices as the polygon.
 * The priority is on performance rather than triangulation quality,
 * so that the output may contain many narrow triangles.
 *
 * Holes are handled by joining them to the shell to form a
 * (self-touching) polygon shell with no holes.
 * Although invalid, this can be triangulated effectively.
 *
 * For better-quality triangulation use geos::triangulate::polygon::ConstrainedDelaunayTriangulator.
 *
 * @see geos::triangulate::polygon::ConstrainedDelaunayTriangulator
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL PolygonTriangulator {

private:

    // Members

    const Geometry* inputGeom;
    const GeometryFactory* geomFact;

    std::unique_ptr<Geometry> compute();

    /**
    * Computes the triangulation of a single polygon
    *
    * @return GeometryCollection of triangular polygons
    */
    void triangulatePolygon(const Polygon* poly, TriList<Tri>& triList);


public:

    /**
    * Constructs a new triangulator.
    *
    * @param p_inputGeom the input geometry
    */
    PolygonTriangulator(const Geometry* p_inputGeom)
        : inputGeom(p_inputGeom)
        , geomFact(p_inputGeom->getFactory())
    {}

    /**
    * Computes a triangulation of each polygon in a geometry.
    *
    * @param geom a geometry containing polygons
    * @return a GeometryCollection containing the triangle polygons
    */
    static std::unique_ptr<Geometry> triangulate(const Geometry* geom);

};



} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos

