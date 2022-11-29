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
 * Last port: geomgraph/Depth.java rev. 1.4 (JTS-1.10)
 *
 **********************************************************************/


#pragma once

#include <geos/export.h>
#include <geos/geom/Location.h>
#include <geos/geom/Position.h>
#include <string>
#include <cstdint>

// Forward declarations
namespace geos {
namespace geomgraph {
class Label;
}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

/// \brief A Depth object records the topological depth of the sides of an Edge
/// for up to two Geometries.
class GEOS_DLL Depth {

private:

    static constexpr int NULL_VALUE = -1; // Replaces NULL

    int depth[2][3];


public:

    static int depthAtLocation(geom::Location location)
    {
        if(location == geom::Location::EXTERIOR) {
            return 0;
        }
        if(location == geom::Location::INTERIOR) {
            return 1;
        }
        return NULL_VALUE;
    };


    Depth()
    {
        // initialize depth array to a sentinel value
        for(std::size_t i = 0; i < 2; i++) {
            for(std::size_t j = 0; j < 3; j++) {
                depth[i][j] = NULL_VALUE;
            }
        }
    };

    virtual ~Depth() = default; // FIXME: shouldn't be virtual!

    int getDepth(int geomIndex, int posIndex) const
    {
        return depth[geomIndex][posIndex];
    };

    void setDepth(int geomIndex, int posIndex, int depthValue)
    {
        depth[geomIndex][posIndex] = depthValue;
    };

    geom::Location getLocation(int geomIndex, int posIndex) const
    {
        if(depth[geomIndex][posIndex] <= 0) {
            return geom::Location::EXTERIOR;
        }
        return geom::Location::INTERIOR;
    };

    void add(int geomIndex, int posIndex, geom::Location location)
    {
        if(location == geom::Location::INTERIOR) {
            depth[geomIndex][posIndex]++;
        }
    };

    /**
     * A Depth object is null (has never been initialized) if all depths are null.
     */
    bool isNull() const
    {
        for(std::size_t i = 0; i < 2; i++) {
            for(std::size_t j = 0; j < 3; j++) {
                if(depth[i][j] != NULL_VALUE) {
                    return false;
                }
            }
        }
        return true;
    };

    bool isNull(uint8_t geomIndex) const
    {
        return depth[geomIndex][1] == NULL_VALUE;
    };

    bool isNull(uint8_t geomIndex, uint8_t posIndex) const
    {
        return depth[geomIndex][posIndex] == NULL_VALUE;
    };

    int getDelta(int geomIndex) const
    {
        return depth[geomIndex][geom::Position::RIGHT] - depth[geomIndex][geom::Position::LEFT];
    };

    void normalize();

    void add(const Label& lbl);

    std::string toString() const;


};


} // namespace geos.geomgraph
} // namespace geos

