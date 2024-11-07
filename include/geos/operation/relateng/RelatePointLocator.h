/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/algorithm/BoundaryNodeRule.h>
#include <geos/algorithm/locate/PointOnGeometryLocator.h>
#include <geos/operation/relateng/AdjacentEdgeLocator.h>
#include <geos/operation/relateng/LinearBoundary.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Location.h>
#include <geos/export.h>

#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace algorithm {
    namespace locate {
        // class PointOnGeometryLocator;
    }
}
namespace operation {
    namespace relateng {
        // class LinearBoundary;
        // class AdjacentEdgeLocator;
    }
}
namespace geom {
    class CoordinateXY;
    class Geometry;
    class LineString;
    class Point;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng


/**
 * Locates a point on a geometry, including mixed-type collections.
 * The dimension of the containing geometry element is also determined.
 * GeometryCollections are handled with union semantics;
 * i.e. the location of a point is that location of that point
 * on the union of the elements of the collection.
 *
 * Union semantics for GeometryCollections has the following behaviours:
 *
 *  * For a mixed-dimension (heterogeneous) collection
 *    a point may lie on two geometry elements with different dimensions.
 *    In this case the location on the largest-dimension element is reported.
 *  * For a collection with overlapping or adjacent polygons,
 *    points on polygon element boundaries may lie in the effective interior
 *    of the collection geometry.
 *
 * Prepared mode is supported via cached spatial indexes.
 *
 * @author Martin Davis
 */
class GEOS_DLL RelatePointLocator {
    using BoundaryNodeRule = geos::algorithm::BoundaryNodeRule;
    using PointOnGeometryLocator = geos::algorithm::locate::PointOnGeometryLocator;
    using Coordinate = geos::geom::Coordinate;
    using CoordinateXY = geos::geom::CoordinateXY;
    using Geometry = geos::geom::Geometry;
    using LineString = geos::geom::LineString;
    using Point = geos::geom::Point;
    using Location = geos::geom::Location;

private:

    // Members

    const Geometry* geom;
    bool isPrepared = false;
    const BoundaryNodeRule& boundaryRule;
    std::unique_ptr<AdjacentEdgeLocator> adjEdgeLocator;
    Coordinate::ConstXYSet points;
    std::vector<const LineString *> lines;
    std::vector<const Geometry *> polygons;
    std::vector<std::unique_ptr<PointOnGeometryLocator>> polyLocator;
    std::unique_ptr<LinearBoundary> lineBoundary;
    bool isEmpty;


public:

    // Constructors

    RelatePointLocator(const Geometry* p_geom)
        : RelatePointLocator(p_geom, false, BoundaryNodeRule::getBoundaryRuleMod2())
        {};

    RelatePointLocator(const Geometry* p_geom, bool p_isPrepared, const BoundaryNodeRule& p_bnRule)
        : geom(p_geom)
        , isPrepared(p_isPrepared)
        , boundaryRule(p_bnRule)
    {
        init(geom);
    };

    void init(const Geometry* p_geom);

    bool hasBoundary() const;

    void extractElements(const Geometry* geom);

    void addPoint(const Point* pt);

    void addLine(const LineString* line);

    void addPolygonal(const Geometry* polygonal);

    Location locate(const CoordinateXY* p);

    int locateLineEndWithDim(const CoordinateXY* p);

    /*
    * Locates a point which is known to be a node of the geometry
    * (i.e. a vertex or on an edge).
    *
    * @param p the node point to locate
    * @param parentPolygonal the polygon the point is a node of
    * @return the location of the node point
    */
    Location locateNode(const CoordinateXY* p, const Geometry* parentPolygonal);

    /**
    * Locates a point which is known to be a node of the geometry,
    * as a DimensionLocation.
    *
    * @param p the point to locate
    * @param parentPolygonal the polygon the point is a node of
    * @return the dimension and location of the point
    */
    int locateNodeWithDim(const CoordinateXY* p, const Geometry* parentPolygonal);

    /**
    * Computes the topological location ( Location) of a single point
    * in a Geometry, as well as the dimension of the geometry element the point
    * is located in (if not in the Exterior).
    * It handles both single-element and multi-element Geometries.
    * The algorithm for multi-part Geometries
    * takes into account the SFS Boundary Determination Rule.
    *
    * @param p the point to locate
    * @return the Location of the point relative to the input Geometry
    */
    int locateWithDim(const CoordinateXY* p);


private:

    // Methods

    /**
    * Computes the topological location (Location) of a single point
    * in a Geometry, as well as the dimension of the geometry element the point
    * is located in (if not in the Exterior).
    * It handles both single-element and multi-element Geometries.
    * The algorithm for multi-part Geometries
    * takes into account the SFS Boundary Determination Rule.
    *
    * @param p the coordinate to locate
    * @param isNode whether the coordinate is a node (on an edge) of the geometry
    * @param polygon
    * @return the Location of the point relative to the input Geometry
    */
    int locateWithDim(const CoordinateXY* p, bool isNode, const Geometry* parentPolygonal);

    int computeDimLocation(const CoordinateXY* p, bool isNode, const Geometry* parentPolygonal);

    Location locateOnPoints(const CoordinateXY* p) const;

    Location locateOnLines(const CoordinateXY* p, bool isNode);

    Location locateOnLine(const CoordinateXY* p, /*bool isNode,*/ const LineString* l);

    Location locateOnPolygons(const CoordinateXY* p, bool isNode, const Geometry* parentPolygonal);

    Location locateOnPolygonal(const CoordinateXY* p,
        bool isNode,
        const Geometry* parentPolygonal,
        std::size_t index);

    PointOnGeometryLocator * getLocator(std::size_t index);



};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

