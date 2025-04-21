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

#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/algorithm/locate/PointOnGeometryLocator.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Location.h>
#include <geos/export.h>

#include <array>

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng


/**
 * Manages the input geometries for an overlay operation.
 * The second geometry is allowed to be null,
 * to support for instance precision reduction.
 *
 * @author Martin Davis
 *
 */

class GEOS_DLL InputGeometry {
    using Geometry = geos::geom::Geometry;
    using Envelope = geos::geom::Envelope;
    using Coordinate = geos::geom::Coordinate;
    using Location = geos::geom::Location;
    using PointOnGeometryLocator = geos::algorithm::locate::PointOnGeometryLocator;

private:

    // Members
    std::array<const Geometry*, 2> geom;
    std::unique_ptr<PointOnGeometryLocator> ptLocatorA;
    std::unique_ptr<PointOnGeometryLocator> ptLocatorB;
    std::array<bool, 2> isCollapsed;



public:

    InputGeometry(const Geometry* geomA, const Geometry* geomB);

    bool isSingle() const;
    int getDimension(uint8_t index) const;
    uint8_t getCoordinateDimension(uint8_t index) const;
    const Geometry* getGeometry(uint8_t geomIndex) const;
    const Envelope* getEnvelope(uint8_t geomIndex) const;
    bool isEmpty(uint8_t geomIndex) const;
    bool isArea(uint8_t geomIndex) const;
    int getAreaIndex() const;
    bool isLine(uint8_t geomIndex) const;
    bool isAllPoints() const;
    bool hasPoints() const;

    /**
    * Tests if an input geometry has edges.
    * This indicates that topology needs to be computed for them.
    *
    * @param geomIndex
    * @return true if the input geometry has edges
    */
    bool hasEdges(uint8_t geomIndex) const;

    /**
    * Determines the location within an area geometry.
    * This allows disconnected edges to be fully
    * located.
    *
    * @param geomIndex the index of the geometry
    * @param pt the coordinate to locate
    * @return the location of the coordinate
    *
    * @see Location
    */
    Location locatePointInArea(uint8_t geomIndex, const Coordinate& pt);

    PointOnGeometryLocator* getLocator(uint8_t geomIndex);
    void setCollapsed(uint8_t geomIndex, bool isGeomCollapsed);


};


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos

