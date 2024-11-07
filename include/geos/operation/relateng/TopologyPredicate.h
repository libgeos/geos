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
#include <geos/export.h>

#include <ostream> // for operator<<
#include <string>

// Forward declarations
namespace geos {
namespace geom {
    class Envelope;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


class GEOS_DLL TopologyPredicate {
    using Envelope = geos::geom::Envelope;
    using Location = geos::geom::Location;

public:

    /* Virtual destructor to ensure proper cleanup of derived classes */
    virtual ~TopologyPredicate() {};

    /**
     * Gets the name of the predicate.
     *
     * @return the predicate name
     */
    virtual std::string name() const = 0;

    /**
     * Indicates that the value of the predicate can be finalized
     * based on its current state.
     */
    virtual void finish() = 0;

    /**
     * Tests if the predicate value is known.
     *
     * @return true if the result is known
     */
    virtual bool isKnown() const = 0;

    /**
     * Gets the current value of the predicate result.
     * The value is only valid if isKnown() is true.
     *
     * @return the predicate result value
     */
    virtual bool value() const = 0;

    /**
     * Reports whether this predicate requires self-noding for
     * geometries which contain crossing edges
     * (for example, LineString, or GeometryCollection
     * containing lines or polygons which may self-intersect).
     * Self-noding ensures that intersections are computed consistently
     * in cases which contain self-crossings and mutual crossings.
     *
     * Most predicates require this, but it can
     * be avoided for simple intersection detection
     * (such as in RelatePredicate#intersects()
     * and RelatePredicate#disjoint().
     * Avoiding self-noding improves performance for polygonal inputs.
     *
     * @return true if self-noding is required.
     */
    virtual bool requireSelfNoding() const {
        return true;
    };

    /**
     * Reports whether this predicate requires interaction between
     * the input geometries.
     * This is the case if
     *
     * IM[I, I] >= 0 or IM[I, B] >= 0 or IM[B, I] >= 0 or IM[B, B] >= 0
     *
     * This allows a fast result if
     * the envelopes of the geometries are disjoint.
     *
     * @return true if the geometries must interact
     */
    virtual bool requireInteraction() const {
        return true;
    };

    /**
     * Reports whether this predicate requires that the source
     * cover the target.
     * This is the case if
     *
     * IM[Ext(Src), Int(Tgt)] = F and IM[Ext(Src), Bdy(Tgt)] = F
     *
     * If true, this allows a fast result if
     * the source envelope does not cover the target envelope.
     *
     * @param isSourceA indicates the source input geometry
     * @return true if the predicate requires checking whether the source covers the target
     */
    virtual bool requireCovers(bool isSourceA) {
        (void)isSourceA;
        return false;
    }

    /**
     * Reports whether this predicate requires checking if the source input intersects
     * the Exterior of the target input.
     * This is the case if:
     *
     * IM[Int(Src), Ext(Tgt)] >= 0 or IM[Bdy(Src), Ext(Tgt)] >= 0
     *
     * If false, this may permit a faster result in some geometric situations.
     *
     * @param isSourceA indicates the source input geometry
     * @return true if the predicate requires checking whether the source intersects the target exterior
     */
    virtual bool requireExteriorCheck(bool isSourceA) const {
        (void)isSourceA;
        return true;
    }

    /**
     * Initializes the predicate for a specific geometric case.
     * This may allow the predicate result to become known
     * if it can be inferred from the dimensions.
     *
     * @param dimA the dimension of geometry A
     * @param dimB the dimension of geometry B
     *
     * @see Dimension
     */
    virtual void init(int dimA, int dimB) {
        (void)dimA;
        (void)dimB;
    };

    /**
     * Initializes the predicate for a specific geometric case.
     * This may allow the predicate result to become known
     * if it can be inferred from the envelopes.
     *
     * @param envA the envelope of geometry A
     * @param envB the envelope of geometry B
     */
    virtual void init(const Envelope& envA, const Envelope& envB)
    {
        //-- default if envelopes provide no information
        (void)envA;
        (void)envB;
    };

    /**
     * Updates the entry in the DE-9IM intersection matrix
     * for given Location in the input geometries.
     *
     * If this method is called with a {@link Dimension} value
     * which is less than the current value for the matrix entry,
     * the implementing class should avoid changing the entry
     * if this would cause information loss.
     *
     * @param locA the location on the A axis of the matrix
     * @param locB the location on the B axis of the matrix
     * @param dimension the dimension value for the entry
     *
     * @see Dimension
     * @see Location
     */
    virtual void updateDimension(Location locA, Location locB, int dimension) = 0;


    friend std::ostream&
    operator<<(std::ostream& os, const TopologyPredicate& ns)
    {
        os << ns.name();
        return os;
    }

};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

