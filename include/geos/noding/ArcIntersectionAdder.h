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
#include <geos/algorithm/CircularArcIntersector.h>
#include <geos/noding/ArcIntersector.h>

namespace geos::noding {

class GEOS_DLL ArcIntersectionAdder : public ArcIntersector {

public:
    void processIntersections(ArcString& e0, std::size_t segIndex0, ArcString& e1, std::size_t segIndex1) override;

    void processIntersections(ArcString& e0, std::size_t segIndex0, SegmentString& e1, std::size_t segIndex1) override;

    void processIntersections(SegmentString& e0, std::size_t segIndex0, SegmentString& e1, std::size_t segIndex1) override;

private:
    algorithm::CircularArcIntersector m_intersector;
};

}