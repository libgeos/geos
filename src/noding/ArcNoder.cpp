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

#include <geos/noding/ArcNoder.h>
#include <geos/noding/PathString.h>
#include <geos/noding/SegmentString.h>

namespace geos::noding {

ArcNoder::~ArcNoder() = default;

void
ArcNoder::computeNodes(const std::vector<SegmentString*>& segStrings)
{
    std::vector<PathString*> pathStrings(segStrings.size());
    for (size_t i = 0; i < segStrings.size(); ++i) {
        pathStrings[i] = segStrings[i];
    }

    computePathNodes(pathStrings);
}

std::vector<std::unique_ptr<SegmentString>>
        ArcNoder::getNodedSubstrings()
{
    auto pathStrings(getNodedPaths());
    std::vector<std::unique_ptr<SegmentString>> segStrings(pathStrings.size());
    for (size_t i = 0; i < pathStrings.size(); ++i) {
        // check inputs to verify they were actually linear, then set a flag that can be
        // checked in this method?
        segStrings[i].reset(detail::down_cast<SegmentString*>(pathStrings[i].release()));
    }

    return segStrings;
}

}