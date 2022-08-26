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

#include <geos/index/strtree/TemplateSTRtree.h>


// Forward declarations
namespace geos {
namespace geom {
class Geometry;
}
}

using geos::geom::Geometry;
using geos::index::strtree::TemplateSTRtree;

namespace geos {      // geos
namespace coverage { // geos::coverage


/**
 * Validates a polygonal coverage, and returns the locations of
 * invalid polygon boundary segments if found.
 *
 * A polygonal coverage is a set of polygons which may be edge-adjacent but do
 * not overlap.
 * Coverage algorithms (such as {@link CoverageUnion} or simplification)
 * generally require the input coverage to be valid to produce correct results.
 * A polygonal coverage is valid if:
 *
 *   * The interiors of all polygons do not intersect (are disjoint).
 *     This is the case if no polygon has a boundary which intersects
 *     the interior of another polygon, and no two polygons are identical.
 *   * If the boundaries of polygons intersect, the vertices
 *     and line segments of the intersection match exactly.
 *
 * A valid coverage may contain holes (regions of no coverage).
 * Sometimes it is desired to detect whether coverages contain
 * narrow gaps between polygons
 * (which can be a result of digitizing error or misaligned data).
 * This class can detect narrow gaps,
 * by specifying a maximum gap width using {@link #setGapWidth(double)}.
 * Note that this also identifies narrow gaps separating disjoint coverage regions,
 * and narrow gores.
 * In some situations it may also produce false positives
 * (linework identified as part of a gap which is actually wider).
 * See CoverageGapFinder for an alternate way to detect gaps which may be more accurate.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL CoverageValidator {

private:

    std::vector<const Geometry*>& m_coverage;
    double m_gapWidth = 0.0;

    std::unique_ptr<Geometry> validate(
        const Geometry* targetGeom,
        TemplateSTRtree<const Geometry*>& index);


public:

    /*
    * Creates a new coverage validator
    *
    * @param coverage a array of polygons representing a polygonal coverage
    */
    CoverageValidator(std::vector<const Geometry*>& coverage)
        : m_coverage(coverage)
        {};

    /**
    * Sets the maximum gap width, if narrow gaps are to be detected.
    *
    * @param gapWidth the maximum width of gaps to detect
    */
    void setGapWidth(double gapWidth) {
        m_gapWidth = gapWidth;
    };

    /**
    * Validates the polygonal coverage.
    * The result is an array of the same size as the input coverage.
    * Each array entry is either null, or if the polygon does not form a valid coverage,
    * a linear geometry containing the boundary segments
    * which intersect polygon interiors, which are mismatched,
    * or form gaps (if checked).
    *
    * @return an array of nulls or linear geometries
    */
    std::vector<std::unique_ptr<Geometry>> validate();

    /**
    * Tests whether a polygonal coverage is valid.
    *
    * @param coverage an array of polygons forming a coverage
    * @return true if the coverage is valid
    */
    static bool isValid(
        std::vector<const Geometry*>& coverage);

    /**
    * Tests if some element of an array of geometries is a coverage invalidity
    * indicator.
    *
    * @param validateResult an array produced by a polygonal coverage validation
    * @return true if the result has at least one invalid indicator
    */
    static bool hasInvalidResult(
        const std::vector<std::unique_ptr<Geometry>>& validateResult);

    /**
    * Validates that a set of polygons forms a valid polygonal coverage.
    * The result is a list of the same length as the input,
    * containing for each input geometry either
    * a linear geometry indicating the locations of invalidities,
    * or a null if the geometry is coverage-valid.
    *
    * @param coverage an array of polygons forming a coverage
    * @return an array of linear geometries indicating coverage errors, or nulls
    */
    static std::vector<std::unique_ptr<Geometry>> validate(
        std::vector<const Geometry*>& coverage);

    /**
    * Validates that a set of polygons forms a valid polygonal coverage
    * and contains no gaps narrower than a specified width.
    * The result is a list of the same length as the input,
    * containing for each input geometry either
    * a linear geometry indicating the locations of invalidities,
    * or a null if the geometry is coverage-valid.
    *
    * @param coverage an array of polygons forming a coverage
    * @param gapWidth the maximum width of invalid gaps
    * @return an array of linear geometries indicating coverage errors, or nulls
    */
    static std::vector<std::unique_ptr<Geometry>> validate(
        std::vector<const Geometry*>& coverage,
        double gapWidth);
};

} // namespace geos::coverage
} // namespace geos
