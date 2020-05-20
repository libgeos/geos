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

#ifndef GEOS_NODING_SNAPROUND_HOTPIXELINDEX_H
#define GEOS_NODING_SNAPROUND_HOTPIXELINDEX_H

#include <geos/export.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h> // for composition
#include <geos/geom/Envelope.h> // for unique_ptr
#include <geos/geom/PrecisionModel.h>
#include <geos/index/strtree/STRtree.h>
#include <geos/io/WKTWriter.h>
#include <geos/noding/snapround/HotPixel.h>
#include <geos/util/IllegalArgumentException.h>

#include <array>
#include <map>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif


namespace geos {
namespace noding { // geos::noding
namespace snapround { // geos::noding::snapround


class GEOS_DLL HotPixelIndex {

private:

    /* members */
    const geom::PrecisionModel* pm;
    algorithm::LineIntersector li;
    double scaleFactor;
    std::unique_ptr<geos::index::strtree::STRtree> index;
    std::map<geom::Coordinate, HotPixel*> hotPixelMap;

    /* methods */
    geom::Coordinate round(const geom::Coordinate& c);

public:

    HotPixelIndex(const geom::PrecisionModel* p_pm);
    ~HotPixelIndex();
    const HotPixel* add(const geom::Coordinate& p);
    std::vector<const HotPixel*> query(const geom::Coordinate& p0, const geom::Coordinate& p1);

};

} // namespace geos::noding::snapround
} // namespace geos::noding
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif // GEOS_NODING_SNAPROUND_HOTPIXELINDEX_H
