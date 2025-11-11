/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006      Refractions Research Inc.
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

// Forward declarations
namespace geos {
namespace noding {
class SegmentString;
}
}

namespace geos {
namespace noding { // geos.noding


/** \brief
 * Computes all intersections between segments in a set of SegmentString.
 *
 * Intersections found are represented as [SegmentNodes](@ref SegmentNode) and
 * added to the [SegmentStrings](@ref SegmentString) in which they occur.
 * As a final step in the noding a new set of segment strings split
 * at the nodes may be returned.
 *
 * Last port: noding/Noder.java rev. 1.8 (JTS-1.7)
 *
 */
class GEOS_DLL Noder {
public:
    /** \brief
     * Computes the noding for a collection of [SegmentStrings](@ref SegmentString).
     *
     * Some Noders may add all these nodes to the input SegmentStrings;
     * others may only add some or none at all.
     *
     * @param segStrings a collection of {@link SegmentString}s to node
     *        The caller remains responsible for releasing the memory
     *        associated with the container and its elements.
     */
    virtual void computeNodes(const std::vector<SegmentString*>& segStrings) = 0;

    /** \brief
     * Returns a collection of fully noded [SegmentStrings](@ref SegmentString).
     * The SegmentStrings have the same context as their parent.
     *
     * @return a std::vector of SegmentStrings (copies of input, if needs be).
     *         Caller is responsible to delete elements of container.
     *         This method may only be called once.
     */
    virtual std::vector<SegmentString*> getNodedSubstrings() = 0;

    virtual
    ~Noder() {}

protected:
    Noder() {}
};

} // namespace geos.noding
} // namespace geos

