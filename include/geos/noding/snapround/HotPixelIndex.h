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
#include <geos/index/strtree/STRtree.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/snapround/HotPixel.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/index/ItemVisitor.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/io/WKTWriter.h>

#include <array>
#include <map>
#include <memory>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
// namespace geom {
// class Envelope;
// }
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
    algorithm::LineIntersector li;
    double scaleFactor;
    /* TODO, check this to a KDtree */
    std::unique_ptr<geos::index::strtree::STRtree> index;
    std::map<geom::Coordinate, HotPixel> hotPixelMap;

    /* methods */
    geom::Coordinate round(const geom::Coordinate& c);

public:

    HotPixelIndex(const geom::PrecisionModel* p_pm);
    const HotPixel* add(const geom::Coordinate& pt);
    void add(const std::vector<geom::Coordinate> pts);
    void query(const geom::Coordinate& p0, const geom::Coordinate& p1, index::ItemVisitor& visitor);

};

} // namespace geos::noding::snapround
} // namespace geos::noding
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif


