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
#include <geos/geom/Coordinate.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Location.h>
#include <geos/operation/relateng/RelatePointLocator.h>
#include <geos/operation/relateng/RelateSegmentString.h>
#include <geos/export.h>

#include <string>
#include <sstream>


// Forward declarations
namespace geos {
namespace geom {
    class CoordinateSequence;
    class Envelope;
    class Geometry;
    class LinearRing;
    class LineString;
    class MultiPolygon;
    class Point;
}
namespace noding {
    class SegmentString;
}
}


namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng

using namespace geos::geom;
using geos::algorithm::BoundaryNodeRule;
using geos::noding::SegmentString;


class GEOS_DLL RelateGeometry {

private:

    // Members

    const Geometry* geom;
    bool m_isPrepared = false;
    const Envelope* geomEnv;
    const BoundaryNodeRule& boundaryNodeRule;
    int geomDim = Dimension::False;
    bool isLineZeroLen = false;
    bool isGeomEmpty = false;

    Coordinate::ConstXYSet uniquePoints;
    std::unique_ptr<RelatePointLocator> locator;
    int elementId = 0;
    bool hasPoints = false;
    bool hasLines = false;
    bool hasAreas = false;

    /*
     * Memory contexts for lower level allocations
     */
    std::vector<std::unique_ptr<const RelateSegmentString>> segStringTempStore;
    std::vector<std::unique_ptr<const RelateSegmentString>> segStringPermStore;
    std::vector<std::unique_ptr<CoordinateSequence>> csStore;


    // Methods

    void analyzeDimensions();

    /**
    * Tests if all geometry linear elements are zero-length.
    * For efficiency the test avoids computing actual length.
    *
    * @param geom
    * @return
    */
    static bool isZeroLength(const Geometry* geom);

    static bool isZeroLength(const LineString* line);

    RelatePointLocator* getLocator();

    Coordinate::ConstXYSet createUniquePoints();

    void extractSegmentStringsFromAtomic(bool isA,
        const Geometry* geom, const MultiPolygon* parentPolygonal,
        const Envelope* env,
        std::vector<const SegmentString*>& segStrings,
        std::vector<std::unique_ptr<const RelateSegmentString>>& segStore);

    void extractRingToSegmentString(bool isA,
        const LinearRing* ring, int ringId, const Envelope* env,
        const Geometry* parentPoly,
        std::vector<const SegmentString*>& segStrings,
        std::vector<std::unique_ptr<const RelateSegmentString>>& segStore);

    void extractSegmentStrings(bool isA,
        const Envelope* env, const Geometry* geom,
        std::vector<const SegmentString*>& segStrings,
        std::vector<std::unique_ptr<const RelateSegmentString>>& segStore);

    const CoordinateSequence* orientAndRemoveRepeated(
        const CoordinateSequence* cs, bool orientCW);

    const CoordinateSequence* removeRepeated(
        const CoordinateSequence* cs);

public:

    static constexpr bool GEOM_A = true;
    static constexpr bool GEOM_B = false;

    RelateGeometry(const Geometry* input)
        : RelateGeometry(input, false, BoundaryNodeRule::getBoundaryRuleMod2())
        {};

    RelateGeometry(const Geometry* input, const BoundaryNodeRule& bnRule)
        : RelateGeometry(input, false, bnRule)
        {};

    RelateGeometry(const Geometry* input, bool p_isPrepared, const BoundaryNodeRule& bnRule);

    static std::string name(bool isA);

    const Geometry* getGeometry() const;

    bool isPrepared() const;

    const Envelope* getEnvelope() const;

    int getDimension() const;

    bool hasDimension(int dim) const;

    /**
    * Gets the actual non-empty dimension of the geometry.
    * Zero-length LineStrings are treated as Points.
    *
    * @return the real (non-empty) dimension
    */
    int getDimensionReal() const;

    bool hasEdges() const;

    bool isNodeInArea(const CoordinateXY* nodePt, const Geometry* parentPolygonal);

    Location locateLineEnd(const CoordinateXY* p) ;

    /**
     * Locates a vertex of a polygon.
     * A vertex of a Polygon or MultiPolygon is on
     * the {@link Location#BOUNDARY}.
     * But a vertex of an overlapped polygon in a GeometryCollection
     * may be in the {@link Location#INTERIOR}.
     *
     * @param pt the polygon vertex
     * @return the location of the vertex
     */
    Location locateAreaVertex(const CoordinateXY* pt);

    Location locateNode(const CoordinateXY* pt, const Geometry* parentPolygonal);

    int locateWithDim(const CoordinateXY* pt);

    bool isPointsOrPolygons() const;

    /**
     * Tests whether the geometry has polygonal topology.
     * This is not the case if it is a GeometryCollection
     * containing more than one polygon (since they may overlap
     * or be adjacent).
     * The significance is that polygonal topology allows more assumptions
     * about the location of boundary vertices.
     *
     * @return true if the geometry has polygonal topology
     */
    bool isPolygonal() const;

    bool isEmpty() const;

    bool hasBoundary();

    Coordinate::ConstXYSet& getUniquePoints();

    std::vector<const Point*> getEffectivePoints();

    /**
     * Extract RSegmentStrings from the geometry which
     * intersect a given envelope.
     * If the envelope is null all edges are extracted.
     * @param geomA
     *
     * @param env the envelope to extract around (may be null)
     * @return a list of SegmentStrings
     */
    std::vector<const SegmentString*> extractSegmentStrings(bool isA, const Envelope* env);

    std::string toString() const;

    friend std::ostream& operator<<(std::ostream& os, const RelateGeometry& rg);

    /**
     * Disable copy construction and assignment. Needed to make this
     * class compile under MSVC. (See https://stackoverflow.com/q/29565299)
     * Classes with members that are vector<> of unique_ptr<> need this.
     */
    RelateGeometry(const RelateGeometry&) = delete;
    RelateGeometry& operator=(const RelateGeometry&) = delete;

};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

