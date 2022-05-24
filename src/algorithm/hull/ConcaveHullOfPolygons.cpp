

/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Polygon.h>
#include <geos/operation/overlayng/CoverageUnion.h>
#include <geos/triangulate/polygon/ConstrainedDelaunayTriangulator.h>
#include <geos/triangulate/tri/Tri.h>
#include <geos/util/IllegalArgumentException.h>


using geos::geom::Coordinate;
using geos::geom::Geometry;
using geos::geom::Envelope;
using geos::geom::GeometryCollection;
using geos::geom::GeometryFactory;
using geos::geom::LinearRing;
using geos::geom::MultiPolygon;
using geos::geom::Polygon;
using geos::operation::overlayng::CoverageUnion;
using geos::triangulate::polygon::ConstrainedDelaunayTriangulator;
using geos::triangulate::tri::Tri;


namespace geos {
namespace algorithm { // geos.algorithm
namespace hull {      // geos.algorithm.hulll

/**
* Computes a concave hull of set of polygons
* using the target criterion of maximum edge length.
*
* @param polygons the input polygons
* @param maxLength the target maximum edge length
* @return the concave hull
*/
/* public static */
std::unique_ptr<Geometry>
concaveHullByLength(const Geometry* polygons, double maxLength)
{
    return concaveHullByLength(polygons, maxLength, false, false);
}

/**
* Computes a concave hull of set of polygons
* using the target criterion of maximum edge length,
* and allowing control over whether the hull boundary is tight
* and can contain holes.
*
* @param polygons the input polygons
* @param maxLength the target maximum edge length
* @param isTight true if the hull should be tight to the outside of the polygons
* @param isHolesAllowed true if holes are allowed in the hull polygon
* @return the concave hull
*/
/* public static */
std::unique_ptr<Geometry>
concaveHullByLength(
    const Geometry* polygons, double maxLength,
    bool isTight, bool isHolesAllowed)
{
    ConcaveHullOfPolygons hull(polygons);
    hull.setMaximumEdgeLength(maxLength);
    hull.setHolesAllowed(isHolesAllowed);
    hull.setTight(isTight);
    return hull.getHull();
}

/**
* Computes a concave hull of set of polygons
* using the target criterion of maximum edge length ratio.
*
* @param polygons the input polygons
* @param lengthRatio the target maximum edge length ratio
* @return the concave hull
*/
/* public static */
std::unique_ptr<Geometry>
concaveHullByLengthRatio(const Geometry* polygons, double lengthRatio)
{
    return concaveHullByLengthRatio(polygons, lengthRatio, false, false);
}

/**
* Computes a concave hull of set of polygons
* using the target criterion of maximum edge length ratio,
* and allowing control over whether the hull boundary is tight
* and can contain holes.
*
* @param polygons the input polygons
* @param lengthRatio the target maximum edge length ratio
* @param isTight true if the hull should be tight to the outside of the polygons
* @param isHolesAllowed true if holes are allowed in the hull polygon
* @return the concave hull
*/
/* public static */
std::unique_ptr<Geometry>
concaveHullByLengthRatio(
    const Geometry* polygons, double lengthRatio,
    bool isTight, bool isHolesAllowed)
{
    ConcaveHullOfPolygons hull(polygons);
    hull.setMaximumEdgeLengthRatio(lengthRatio);
    hull.setHolesAllowed(isHolesAllowed);
    hull.setTight(isTight);
    return hull.getHull();
}

/**
* Computes a concave fill area between a set of polygons,
* using the target criterion of maximum edge length.
*
* @param polygons the input polygons
* @param maxLength the target maximum edge length
* @return the concave fill
*/
/* public static */
std::unique_ptr<Geometry>
concaveFillByLength(const Geometry* polygons, double maxLength)
{
    ConcaveHullOfPolygons hull(polygons);
    hull.setMaximumEdgeLength(maxLength);
    return hull.getFill();
}

/**
* Computes a concave fill area between a set of polygons,
* using the target criterion of maximum edge length ratio.
*
* @param polygons the input polygons
* @param lengthRatio the target maximum edge length ratio
* @return the concave fill
*/
/* public static */
std::unique_ptr<Geometry>
concaveFillByLengthRatio(const Geometry* polygons, double lengthRatio)
{
    ConcaveHullOfPolygons hull(polygons);
    hull.setMaximumEdgeLengthRatio(lengthRatio);
    return hull.getFill();
}


/**
* Creates a new instance for a given geometry.
*
* @param geom the input geometry
*/
/* public */
ConcaveHullOfPolygons(const Geometry* polygons)
    : inputPolygons(polygons)
    , geomFactory(polygons->getFactory())
{
    if (! polygons->isPolygonal()) {
        throw util::IllegalArgumentException("Input must be polygonal");
    }
}

/**
* Sets the target maximum edge length for the concave hull.
* The length value must be zero or greater.
* <ul>
* <li>The value 0.0 produces the input polygons.
* <li>Larger values produce less concave results.
* Above a certain large value the result is the convex hull of the input.
* <p>
* The edge length ratio provides a scale-free parameter which
* is intended to produce similar concave results for a variety of inputs.
*
* @param edgeLength a non-negative length
*/
/* public */
void
setMaximumEdgeLength(double edgeLength)
{
    if (edgeLength < 0)
        throw util::IllegalArgumentException("Edge length must be non-negative");
    maxEdgeLength = edgeLength;
    maxEdgeLengthRatio = -1;
}

/**
* Sets the target maximum edge length ratio for the concave hull.
* The edge length ratio is a fraction of the difference
* between the longest and shortest edge lengths
* in the Delaunay Triangulation of the area between the input polygons.
* (Roughly speaking, it is a fraction of the difference between
* the shortest and longest distances between the input polygons.)
* It is a value in the range 0 to 1.
* <ul>
* <li>The value 0.0 produces the original input polygons.
* <li>The value 1.0 produces the convex hull.
* <ul>
*
* @param edgeLengthRatio a length factor value between 0 and 1
*/
/* public */
void
setMaximumEdgeLengthRatio(double edgeLengthRatio)
{
    if (edgeLengthRatio < 0 || edgeLengthRatio > 1)
        throw util::IllegalArgumentException("Edge length ratio must be in range [0,1]");
    maxEdgeLengthRatio = edgeLengthRatio;
}

/**
* Sets whether holes are allowed in the concave hull polygon.
*
* @param p_isHolesAllowed true if holes are allowed in the result
*/
/* public */
void
setHolesAllowed(bool p_isHolesAllowed)
{
    isHolesAllowed = p_isHolesAllowed;
}

/**
* Sets whether the boundary of the hull polygon is kept
* tight to the outer edges of the input polygons.
*
* @param p_isTight true if the boundary is kept tight
*/
/* public */
void
setTight(bool p_isTight)
{
    isTight = p_isTight;
}

/**
* Gets the computed concave hull.
*
* @return the concave hull
*/
/* public */
std::unique_ptr<Geometry>
getHull()
{
    if (inputPolygons.isEmpty()) {
        return createEmptyHull();
    }
    buildHullTris();
    std::unique_ptr<Geometry> hull = createHullGeometry(hullTris, true);
    return hull;
}

  /**
   * Gets the concave fill, which is the area between the input polygons,
   * subject to the concaveness control parameter.
   *
   * @return the concave fill
   */
/* public */
std::unique_ptr<Geometry>
getFill()
{
    isTight = true;
    if (inputPolygons.isEmpty()) {
      return createEmptyHull();
    }
    buildHullTris();
    Geometry fill = createHullGeometry(hullTris, false);
    return fill;
}

/* private */
std::unique_ptr<Geometry>
createEmptyHull()
{
    return geomFactory.createPolygon();
}

/* private */
void
buildHullTris()
{
    polygonRings = extractShellRings(inputPolygons);
    Polygon frame = createFrame(inputPolygons.getEnvelopeInternal(), polygonRings, geomFactory);
    ConstrainedDelaunayTriangulator cdt = new ConstrainedDelaunayTriangulator(frame);
    List<Tri> tris = cdt.getTriangles();
    //System.out.println(tris);

    Coordinate[] framePts = frame.getExteriorRing().getCoordinates();
    if (maxEdgeLengthRatio >= 0) {
      maxEdgeLength = computeTargetEdgeLength(tris, framePts, maxEdgeLengthRatio);
    }

    hullTris = removeFrameCornerTris(tris, framePts);

    removeBorderTris();
    if (isHolesAllowed) removeHoleTris();
}

/* private static */
double computeTargetEdgeLength(List<Tri> triList,
      Coordinate[] frameCorners,
      double edgeLengthRatio) {
    if (edgeLengthRatio == 0) return 0;
    double maxEdgeLen = -1;
    double minEdgeLen = -1;
    for (Tri tri : triList) {
      //-- don't include frame triangles
      if (isFrameTri(tri, frameCorners))
        continue;

      for (int i = 0; i < 3; i++) {
        //-- constraint edges are not used to determine ratio
        if (! tri.hasAdjacent(i))
          continue;

        double len = tri.getLength(i);
        if (len > maxEdgeLen)
          maxEdgeLen = len;
        if (minEdgeLen < 0 || len < minEdgeLen)
          minEdgeLen = len;
      }
    }
    //-- if ratio = 1 ensure all edges are included
    if (edgeLengthRatio == 1)
      return 2 * maxEdgeLen;

    return edgeLengthRatio * (maxEdgeLen - minEdgeLen) + minEdgeLen;
}

/* private static */
bool isFrameTri(Tri tri, Coordinate[] frameCorners)
{
    int index = vertexIndex(tri, frameCorners);
    bool isFrameTri = index >= 0;
    return isFrameTri;
}

/* private */
Set<Tri> removeFrameCornerTris(List<Tri> tris, Coordinate[] frameCorners)
{
    Set<Tri> hullTris = new HashSet<Tri>();
    borderTriQue = new ArrayDeque<Tri>();
    for (Tri tri : tris) {
      int index = vertexIndex(tri, frameCorners);
      bool isFrameTri = index >= 0;
      if (isFrameTri) {
        /**
         * Frame tris are adjacent to at most one border tri,
         * which is opposite the frame corner vertex.
         * The opposite tri may be another frame tri.
         * This is detected when it is processed,
         * since it is not in the hullTri set.
         */
        int oppIndex = Tri.oppEdge(index);
        addBorderTri(tri, oppIndex);
        tri.remove();
      }
      else {
        hullTris.add(tri);
      }
    }
    return hullTris;
}

  /**
   * Get the tri vertex index of some point in a list,
   * or -1 if none are vertices.
   *
   * @param tri the tri to test for containing a point
   * @param pts the points to test
   * @return the vertex index of a point, or -1
   */
/* private static */
int vertexIndex(Tri tri, Coordinate[] pts)
{
    for (Coordinate p : pts) {
      int index = tri.getIndex(p);
      if (index >= 0)
        return index;
    }
    return -1;
}

/* private */
void
removeBorderTris()
{
    while (! borderTriQue.isEmpty()) {
      Tri tri = borderTriQue.pop();
      //-- tri might have been removed already
      if (! hullTris.contains(tri)) {
        continue;
      }
      if (isRemovable(tri)) {
        addBorderTris(tri);
        removeBorderTri(tri);
      }
    }
}

/* private */
void
removeHoleTris()
{
    while (true) {
      Tri holeTri = findHoleTri(hullTris);
      if (holeTri == null)
        return;
      addBorderTris(holeTri);
      removeBorderTri(holeTri);
      removeBorderTris();
    }
}

/* private */
Tri findHoleTri(Set<Tri> tris)
{
    for (Tri tri : tris) {
      if (isHoleTri(tri))
        return tri;
    }
    return null;
}

/* private */
bool isHoleTri(Tri tri)
{
    for (int i = 0; i < 3; i++) {
      if (tri.hasAdjacent(i)
          && tri.getLength(i) > maxEdgeLength)
         return true;
    }
    return false;
}

/* private */
bool isRemovable(Tri tri)
{
    //-- remove non-bridging tris if keeping hull boundary tight
    if (isTight && isTouchingSinglePolygon(tri))
      return true;

    //-- check if outside edge is longer than threshold
    if (borderEdgeMap.containsKey(tri)) {
      int borderEdgeIndex = borderEdgeMap.get(tri);
      double edgeLen = tri.getLength(borderEdgeIndex);
      if (edgeLen > maxEdgeLength)
        return true;
    }
    return false;
}

  /**
   * Tests whether a triangle touches a single polygon at all vertices.
   * If so, it is a candidate for removal if the hull polygon
   * is being kept tight to the outer boundary of the input polygons.
   * Tris which touch more than one polygon are called "bridging".
   *
   * @param tri
   * @return true if the tri touches a single polygon
   */
/* private */
bool isTouchingSinglePolygon(Tri tri)
{
    Envelope envTri = envelope(tri);
    for (LinearRing ring : polygonRings) {
      //-- optimization heuristic: a touching tri must be in ring envelope
      if (ring.getEnvelopeInternal().intersects(envTri)) {
        if (hasAllVertices(ring, tri))
          return true;
      }
    }
    return false;
}

/* private */
void
addBorderTris(Tri tri)
{
    addBorderTri(tri, 0);
    addBorderTri(tri, 1);
    addBorderTri(tri, 2);
}

  /**
   * Adds an adjacent tri to the current border.
   * The adjacent edge is recorded as the border edge for the tri.
   * Note that only edges adjacent to another tri can become border edges.
   * Since constraint-adjacent edges do not have an adjacent tri,
   * they can never be on the border and thus will not be removed
   * due to being shorter than the length threshold.
   * The tri containing them may still be removed via another edge, however.
   *
   * @param tri the tri adjacent to the tri to be added to the border
   * @param index the index of the adjacent tri
   */
/* private */
void
addBorderTri(Tri tri, int index)
{
    Tri adj = tri.getAdjacent( index );
    if (adj == null)
      return;
    borderTriQue.add(adj);
    int borderEdgeIndex = adj.getIndex(tri);
    borderEdgeMap.put(adj, borderEdgeIndex);
}

/* private */
void
removeBorderTri(Tri tri)
{
    tri.remove();
    hullTris.remove(tri);
    borderEdgeMap.remove(tri);
}

/* private static */
bool hasAllVertices(LinearRing ring, Tri tri)
{
    for (int i = 0; i < 3; i++) {
      Coordinate v = tri.getCoordinate(i);
      if (! hasVertex(ring, v)) {
        return false;
      }
    }
    return true;
}

/* private static */
bool hasVertex(LinearRing ring, Coordinate v)
{
    for(int i = 1; i < ring.getNumPoints(); i++) {
      if (v.equals2D(ring.getCoordinateN(i))) {
        return true;
      }
    }
    return false;
}

/* private static */
Envelope envelope(Tri tri)
{
    Envelope env = new Envelope(tri.getCoordinate(0), tri.getCoordinate(1));
    env.expandToInclude(tri.getCoordinate(2));
    return env;
}

/* private */
std::unique_ptr<Geometry>
createHullGeometry(Set<Tri> hullTris, bool isIncludeInput)
{
    if (! isIncludeInput && hullTris.isEmpty())
      return createEmptyHull();

    //-- union triangulation
    Geometry triCoverage = Tri.toGeometry(hullTris, geomFactory);
    //System.out.println(triCoverage);
    Geometry fillGeometry = CoverageUnion.union(triCoverage);

    if (! isIncludeInput) {
      return fillGeometry;
    }
    if (fillGeometry.isEmpty()) {
      return inputPolygons.copy();
    }
    //-- union with input polygons
    Geometry[] geoms = new Geometry[] { fillGeometry, inputPolygons };
    GeometryCollection geomColl = geomFactory.createGeometryCollection(geoms);
    Geometry hull = CoverageUnion.union(geomColl);
    return hull;
}

  /**
   * Creates a rectangular "frame" around the input polygons,
   * with the input polygons as holes in it.
   * The frame is large enough that the constrained Delaunay triangulation
   * of it should contain the convex hull of the input as edges.
   * The frame corner triangles can be removed to produce a
   * triangulation of the space around and between the input polygons.
   *
   * @param polygonsEnv
   * @param polygonRings
   * @param geomFactory
   * @return the frame polygon
   */
/* private static */
std::unique_ptr<Polygon>
createFrame(Envelope polygonsEnv, LinearRing[] polygonRings, GeometryFactory geomFactory)
{
    double diam = polygonsEnv.getDiameter();
    Envelope envFrame = polygonsEnv.copy();
    envFrame.expandBy(FRAME_EXPAND_FACTOR * diam);
    Polygon frameOuter = (Polygon) geomFactory.toGeometry(envFrame);
    LinearRing shell = (LinearRing) frameOuter.getExteriorRing().copy();
    Polygon frame = geomFactory.createPolygon(shell, polygonRings);
    return frame;
}

/* private static */
LinearRing[] extractShellRings(const Geometry* polygons)
{
    LinearRing[] rings = new LinearRing[polygons.getNumGeometries()];
    for (int i = 0; i < polygons.getNumGeometries(); i++) {
      Polygon consPoly = (Polygon) polygons.getGeometryN(i);
      rings[i] = (LinearRing) consPoly.getExteriorRing().copy();
    }
    return rings;
}





} // namespace geos.algorithm.hull
} // namespace geos.algorithm
} // namespace geos


