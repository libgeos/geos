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
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Envelope.h> // for unique_ptr
#include <geos/geom/Coordinate.h> // for composition
#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/snapround/HotPixel.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/io/WKTWriter.h>
#include <geos/index/kdtree/KdTree.h>
#include <geos/index/kdtree/KdNodeVisitor.h>
#include <geos/util.h>

#include <array>
#include <map>
#include <memory>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace algorithm {
class LineIntersector;
}
namespace index {
class ItemVisitor;
}
namespace noding {
namespace snapround {
class HotPixel;
}
}
}


namespace geos {
namespace noding { // geos::noding
namespace snapround { // geos::noding::snapround


class GEOS_DLL HotPixelIndex {

private:

    /* members */
    const geom::PrecisionModel* pm;
    double scaleFactor;
    std::unique_ptr<geos::index::kdtree::KdTree> index;
    std::deque<HotPixel> hotPixelQue;

    /* methods */
    template<typename CoordType>
    geom::CoordinateXYZM round(const CoordType& pt) {
        geom::CoordinateXYZM p2(pt);
        pm->makePrecise(p2);
        return p2;
    }

    HotPixel* find(const geom::Coordinate& pixelPt);

public:

    HotPixelIndex(const geom::PrecisionModel* p_pm);
    HotPixel* addRounded(const geom::CoordinateXYZM& pt);

    template<typename CoordType>
    HotPixel* add(const CoordType& p) {
        static_assert(std::is_base_of<geom::CoordinateXY, CoordType>(), "Only valid for Coordinate types");

        auto pRound = round(p);
        return addRounded(pRound);
    }

    void add(const geom::CoordinateSequence* pts);
    void add(const std::vector<geom::Coordinate>& pts);
    void addNodes(const geom::CoordinateSequence* pts);
    void addNodes(const std::vector<geom::Coordinate>& pts);

    /**
    * Visits all the hot pixels which may intersect a segment (p0-p1).
    * The visitor must determine whether each hot pixel actually intersects
    * the segment.
    */
    void query(const geom::CoordinateXY& p0, const geom::CoordinateXY& p1,
               index::kdtree::KdNodeVisitor& visitor);

};

} // namespace geos::noding::snapround
} // namespace geos::noding
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif


