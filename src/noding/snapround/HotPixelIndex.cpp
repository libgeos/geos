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
    auto hpSearch = hotPixelMap.find(ptRound);
    if (hpSearch != hotPixelMap.end()) {
        return &(hpSearch->second);
    }

    /* Carefully instantiate the key and value into the map */
    hotPixelMap.emplace(std::piecewise_construct,
                        std::forward_as_tuple(ptRound),
                        std::forward_as_tuple(ptRound, scaleFactor, li));

    /* So we can pull a pointer back off the map */
    const HotPixel* ptrHp;
    auto hpSearch2 = hotPixelMap.find(ptRound);
    if (hpSearch2 != hotPixelMap.end()) {
        ptrHp = &(hpSearch2->second);
    }

    /* And use that pointer to add to the STRtree */
    const Envelope& hpEnv = ptrHp->getSafeEnvelope();
    index->insert(&hpEnv, (void*)ptrHp);

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



