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
#include <geos/noding/ArcIntersector.h>
#include <geos/noding/Noder.h>

namespace geos::noding {
class PathString;
}

namespace geos::noding {

class GEOS_DLL ArcNoder : public Noder {

public:
    ArcNoder() = default;

    explicit ArcNoder(std::unique_ptr<ArcIntersector> intersector) :
        m_intersector(std::move(intersector)) {}

    ~ArcNoder() override;

    void setArcIntersector(std::unique_ptr<ArcIntersector> arcIntersector) {
        m_intersector = std::move(arcIntersector);
    }

    void computeNodes(const std::vector<SegmentString*>& segStrings) override;

    std::vector<std::unique_ptr<SegmentString>> getNodedSubstrings() override;

    virtual void computePathNodes(const std::vector<PathString*>& inputPaths) = 0;

    virtual std::vector<std::unique_ptr<PathString>> getNodedPaths() = 0;

protected:
    std::unique_ptr<ArcIntersector> m_intersector;
};

}