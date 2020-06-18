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
 **********************************************************************
 *
 * Last port: noding/snap/SnappingPointIndex.java
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/index/kdtree/KdTree.h>
#include <geos/index/kdtree/KdNode.h>

namespace geos {
namespace noding { // geos::noding
namespace snap { // geos::noding::snap

class GEOS_DLL SnappingPointIndex {

private:

    double snapTolerance;
    std::unique_ptr<index::kdtree::KdTree> snapPointIndex;


public:

    SnappingPointIndex(double p_snapTolerance);
    const geom::Coordinate& snap(const geom::Coordinate& p);

};

} // namespace geos::noding::snap
} // namespace geos::noding
} // namespace geos



