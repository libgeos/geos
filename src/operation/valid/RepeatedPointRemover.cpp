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
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/util/GeometryEditor.h>
#include <geos/geom/util/CoordinateOperation.h>

#include <geos/util.h>

using geos::geom::Coordinate;
using geos::geom::CoordinateArraySequence;
using geos::geom::CoordinateFilter;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateSequenceFactory;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::util::CoordinateOperation;
using geos::geom::util::GeometryEditor;

namespace geos {
namespace operation {
namespace valid {

class RepeatedPointFilter : public CoordinateFilter {
    public:

        RepeatedPointFilter()
            : m_prev(nullptr)
            , sqTolerance(0.0) {}

        RepeatedPointFilter(double tolerance)
            : m_prev(nullptr)
            , sqTolerance(tolerance*tolerance) {}

        void filter_ro(const Coordinate* curr) override final {

            // skip duplicate point or too-close poinnt
            if (m_prev != nullptr && (
                curr->equals2D(*m_prev) ||
                curr->distanceSquared(*m_prev) <= sqTolerance))
            {
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
        double sqTolerance;
};


/* public static */
std::unique_ptr<CoordinateArraySequence>
RepeatedPointRemover::removeRepeatedPoints(const CoordinateSequence* seq, double tolerance) {

    if (seq->isEmpty()) {
        return detail::make_unique<CoordinateArraySequence>(0u, seq->getDimension());
    }

    RepeatedPointFilter filter(tolerance);
    seq->apply_ro(&filter);
    return detail::make_unique<CoordinateArraySequence>(filter.getCoords());
}


class RepeatedInvalidPointFilter : public CoordinateFilter {
    public:

        RepeatedInvalidPointFilter()
            : m_prev(nullptr)
            , sqTolerance(0.0) {}

        RepeatedInvalidPointFilter(double tolerance)
            : m_prev(nullptr)
            , sqTolerance(tolerance*tolerance) {}

        void filter_ro(const Coordinate* curr) override final {

            bool invalid = ! curr->isValid();
            // skip initial invalids
            if (m_prev == nullptr && invalid)
                return;

            // skip duplicate/invalid points
            if (m_prev != nullptr && (
                invalid ||
                curr->equals2D(*m_prev) ||
                curr->distanceSquared(*m_prev) <= sqTolerance))
            {
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
        double sqTolerance;
};


/* public static */
std::unique_ptr<CoordinateArraySequence>
RepeatedPointRemover::removeRepeatedAndInvalidPoints(const CoordinateSequence* seq, double tolerance) {

    if (seq->isEmpty()) {
        return detail::make_unique<CoordinateArraySequence>(0u, seq->getDimension());
    }

    RepeatedInvalidPointFilter filter(tolerance);
    seq->apply_ro(&filter);
    return detail::make_unique<CoordinateArraySequence>(filter.getCoords());
}


class RepeatedPointCoordinateOperation : public CoordinateOperation
{

    // using CoordinateOperation::edit;

private:

    double tolerance;

public:

    RepeatedPointCoordinateOperation()
        : tolerance(0.0)
        {};

    RepeatedPointCoordinateOperation(double p_tolerance)
        : tolerance(p_tolerance)
        {};


    std::unique_ptr<CoordinateSequence> edit(
        const CoordinateSequence* coordinates,
        const Geometry* geom) override
    {
        if (!coordinates) return nullptr;

        if(geom->getGeometryTypeId() == geom::GEOS_POINT) {
            return coordinates->clone();
        }

        unsigned int minLength = 0;
        if(geom->getGeometryTypeId() == geom::GEOS_LINESTRING) {
            minLength = 2;
        }
        if(geom->getGeometryTypeId() == geom::GEOS_LINEARRING) {
            minLength = 4;
        }

        // No way to filter short sequences.
        // Zero coordinates => empty
        // One coordinate => point
        // Two coordinates => removing the end point would just
        // create a degenerate line
        if(coordinates->size() <= minLength)
            return coordinates->clone();

        // Create new vector of coordinates filtered to the
        // the tolerance.
        RepeatedInvalidPointFilter filter(tolerance);
        coordinates->apply_ro(&filter);
        std::vector<Coordinate> filtCoords = filter.getCoords();

        if (filtCoords.size() == 0) return nullptr;

        // End points for comparison and sequence repair
        const Coordinate& origEndCoord = coordinates->back();
        const Coordinate& filtEndCoord = filtCoords.back();

        // Fluff up overly small filtered outputs
        if(filtCoords.size() < minLength) {
            filtCoords.push_back(origEndCoord);
        }

        // We stripped the last point, let's put it back on
        if (!origEndCoord.equals2D(filtEndCoord)) {
            // If the end of the filtered coordinates is within
            // tolerance of the original end, we drop the last filtered
            // coordinate so the output still follows the tolerance rule
            if(origEndCoord.distanceSquared(filtEndCoord) <= tolerance*tolerance) {
                filtCoords.pop_back();
            }
            // Put the original end coordinate back on
            filtCoords.push_back(origEndCoord);
        }

        auto fact = geom->getFactory();
        auto csfact = fact->getCoordinateSequenceFactory();
        return csfact->create(std::move(filtCoords));
    };
}; // RepeatedPointCoordinateOperation


/* public static */
std::unique_ptr<Geometry>
RepeatedPointRemover::removeRepeatedPoints(
    const Geometry* inputGeom,
    double tolerance)
{
    GeometryEditor geomEdit;
    RepeatedPointCoordinateOperation rpco(tolerance);
    return geomEdit.edit(inputGeom, &rpco);
}


} // valid
} // operation
} // geos
