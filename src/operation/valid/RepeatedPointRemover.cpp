/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Daniel Baston <dbaston@gmail.com
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/valid/RepeatedPointRemover.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/util.h>

namespace geos {
namespace operation {
namespace valid {

std::unique_ptr<geom::CoordinateArraySequence>
RepeatedPointRemover::removeRepeatedPoints(const geom::CoordinateSequence* seq) {
    using geom::Coordinate;

    if (seq->isEmpty()) {
        return detail::make_unique<geom::CoordinateArraySequence>(0u, seq->getDimension());
    }

    auto pts = detail::make_unique<std::vector<Coordinate>>();
    auto sz = seq->getSize();
    pts->reserve(sz); // assume not many points are repeated

    const Coordinate* prevPt = &(seq->getAt(0));
    pts->push_back(*prevPt) ;

    for (std::size_t i = 1; i < sz; i++) {
        const Coordinate* curPt = &(seq->getAt(i));
        if (*curPt != *prevPt) {
            pts->push_back(*curPt);
            prevPt = curPt;
        }
    }

    return detail::make_unique<geom::CoordinateArraySequence>(pts.release(), seq->getDimension());
}

std::unique_ptr<geom::CoordinateArraySequence>
RepeatedPointRemover::removeRepeatedAndInvalidPoints(const geom::CoordinateSequence* seq) {
    using geom::Coordinate;
    std::size_t start = 0;

    if (seq->isEmpty()) {
        return detail::make_unique<geom::CoordinateArraySequence>(0u, seq->getDimension());
    }

    auto pts = detail::make_unique<std::vector<Coordinate>>();
    auto sz = seq->getSize();
    pts->reserve(sz); // assume not many points are repeated

    // Find first valid poit
    while(!seq->getAt(start).isValid()) {
        start++;
    }

    const Coordinate* prevPt = &(seq->getAt(start));
    pts->push_back(*prevPt);

    for (std::size_t i = start+1; i < sz; i++) {
        const Coordinate* curPt = &(seq->getAt(i));
        if (*curPt != *prevPt && curPt->isValid()) {
            pts->push_back(*curPt);
            prevPt = curPt;
        }
    }

    return detail::make_unique<geom::CoordinateArraySequence>(pts.release(), seq->getDimension());
}


}
}
}
