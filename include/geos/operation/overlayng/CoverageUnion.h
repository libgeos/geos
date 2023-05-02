/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
}
}

using geos::geom::Geometry;

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/**
 * Unions a valid coverage of polygons or lines
 * in an efficient way.
 *
 * A valid polygonal coverage is a collection of geom::Polygon
 * which satisfy the following conditions:
 *
 *  * Vector-clean - Line segments within the collection
 *    must either be identical or intersect only at endpoints.
 *  * Non-overlapping - No two polygons
 *    may overlap. Equivalently, polygons must be interior-disjoint.
 *
 * A valid linear coverage is a collection of geom::LineString
 * which satisfies the Vector-clean condition.
 * Note that this does not require the LineStrings to be fully noded
 * - i.e. they may contain coincident linework.
 * Coincident line segments are dissolved by the union.
 * Currently linear output is not merged (this may be added in a future release.)
 *
 * Currently no checking is done to determine whether the input is a valid coverage.
 * This is because coverage validation involves segment intersection detection,
 * which is much more expensive than the union phase.
 * If the input is not a valid coverage
 * then in some cases this will be detected during processing
 * and a util::TopologyException is thrown.
 * Otherwise, the computation will produce output, but it will be invalid.
 *
 * Unioning a valid coverage implies that no new vertices are created.
 * This means that a precision model does not need to be specified.
 * The precision of the vertices in the output geometry is not changed.
 *
 * @author Martin Davis
 *
 * @see noding::SegmentExtractingNoder
 *
 */
class GEOS_DLL CoverageUnion {

private:

    CoverageUnion()
    {
        // No instantiation for now
    };



public:

    static constexpr double AREA_PCT_DIFF_TOL = 1e-6;

    /**
    * Unions a valid polygonal coverage or linear network.
    *
    * @param coverage a coverage of polygons or lines
    * @return the union of the coverage
    *
    * @throws util::TopologyException in some cases if the coverage is invalid
    */
    static std::unique_ptr<Geometry> geomunion(const Geometry* coverage);


};


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
