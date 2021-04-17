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
#include <geos/geom/CoordinateFilter.h>

#include <geos/util.h>

using geos::geom::Coordinate;
using geos::geom::CoordinateFilter;
using geos::geom::CoordinateArraySequence;

namespace geos {
namespace operation {
namespace valid {

class RepeatedPointFilter : public CoordinateFilter {
    public:

        RepeatedPointFilter() : m_prev(nullptr) {}

        void filter_ro(const Coordinate* curr) override final {

            // skip duplicate point
            if (m_prev != nullptr && curr->equals2D(*m_prev)) {
                return;
            }

            m_coords.push_back(*curr);
            m_prev = curr;
        }

        std::vector<Coordinate> getCoords() {
            return std::move(m_coords);
        }

    private:

        std::vector<Coordinate> m_coords;
        const Coordinate* m_prev;
};

std::unique_ptr<CoordinateArraySequence>
RepeatedPointRemover::removeRepeatedPoints(const geom::CoordinateSequence* seq) {

    if (seq->isEmpty()) {
        return detail::make_unique<CoordinateArraySequence>(0u, seq->getDimension());
    }

    RepeatedPointFilter filter;
    seq->apply_ro(&filter);
    return detail::make_unique<CoordinateArraySequence>(filter.getCoords());
}

class RepeatedInvalidPointFilter : public CoordinateFilter {
    public:

        RepeatedInvalidPointFilter() : m_prev(nullptr) {}

        void filter_ro(const Coordinate* curr) override final {

            bool invalid = ! curr->isValid();
            // skip initial invalids
            if (m_prev == nullptr && invalid)
                return;

            // skip duplicate/invalid points
            if (m_prev != nullptr &&
                (invalid || curr->equals2D(*m_prev)) ) {
                return;
            }

            m_coords.push_back(*curr);
            m_prev = curr;
        }

        std::vector<Coordinate> getCoords() {
            return std::move(m_coords);
        }

    private:

        std::vector<Coordinate> m_coords;
        const Coordinate* m_prev;
};




std::unique_ptr<geom::CoordinateArraySequence>
RepeatedPointRemover::removeRepeatedAndInvalidPoints(const geom::CoordinateSequence* seq) {

    if (seq->isEmpty()) {
        return detail::make_unique<CoordinateArraySequence>(0u, seq->getDimension());
    }

    RepeatedInvalidPointFilter filter;
    seq->apply_ro(&filter);
    return detail::make_unique<CoordinateArraySequence>(filter.getCoords());
}


}
}
}
