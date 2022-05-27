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
 * Computes the Constrained Delaunay Triangulation of polygons.
 * The Constrained Delaunay Triangulation of a polygon is a set of triangles
 * covering the polygon, with the maximum total interior angle over all
 * possible triangulations.  It provides the "best quality" triangulation
 * of the polygon.
 * <p>
 * Holes are supported.
 */
class GEOS_DLL ConstrainedDelaunayTriangulator {

private:

    // Members
    const Geometry* inputGeom;
    const GeometryFactory* geomFact;

    std::unique_ptr<Geometry> compute();

    static std::unique_ptr<Geometry> toGeometry(
        const geom::GeometryFactory* geomFact,
        const std::vector<std::unique_ptr<TriList<Tri>>>& allTriLists);


public:

    /**
    * Constructs a new triangulator.
    *
    * @param p_inputGeom the input geometry
    */
    ConstrainedDelaunayTriangulator(const Geometry* p_inputGeom)
        : inputGeom(p_inputGeom)
        , geomFact(p_inputGeom->getFactory())
    {}

    /**
    * Computes the Constrained Delaunay Triangulation of each polygon element in a geometry.
    *
    * @param geom the input geometry
    * @return a GeometryCollection of the computed triangle polygons
    */
    static std::unique_ptr<Geometry> triangulate(const Geometry* geom);

    /**
    * Computes the triangulation of a single polygon
    * and returns it as a list of {@link geos::triangulate::tri::Tri}s.
    *
    * @param poly the input polygon
    * @param triList the list to store the triangulation in
    */
    static void triangulatePolygon(const Polygon* poly, TriList<Tri>& triList);

};



} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos

