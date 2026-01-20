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
 **********************************************************************
 *
 * Last port: noding/IteratedNoder.java r591 (JTS-1.12+)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <functional>
#include <memory>
#include <vector>

#include <geos/algorithm/CircularArcIntersector.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/SegmentString.h> // due to inlines
#include <geos/noding/ArcNoder.h> // for inheritance

// Forward declarations
namespace geos {
namespace geom {
class PrecisionModel;
}
}

namespace geos {
namespace noding { // geos::noding

/** \brief
 * Nodes a set of SegmentStrings completely.
 *
 * The set of segmentStrings is fully noded;
 * i.e. noding is repeated until no further
 * intersections are detected.
 *
 * Iterated noding using a FLOATING precision model is not guaranteed to converge,
 * due to roundoff error. This problem is detected and an exception is thrown.
 * Clients can choose to rerun the noding using a lower precision model.
 *
 */
class GEOS_DLL IteratedNoder : public ArcNoder { // implements Noder

private:
    static constexpr int MAX_ITER = 5;


    const geom::PrecisionModel* pm;
    algorithm::CircularArcIntersector cai;
    algorithm::LineIntersector li;
    std::vector<std::unique_ptr<PathString>> nodedPaths;
    int maxIter;
    std::function<std::unique_ptr<Noder>()> m_noderFunction;

    /**
     * Node the input segment strings once
     * and create the split edges between the nodes
     */
    void node(const std::vector<PathString*>& segStrings,
              int& numInteriorIntersections,
              geom::CoordinateXY& intersectionPoint);

    static std::unique_ptr<Noder> createDefaultNoder();

public:

    /** \brief
     * Construct an IteratedNoder using a specific precisionModel and underlying Noder.
     */
    IteratedNoder(const geom::PrecisionModel* newPm, std::function<std::unique_ptr<Noder>()> noderFunction = createDefaultNoder);

    ~IteratedNoder() override;

    /** \brief
     * Sets the maximum number of noding iterations performed before
     * the noding is aborted.
     *
     * Experience suggests that this should rarely need to be changed
     * from the default.
     * The default is MAX_ITER.
     *
     * @param n the maximum number of iterations to perform
     */
    void
    setMaximumIterations(int n)
    {
        maxIter = n;
    }

    std::vector<std::unique_ptr<PathString>> getNodedPaths() override
    {
        return std::move(nodedPaths);
    }

    /** \brief
     * Fully nodes a list of {@link PathString}s, i.e. performs noding iteratively
     * until no intersections are found between segments.
     *
     * Maintains labelling of edges correctly through the noding.
     *
     * @param inputPathStrings a collection of SegmentStrings to be noded
     * @throws TopologyException if the iterated noding fails to converge.
     */
    void computePathNodes(const std::vector<PathString*>& inputPathStrings) override;

    // Declare type as noncopyable
    IteratedNoder(IteratedNoder const&) = delete;
    IteratedNoder& operator=(IteratedNoder const&) = delete;
};

} // namespace geos::noding
} // namespace geos

