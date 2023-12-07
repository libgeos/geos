/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: simplify/TaggedLinesSimplifier.java rev. 1.4 (JTS-1.7.1)
 *
 **********************************************************************
 *
 * NOTES: changed from JTS design adding a private
 *        TaggedLineStringSimplifier member and making
 *        simplify(collection) method become a templated
 *        function.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <vector>
#include <memory>
#include <cassert>

#include <geos/simplify/LineSegmentIndex.h> // for templated function body
#include <geos/simplify/TaggedLineStringSimplifier.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace simplify {
class TaggedLineString;
}
}

namespace geos {
namespace simplify { // geos::simplify

/** \brief
 * Simplifies a collection of TaggedLineStrings, preserving topology
 * (in the sense that no new intersections are introduced).
 */
class GEOS_DLL TaggedLinesSimplifier {

public:

    TaggedLinesSimplifier();

    /** \brief
     * Sets the distance tolerance for the simplification.
     *
     * All vertices in the simplified geometry will be within this
     * distance of the original geometry.
     *
     * @param tolerance the approximation tolerance to use
     */
    void setDistanceTolerance(double tolerance);

    void simplify(std::vector<TaggedLineString*>& tlsVector);

private:

    std::unique_ptr<LineSegmentIndex> inputIndex;

    std::unique_ptr<LineSegmentIndex> outputIndex;

    double distanceTolerance;
};

} // namespace geos::simplify
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

