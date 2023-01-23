/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

// #include <geos/algorithm/LineIntersector.h>
// #include <geos/geom/Coordinate.h>
//#include <geos/geom/CoordinateSequence.h>

#include <map>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
}
namespace noding {
class NodedSegmentString;
}
}

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::noding::NodedSegmentString;
using geos::algorithm::LineIntersector;


namespace geos {
namespace triangulate {
namespace polygon {


/**
 * Adds node vertices to the rings of a polygon
 * where holes touch the shell or each other.
 * The structure of the polygon is preserved.
 *
 * This does not fix invalid polygon topology
 * (such as self-touching or crossing rings).
 * Invalid input remains invalid after noding,
 * and does not trigger an error.
 */
class GEOS_DLL PolygonNoder {

public:

    PolygonNoder(
        std::unique_ptr<CoordinateSequence>& shellRing,
        std::vector<std::unique_ptr<CoordinateSequence>>& holeRings);

    void node();
    bool isShellNoded();
    bool isHoleNoded(std::size_t i);
    std::unique_ptr<CoordinateSequence> getNodedShell();
    std::unique_ptr<CoordinateSequence> getNodedHole(std::size_t i);
    std::vector<bool>& getHolesTouching();



private:

    // Members
    std::vector<bool> isHoleTouching;
    std::map<NodedSegmentString*, std::size_t> nodedRingIndexes;
    std::vector<std::unique_ptr<NodedSegmentString>> nodedRings;

    // Classes
    class NodeAdder;
    friend class PolygonNoder::NodeAdder;

    // Methods
    NodedSegmentString*
        createNodedSegString(std::unique_ptr<CoordinateSequence>& ringPts, std::size_t i);

    void createNodedSegmentStrings(
        std::unique_ptr<CoordinateSequence>& shellRing,
        std::vector<std::unique_ptr<CoordinateSequence>>& holeRings);

    /* Turn off copy constructors for MSVC */
    PolygonNoder(const PolygonNoder&) = delete;
    PolygonNoder& operator=(const PolygonNoder&) = delete;

};



} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos
