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

#include <geos/geom/Location.h>
#include <geos/export.h>

#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace operation {
namespace relateng {
class RelateNode;
}
}
namespace geom {
    class CoordinateXY;
    class Geometry;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng


class GEOS_DLL RelateEdge {
    using CoordinateXY = geos::geom::CoordinateXY;
    using Geometry = geos::geom::Geometry;
    using Location = geos::geom::Location;

private:

    /**
    * Indicates that the location is currently unknown
    */
    static constexpr Location LOC_UNKNOWN = Location::NONE;

    // Members
    const RelateNode* node;
    const CoordinateXY* dirPt;

    int aDim = DIM_UNKNOWN;
    Location aLocLeft = LOC_UNKNOWN;
    Location aLocRight = LOC_UNKNOWN;
    Location aLocLine = LOC_UNKNOWN;

    int bDim = DIM_UNKNOWN;
    Location bLocLeft = LOC_UNKNOWN;
    Location bLocRight = LOC_UNKNOWN;
    Location bLocLine = LOC_UNKNOWN;


public:

    // Constants
    static constexpr bool IS_FORWARD = true;
    static constexpr bool IS_REVERSE = false;
    static constexpr int DIM_UNKNOWN = -1;

    // Constructors
    RelateEdge(
        const RelateNode* node, const CoordinateXY* pt,
        bool isA, bool isForward);

    RelateEdge(
        const RelateNode* node, const CoordinateXY* pt,
        bool isA);

    RelateEdge(
        const RelateNode* node, const CoordinateXY* pt,
        bool isA, Location locLeft, Location locRight, Location locLine);

    // Methods
    static RelateEdge* create(
        const RelateNode* node,
        const CoordinateXY* dirPt,
        bool isA, int dim, bool isForward);

    static std::size_t findKnownEdgeIndex(
        std::vector<std::unique_ptr<RelateEdge>>& edges,
        bool isA);

    static void setAreaInterior(
        std::vector<std::unique_ptr<RelateEdge>>& edges,
        bool isA);

    bool isInterior(bool isA, int position) const;

    Location location(bool isA, int position) const;

    int compareToEdge(const CoordinateXY* edgeDirPt) const;

    void setDimLocations(bool isA, int dim, Location loc);

    void setAreaInterior(bool isA);

    void setLocation(bool isA, int pos, Location loc);

    void setAllLocations(bool isA, Location loc);

    void setUnknownLocations(bool isA, Location loc);

    void merge(bool isA, int dim, bool isForward);

    std::string toString() const;

    friend std::ostream& operator<<(std::ostream& os, const RelateEdge& re);


private:

    // Methods
    void mergeSideLocation(bool isA, int pos, Location loc);

    /**
    * Area edges override Line edges.
    * Merging edges of same dimension is a no-op for
    * the dimension and on location.
    * But merging an area edge into a line edge
    * sets the dimension to A and the location to BOUNDARY.
    *
    * @param isA
    * @param locEdge
    */
    void mergeDimEdgeLoc(bool isA, Location locEdge);

    void setDimension(bool isA, int dimension);

    void setLeft(bool isA, Location loc);

    void setRight(bool isA, Location loc);

    void setOn(bool isA, Location loc);

    int dimension(bool isA) const;

    bool isKnown(bool isA) const;

    bool isKnown(bool isA, int pos) const;

    void setLocations(bool isA, Location locLeft, Location locRight, Location locLine);

    void setLocationsLine(bool isA);

    void setLocationsArea(bool isA, bool isForward);

    std::string labelString() const;

    std::string locationString(bool isA) const;


};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

