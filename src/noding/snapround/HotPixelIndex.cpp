/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 *********************************************************************/

#include <geos/noding/snapround/HotPixelIndex.h>

#include <algorithm> // for std::min and std::max
#include <cassert>
#include <memory>

using namespace geos::algorithm;
using namespace geos::geom;
using namespace geos::index::strtree;

namespace geos {
namespace noding { // geos.noding
namespace snapround { // geos.noding.snapround

/*public*/
HotPixelIndex::HotPixelIndex(const PrecisionModel* p_pm)
    :
    pm(p_pm),
    li(p_pm),
    scaleFactor(p_pm->getScale()),
    index(new STRtree())
{
}

/*public*/
const HotPixel*
HotPixelIndex::add(const Coordinate& pt)
{
    Coordinate ptRound = round(pt);

    /* Is the HotPixel already in the map? */
    auto itSearch = hotPixelMap.find(ptRound);
    if (itSearch != hotPixelMap.end()) {
        return &(itSearch->second);
    }

    /* Carefully instantiate the key and value into the map */
    auto rsltEmplace = hotPixelMap.emplace(std::piecewise_construct,
                    std::forward_as_tuple(ptRound),
                    std::forward_as_tuple(ptRound, scaleFactor));

    /* Read inserted HotPixel back. */
    /* std::map.emplace() returns std::pair<iterator, bool> */
    auto itEmplace = rsltEmplace.first;
    auto inserted = rsltEmplace.second;
    const HotPixel *ptrHp = &(itEmplace->second);

    if (inserted) {
        const Envelope& hpEnv = ptrHp->getSafeEnvelope();
        index->insert(&hpEnv, (void*)ptrHp);
    }

    /* And return that pointer to the caller */
    return ptrHp;
}

/*public*/
void
HotPixelIndex::add(const std::vector<Coordinate> pts)
{
    for (auto pt: pts) {
        add(pt);
    }
}

/*private*/
Coordinate
HotPixelIndex::round(const Coordinate& pt)
{
    Coordinate p2 = pt;
    pm->makePrecise(p2);
    return p2;
}

/*public*/
void
HotPixelIndex::query(const Coordinate& p0, const Coordinate& p1, index::ItemVisitor& visitor)
{
    Envelope queryEnv(p0, p1);
    index->query(&queryEnv, visitor);
}


} // namespace geos.noding.snapround
} // namespace geos.noding
} // namespace geos



