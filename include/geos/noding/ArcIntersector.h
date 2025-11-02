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
#include <geos/noding/SegmentString.h>

namespace geos::noding {

class GEOS_DLL ArcIntersector {
public:
    virtual void processIntersections(ArcString& e0, std::size_t arcIndex0,
                                      ArcString& e1, std::size_t arcIndex1) = 0;
    virtual void processIntersections(SegmentString& e0, std::size_t segIndex0,
                                      SegmentString& e1, std::size_t segIndex1) = 0;
    virtual void processIntersections(ArcString& e0, std::size_t arcIndex0,
                                      SegmentString& e1, std::size_t segIndex1) = 0;

    virtual void processIntersections(SegmentString& e0, std::size_t segIndex0,
                                      ArcString& e1, std::size_t arcIndex1) {
        processIntersections(e1, arcIndex1, e0, segIndex0);
    }

    virtual bool isDone() const {
        return false;
    }

    virtual ~ArcIntersector() = default;
};

}