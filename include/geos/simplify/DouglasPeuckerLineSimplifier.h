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
 * Last port: simplify/DouglasPeuckerLineSimplifier.java rev. 1.4
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/CoordinateSequence.h>
#include <vector>
#include <memory> // for unique_ptr

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
}

namespace geos {
namespace simplify { // geos::simplify

/** \brief
 * Simplifies a linestring (sequence of points) using
 * the standard Douglas-Peucker algorithm.
 */
class GEOS_DLL DouglasPeuckerLineSimplifier {

public:

    /** \brief
     * Returns a newly allocated Coordinate vector, wrapped
     * into an unique_ptr
     */
    static std::unique_ptr<geom::CoordinateSequence> simplify(
        const geom::CoordinateSequence& nPts,
        double distanceTolerance,
        bool preserveClosedEndpoint);

    DouglasPeuckerLineSimplifier(const geom::CoordinateSequence& nPts);

    /** \brief
     * Sets the distance tolerance for the simplification.
     *
     * All vertices in the simplified linestring will be within this
     * distance of the original linestring.
     *
     * @param nDistanceTolerance the approximation tolerance to use
     */
    void setDistanceTolerance(double nDistanceTolerance);

    /** \brief
     * Sets whether the endpoint of a closed LineString should be preserved
     *
     * @param preserve `true` if the endpoint should be preserved
     */
    void setPreserveClosedEndpoint(bool preserve);

    /** \brief
     * Returns a newly allocated Coordinate vector, wrapped
     * into an unique_ptr
     */
    std::unique_ptr<geom::CoordinateSequence> simplify();

private:

    const geom::CoordinateSequence& pts;
    std::vector<bool> usePt;
    double distanceTolerance;
    bool preserveEndpoint;

    void simplifySection(std::size_t i, std::size_t j);

    // Declare type as noncopyable
    DouglasPeuckerLineSimplifier(const DouglasPeuckerLineSimplifier& other) = delete;
    DouglasPeuckerLineSimplifier& operator=(const DouglasPeuckerLineSimplifier& rhs) = delete;
};

} // namespace geos::simplify
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

