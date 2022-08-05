/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <vector>
#include <memory>


// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class LinearRing;
}
}

using geos::geom::Geometry;
using geos::geom::LinearRing;


namespace geos {      // geos
namespace coverage { // geos::coverage

/**
 * Finds gaps in a polygonal coverage.
 * Gaps are holes in the coverage which are narrower than a given width.
 *
 * The coverage should be valid according to CoverageValidator.
 * If this is not the case, some gaps may not be reported, or the invocation may fail.
 *
 * This is a more accurate way of identifying gaps
 * than using CoverageValidator::setGapWidth(double).
 * Gaps which separate the coverage into two disjoint regions are not detected.
 * Gores are not identified as gaps.
 *
 * @author mdavis
 *
 */
class GEOS_DLL CoverageGapFinder {

private:

    std::vector<const Geometry*>& m_coverage;

    bool isGap(const LinearRing* hole, double gapWidth);


public:

    /**
    * Creates a new polygonal coverage gap finder.
    *
    * @param coverage a set of polygons forming a polygonal coverage
    */
    CoverageGapFinder(std::vector<const Geometry*>& coverage)
        : m_coverage(coverage)
        {};

    /**
    * Finds gaps in a polygonal coverage.
    * Returns lines indicating the locations of the gaps.
    *
    * @param coverage a set of polygons forming a polygonal coverage
    * @param gapWidth the maximum width of gap to detect
    * @return a geometry indicating the locations of gaps (which is empty if no gaps were found), or null if the coverage was empty
    */
    static std::unique_ptr<Geometry> findGaps(
        std::vector<const Geometry*>& coverage,
        double gapWidth);

    /**
    * Finds gaps in the coverage.
    * Returns lines indicating the locations of the gaps.
    *
    * @param gapWidth the maximum width of gap to detect
    * @return a geometry indicating the locations of gaps (which is empty if no gaps were found), or null if the coverage was empty
    */
    std::unique_ptr<Geometry> findGaps(double gapWidth);


};

} // namespace geos::coverage
} // namespace geos









