/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/util.h>

namespace geos {
namespace geom {

class GEOS_DLL DefaultCoordinateSequenceFactory : public CoordinateSequenceFactory {
public:

    std::unique_ptr<CoordinateSequence> create() const final override {
        return detail::make_unique<CoordinateSequence>();
    }

    std::unique_ptr<CoordinateSequence> create(std::vector<Coordinate> *coords, std::size_t dims = 0) const final override {
        auto ret = detail::make_unique<CoordinateSequence>(std::move(*coords), dims);
        delete coords;
        return ret;
    }

    std::unique_ptr <CoordinateSequence> create(std::vector <Coordinate> &&coords, std::size_t dims = 0) const final override {
        return detail::make_unique<CoordinateSequence>(std::move(coords), dims);
    }

    std::unique_ptr <CoordinateSequence> create(std::vector <CoordinateXY> &&coords, std::size_t dims = 0) const final override {
        return detail::make_unique<CoordinateSequence>(std::move(coords), dims);
    }

    std::unique_ptr <CoordinateSequence> create(std::size_t size, std::size_t dims = 0) const final override {
        return detail::make_unique<CoordinateSequence>(size, dims);
    }

    std::unique_ptr <CoordinateSequence> create(const CoordinateSequence &coordSeq) const final override {
        auto cs = create(coordSeq.size(), coordSeq.getDimension());
        for (std::size_t i = 0; i < cs->size(); i++) {
            cs->setAt(coordSeq[i], i);
        }
        return cs;
    }

    static const CoordinateSequenceFactory *instance();
};

}
}

