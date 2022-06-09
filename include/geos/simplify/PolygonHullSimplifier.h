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

#include <geos/geom/Geometry.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/simplify/RingHull.h>


namespace geos {
namespace geom {
class GeometryFactory;
class LinearRing;
class MultiPolygon;
class Polygon;
}
namespace algorithm {
namespace hull {
class RingHullIndex;
}
}
}


using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LinearRing;
using geos::geom::MultiPolygon;
using geos::geom::Polygon;


namespace geos {
namespace simplify { // geos::simplify

/**
 * Computes topology-preserving simplified hulls of polygonal geometry.
 * Both outer and inner hulls can be computed.
 * Outer hulls contain the input geometry and are larger in area.
 * Inner hulls are contained by the input geometry and are smaller in area.
 * In both the hull vertices are a subset of the input vertices.
 * The hull construction attempts to minimize the area difference
 * with the input geometry.
 * Hulls are generally concave if the input is.
 * Computed hulls are topology-preserving:
 * they do not contain any self-intersections or overlaps,
 * so the result polygonal geometry is valid.
 *
 * Polygons with holes and MultiPolygons are supported.
 * The result has the same geometric type and structure as the input.
 *
 * The number of vertices in the computed hull is determined by a target parameter.
 * Two parameters are supported:
 *
 *  * Vertex Number fraction: the fraction of the input vertices retained in the result.
 *    Value 1 produces the original geometry.
 *    Smaller values produce less concave results.
 *    For outer hulls, value 0 produces the convex hull (with triangles for any holes).
 *    For inner hulls, value 0 produces a triangle (if no holes are present).
 *
 *  * Area Delta ratio: the ratio of the change in area to the input area.
 *    Value 0 produces the original geometry.
 *    Larger values produce less concave results.
 *
 * The algorithm ensures that the result does not cause the target parameter
 * to be exceeded.  This allows computing outer or inner hulls
 * with a small area delta ratio as an effective way of removing
 * narrow gores and spikes.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL PolygonHullSimplifier
{

public:

    /**
    * Creates a new instance
    * to compute a simplified hull of a polygonal geometry.
    * An outer or inner hull is computed
    * depending on the value of "isOuter".
    *
    * @param geom the polygonal geometry to process
    * @param bOuter indicates whether to compute an outer or inner hull
    */
    PolygonHullSimplifier(const Geometry* geom, bool bOuter)
        : inputGeom(geom)
        , geomFactory(geom->getFactory())
        , isOuter(bOuter)
        , vertexNumFraction(-1.0)
        , areaDeltaRatio(-1.0)
    {
        if (!geom->isPolygonal()) {
            throw util::IllegalArgumentException("Input geometry must be polygonal");
        }
    };

    /**
    * Computes a topology-preserving simplified hull of a polygonal geometry,
    * with hull shape determined by a target parameter
    * specifying the fraction of the input vertices retained in the result.
    * Larger values compute less concave results.
    * A value of 1 produces the convex hull; a value of 0 produces the original geometry.
    * Either outer or inner hulls can be computed.
    *
    * @param geom the polygonal geometry to process
    * @param isOuter indicates whether to compute an outer or inner hull
    * @param vertexNumFraction the target fraction of number of input vertices in result
    * @return the hull geometry
    */
    static std::unique_ptr<Geometry> hull(
        const Geometry* geom,
        bool isOuter,
        double vertexNumFraction);

    /**
    * Computes a topology-preserving simplified hull of a polygonal geometry,
    * with hull shape determined by a target parameter
    * specifying the ratio of maximum difference in area to original area.
    * Larger values compute less concave results.
    * A value of 0 produces the original geometry.
    * Either outer or inner hulls can be computed..
    *
    * @param geom the polygonal geometry to process
    * @param isOuter indicates whether to compute an outer or inner hull
    * @param areaDeltaRatio the target ratio of area difference to original area
    * @return the hull geometry
    */
    static std::unique_ptr<Geometry> hullByAreaDelta(
        const Geometry* geom,
        bool isOuter,
        double areaDeltaRatio);


    /**
    * Sets the target fraction of input vertices
    * which are retained in the result.
    * The value should be in the range [0,1].
    *
    * @param p_vertexNumFraction a fraction of the number of input vertices
    */
    void setVertexNumFraction(double p_vertexNumFraction);
    /**
    * Sets the target maximum ratio of the change in area of the result to the input area.
    * The value must be 0 or greater.
    *
    * @param p_areaDeltaRatio a ratio of the change in area of the result
    */
    void setAreaDeltaRatio(double p_areaDeltaRatio);

    /**
    * Gets the result polygonal hull geometry.
    *
    * @return the polygonal geometry for the hull
    */
    std::unique_ptr<Geometry> getResult();



private:

    // Members
    const Geometry* inputGeom;
    const GeometryFactory* geomFactory;
    bool isOuter;
    double vertexNumFraction;
    double areaDeltaRatio;
    // Allocate the RingHull* in here so they are cleaned
    // up with PolygonHullSimplifier
    std::vector<std::unique_ptr<RingHull>> ringStore;

    /**
    * Computes hulls for MultiPolygon elements for
    * the cases where hulls might overlap.
    *
    * @param multiPoly the MultiPolygon to process
    * @return the hull geometry
    */
    std::unique_ptr<Geometry> computeMultiPolygonAll(const MultiPolygon* multiPoly);
    std::unique_ptr<Geometry> computeMultiPolygonEach(const MultiPolygon* multiPoly);
    std::unique_ptr<Polygon> computePolygon(const Polygon* poly);

    /**
    * Create all ring hulls for the rings of a polygon,
    * so that all are in the hull index if required.
    *
    * @param poly the polygon being processed
    * @param hullIndex the hull index if present, or null
    * @return the list of ring hulls
    */
    std::vector<RingHull*> initPolygon(const Polygon* poly,
        RingHullIndex& hullIndex);

    double ringArea(const Polygon* poly) const;

    RingHull* createRingHull(
        const LinearRing* ring,
        bool isOuter,
        double areaTotal,
        RingHullIndex& hullIndex);

    std::unique_ptr<Polygon> polygonHull(
        const Polygon* poly,
        std::vector<RingHull*>& ringHulls,
        RingHullIndex& hullIndex) const;

    /**
     * Disable copy construction and assignment. Needed to make this
     * class compile under MSVC. (See https://stackoverflow.com/q/29565299)
     */
    PolygonHullSimplifier(const PolygonHullSimplifier&) = delete;
    PolygonHullSimplifier& operator=(const PolygonHullSimplifier&) = delete;

}; // PolygonHullSimplifier


} // geos::simplify
} // geos

