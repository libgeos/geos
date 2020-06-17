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
#include <geos/index/kdtree/KdTree.h>
#include <geos/index/ItemVisitor.h>
#include <geos/geom/CoordinateSequence.h>

#include <algorithm> // for std::min and std::max
#include <cassert>
#include <memory>

using namespace geos::algorithm;
using namespace geos::geom;
using geos::index::kdtree::KdTree;
using geos::index::ItemVisitor;

namespace geos {
namespace noding { // geos.noding
namespace snapround { // geos.noding.snapround

/*public*/
HotPixelIndex::HotPixelIndex(const PrecisionModel* p_pm)
    :
    pm(p_pm),
    scaleFactor(p_pm->getScale()),
    index(new KdTree())
{
}


/*public*/
const HotPixel*
HotPixelIndex::add(const Coordinate& p)
{
    Coordinate pRound = round(p);
    const HotPixel* hp = find(p);
    if (hp != nullptr)
        return hp;

    // Store the HotPixel in a std::deque to avoid individually
    // allocating a pile of HotPixels on the heap and to
    // get them freed automatically when the std::deque
    // goes away when this object is disposed.
    hotPixelQue.emplace_back(pRound, scaleFactor);

    // Pick up a pointer to the most recently added
    // HotPixel.
    hp = &(hotPixelQue.back());

    index->insert(hp->getCoordinate(), (void*)hp);
    return hp;

}

/*public*/
void
HotPixelIndex::add(const CoordinateSequence *pts)
{
    for (size_t i = 0, sz = pts->size(); i < sz; i++) {
        add(pts->getAt(i));
    }
}

/*public*/
void
HotPixelIndex::add(const std::vector<geom::Coordinate>& pts)
{
    for (auto pt: pts) {
        add(pt);
    }
}

/*private*/
const HotPixel*
HotPixelIndex::find(const geom::Coordinate& pixelPt)
{
    index::kdtree::KdNode *kdNode = index->query(pixelPt);
    if (kdNode == nullptr) {
        return nullptr;
    }
    return (const HotPixel*)(kdNode->getData());
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
HotPixelIndex::query(const Coordinate& p0, const Coordinate& p1, index::kdtree::KdNodeVisitor& visitor)
{
    Envelope queryEnv(p0, p1);
    queryEnv.expandBy(1.0 / scaleFactor);
    index->query(queryEnv, visitor);
}


} // namespace geos.noding.snapround
} // namespace geos.noding
} // namespace geos



