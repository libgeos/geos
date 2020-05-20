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
HotPixelIndex::HotPixelIndex(const geom::PrecisionModel* p_pm)
    :
    pm(p_pm),
    li(p_pm),
    scaleFactor(p_pm->getScale()),
    index(new STRtree())
{
}

/*public*/
HotPixelIndex::~HotPixelIndex()
{
    // Free all the HotPixel* in the map
    auto it = hotPixelMap.begin();
    while(it != hotPixelMap.end())
    {
        delete it->second;
        it++;
    }
}

/*public*/
const HotPixel*
HotPixelIndex::add(const Coordinate& p)
{
    Coordinate pRound = round(p);

    /* Is the HotPixel already in the map? */
    auto hpSearch = hotPixelMap.find(pRound);
    if (hpSearch != hotPixelMap.end()) {
        return hpSearch->second;
    }

    HotPixel *hp = new HotPixel(pRound, scaleFactor, li);
    const Envelope& hpEnv = hp->getSafeEnvelope();
    hotPixelMap.insert(std::make_pair(pRound, hp));
    index->insert(&hpEnv, hp);
    return hp;
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
std::vector<const HotPixel*>
HotPixelIndex::query(const Coordinate& p0, const Coordinate& p1)
{
    Envelope queryEnv(p0, p1);
    std::vector<void*> voidHotPixels;
    std::vector<const HotPixel*> hotPixels;
    index->query(&queryEnv, voidHotPixels);
    // TODO: replace with visitor or some magic cast to avoid this
    // stupid casting step?
    hotPixels.reserve(voidHotPixels.size());
    for (auto voidhp: voidHotPixels) {
        hotPixels.push_back(static_cast<const HotPixel*>(voidhp));
    }
    return hotPixels;
}



} // namespace geos.noding.snapround
} // namespace geos.noding
} // namespace geos



