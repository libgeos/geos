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
    NodableArcString(std::vector<geom::CircularArc> arcs, const std::shared_ptr<const geom::CoordinateSequence>& coords, bool constructZ, bool constructM, void* context);

    std::unique_ptr<ArcString> clone() const;

    using NodablePath::addIntersection;

    void addIntersection(const geom::CoordinateXYZM& intPt, size_t segmentIndex) override {
        m_adds[segmentIndex].push_back(intPt);
    }

    std::unique_ptr<ArcString> getNoded();

private:
    std::map<size_t, std::vector<geom::CoordinateXYZM>> m_adds;
    bool m_constructZ = false;
    bool m_constructM = false;
};

}