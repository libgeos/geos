/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Location.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/operation/relateng/BasicPredicate.h>
#include <geos/export.h>

#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
    class Envelope;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation.
namespace relateng {  // geos.operation.relateng


class GEOS_DLL IMPredicate : public BasicPredicate {
protected:
    using Envelope = geos::geom::Envelope;
    using Location = geos::geom::Location;
    using Dimension = geos::geom::Dimension;
    using IntersectionMatrix = geos::geom::IntersectionMatrix;

private:



protected:

    static constexpr int DIM_UNKNOWN = Dimension::DONTCARE;

    int dimA;
    int dimB;
    IntersectionMatrix intMatrix;

    /**
     * Gets the value of the predicate according to the current
     * intersection matrix state.
     *
     * @return the current predicate value
     */
    virtual bool valueIM() = 0;

    /**
     * Tests whether predicate evaluation can be short-circuited
     * due to the current state of the matrix providing
     * enough information to determine the predicate value.
     *
     * If this value is true then valueIM()
     * must provide the correct result of the predicate.
     *
     * @return true if the predicate value is determined
     */
    virtual bool isDetermined() const = 0;

    /**
     * Tests whether the exterior of the specified input geometry
     * is intersected by any part of the other input.
     *
     * @param isA the input geometry
     * @return true if the input geometry exterior is intersected
     */
    bool intersectsExteriorOf(bool isA) const;

    bool isIntersects(Location locA, Location locB) const;


public:

    IMPredicate()
    {
        // intMatrix = new IntersectionMatrix();
        //-- E/E is always dim = 2
        intMatrix.set(Location::EXTERIOR, Location::EXTERIOR, Dimension::A);
    }

    static bool isDimsCompatibleWithCovers(int dim0, int dim1);

    void init(int dA, int dB) override;

    void updateDimension(Location locA, Location locB, int dimension) override;

    bool isDimChanged(Location locA, Location locB, int dimension) const;

    using TopologyPredicate::isKnown;
    bool isKnown(Location locA, Location locB) const;

    bool isDimension(Location locA, Location locB, int dimension) const;

    int getDimension(Location locA, Location locB) const;

    /**
     * Sets the final value based on the state of the IM.
     */
    void finish() override;

    std::string toString() const;

    friend std::ostream& operator<<(std::ostream& os, const IMPredicate& imp);


};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

