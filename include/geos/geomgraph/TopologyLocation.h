/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/TopologyLocation.java r428 (JTS-1.12+)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Location.h>
#include <geos/geom/Position.h>

#include <vector>
#include <array>
#include <string>
#include <cassert>
#include <cstdint>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

using geos::geom::Position;
using geos::geom::Location;

namespace geos {
namespace geomgraph { // geos.geomgraph

/** \brief
 * A TopologyLocation is the labelling of a
 * GraphComponent's topological relationship to a single Geometry.
 *
 * If the parent component is an area edge, each side and the edge itself
 * have a topological location.  These locations are named
 *
 *  - ON: on the edge
 *  - LEFT: left-hand side of the edge
 *  - RIGHT: right-hand side
 *
 * If the parent component is a line edge or node, there is a single
 * topological relationship attribute, ON.
 *
 * The possible values of a topological location are
 * {Location::NONE, Location::EXTERIOR, Location::BOUNDARY, Location::INTERIOR}
 *
 * The labelling is stored in an array location[j] where
 * where j has the values ON, LEFT, RIGHT
 */
class GEOS_DLL TopologyLocation {

public:

    friend std::ostream& operator<< (std::ostream&, const TopologyLocation&);

    TopologyLocation() = default;

    /** \brief
     * Constructs a TopologyLocation specifying how points on, to the
     * left of, and to the right of some GraphComponent relate to some
     * Geometry.
     *
     * Possible values for the
     * parameters are Location::NONE, Location::EXTERIOR, Location::BOUNDARY,
     * and Location::INTERIOR.
     *
     * @see Location
     */
    TopologyLocation(Location on, Location left, Location right)
        : locationSize(3)
    {
        location[Position::ON] = on;
        location[Position::LEFT] = left;
        location[Position::RIGHT] = right;
    }

    TopologyLocation(Location on)
        : locationSize(1)
    {
        location.fill(Location::NONE);
        location[Position::ON] = on;
    };

    TopologyLocation(const TopologyLocation& gl)
        : location(gl.location)
        , locationSize(gl.locationSize)
        {};

    TopologyLocation& operator= (const TopologyLocation& gl)
    {
        location = gl.location;
        locationSize = gl.locationSize;
        return *this;
    };

    Location get(std::size_t posIndex) const
    {
        // should be an assert() instead ?
        if(posIndex < locationSize) {
            return location[posIndex];
        }
        return Location::NONE;
    };

    /**
     * @return true if all locations are Location::NONE
     */
    bool isNull() const
    {
        for(std::size_t i = 0; i < locationSize; ++i) {
            if(location[i] != Location::NONE) {
                return false;
            }
        }
        return true;
    };

    /**
     * @return true if any locations is Location::NONE
     */
    bool isAnyNull() const
    {
        for(std::size_t i = 0; i < locationSize; ++i) {
            if(location[i] == Location::NONE) {
                return true;
            }
        }
        return false;
    };

    bool isEqualOnSide(const TopologyLocation& le, uint32_t locIndex) const
    {
        return location[locIndex] == le.location[locIndex];
    };

    bool isArea() const
    {
        return locationSize > 1;
    };

    bool isLine() const
    {
        return locationSize == 1;
    };

    void flip()
    {
        if(locationSize <= 1) {
            return;
        }
        std::swap(location[Position::LEFT], location[Position::RIGHT]);
    };

    void setAllLocations(Location locValue)
    {
        location.fill(locValue);
    };


    void setAllLocationsIfNull(Location locValue)
    {
        for(std::size_t i = 0; i < locationSize; ++i) {
            if(location[i] == Location::NONE) {
                location[i] = locValue;
            }
        }
    };

    void setLocation(std::size_t locIndex, Location locValue)
    {
        location[locIndex] = locValue;
    };

    void setLocation(Location locValue)
    {
        setLocation(Position::ON, locValue);
    };

    const std::array<Location, 3>& getLocations() const
    {
        return location;
    };

    void setLocations(Location on, Location left, Location right)
    {
        assert(locationSize >= 3);
        location[Position::ON] = on;
        location[Position::LEFT] = left;
        location[Position::RIGHT] = right;
    };

    bool allPositionsEqual(Location loc) const
    {
        for(std::size_t i = 0; i < locationSize; ++i) {
            if(location[i] != loc) {
                return false;
            }
        }
        return true;
    };

    /** \brief
     * merge updates only the UNDEF attributes of this object
     * with the attributes of another.
     */
    void merge(const TopologyLocation& gl);

    std::string toString() const;


private:

    std::array<geom::Location, 3> location;
    std::uint8_t locationSize;

};

std::ostream& operator<< (std::ostream&, const TopologyLocation&);

} // namespace geos.geomgraph
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif
