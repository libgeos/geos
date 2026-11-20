/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/noding/ArcString.h>
#include <geos/noding/NodablePath.h>

namespace geos::noding {

class GEOS_DLL NodableArcString : public ArcString, public NodablePath {

public:
    /** Construct a NodableArcString
     *
     * @param arcs a vector of CircularArcs
     * @param coords arc coordinates
     * @param constructZ whether new vertices should be constructed with a Z dimension
     * @param constructM whether new vertices should be constructed with an M dimension
     * @param controlPointTolerance whether to re-use an arc's supplied control point or
     *                              compute a new one at the central angle. A new point
     *                              will be used if it is more than radius * tol from the
     *                              original control point.
     * @param context data object
     */
    NodableArcString(std::vector<geom::CircularArc> arcs,
                     const std::shared_ptr<const geom::CoordinateSequence>& coords,
                     bool constructZ,
                     bool constructM,
                     double controlPointTolerance,
                     const void* context);

    using NodablePath::addIntersection;

    void addIntersection(const geom::CoordinateXYZM& intPt, size_t segmentIndex) override {
        m_adds[segmentIndex].push_back(intPt);
    }

    void getNoded(std::vector<std::unique_ptr<ArcString>>& splitArcs);

private:
    std::map<size_t, std::vector<geom::CoordinateXYZM>> m_adds;
    const double m_controlPointTolerance = 0;
    const bool m_constructZ = false;
    const bool m_constructM = false;
};

}