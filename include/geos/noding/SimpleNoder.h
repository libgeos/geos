/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/SimpleNoder.java rev. 1.7 (JTS-1.9)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <vector>

#include <geos/noding/ArcNoder.h>

// Forward declarations
namespace geos {
namespace noding {
class PathString;
}
}

namespace geos {
namespace noding { // geos.noding

/** \brief
 * Nodes a set of {@link SegmentString}s and/or {@link ArcString}s by
 * performing a brute-force comparison of every segment to every other one.
 *
 * This has n^2 performance, so is too slow for use on large numbers
 * of segments.
 *
 * @version 1.7
 */
class GEOS_DLL SimpleNoder: public ArcNoder {
private:
    std::vector<PathString*> nodedSegStrings;
    void computeIntersects(PathString& e0, PathString& e1);

public:

    SimpleNoder(std::unique_ptr<ArcIntersector>(nSegInt)) : ArcNoder(std::move(nSegInt)) {}

    void computePathNodes(const std::vector<PathString*>& inputSegmentStrings) override;

    std::vector<std::unique_ptr<PathString>> getNodedPaths() override;

private:
    std::vector<PathString*> m_pathStrings;
};

} // namespace geos.noding
} // namespace geos

