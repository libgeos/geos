/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 ISciences, LLC
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/Curve.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Surface.h>
#include <geos/util/IllegalArgumentException.h>

namespace geos {
namespace geom {

template<typename RingType>
class SurfaceImpl : public Surface {

protected:

    SurfaceImpl(const SurfaceImpl& p)
        :
        Surface(p),
        shell(static_cast<RingType*>(p.shell->clone().release())),
        holes(p.holes.size())
    {
        for (std::size_t i = 0; i < holes.size(); ++i) {
            holes[i].reset(static_cast<RingType*>(p.holes[i]->clone().release()));
        }
    }

    /**
     * Constructs a <code>Surface</code> with the given exterior
     * and interior boundaries.
     *
     * @param  newShell  the outer boundary of the new Polygon,
     *                   or <code>null</code> or an empty
             *		     Curve if the empty geometry
     *                   is to be created.
     *
     * @param  newHoles  the rings defining the inner
     *                   boundaries of the new Surface, or
     *                   null or empty Curve
     *                   if the empty geometry is to be created.
     *
     * @param newFactory the GeometryFactory used to create this geometry
     *
     * Polygon will take ownership of shell and hole curves
     */
    SurfaceImpl(std::unique_ptr<RingType>&& newShell,
                const GeometryFactory& newFactory) :
        Surface(&newFactory),
        shell(std::move(newShell))
    {
        if (shell == nullptr) {
            shell.reset(static_cast<RingType*>(createEmptyRing(newFactory).release()));
        }
    }

    SurfaceImpl(std::unique_ptr<RingType>&& newShell,
                std::vector<std::unique_ptr<RingType>>&& newHoles,
                const GeometryFactory& newFactory) :
        Surface(&newFactory),
        shell(std::move(newShell)),
        holes(std::move(newHoles))
    {
        if (shell == nullptr) {
            shell.reset(static_cast<RingType*>(createEmptyRing(newFactory).release()));
        }

        if(shell->isEmpty() && hasNonEmptyElements(&holes)) {
            throw geos::util::IllegalArgumentException("shell is empty but holes are not");
        }
        if (hasNullElements(&holes)) {
            throw geos::util::IllegalArgumentException("holes must not contain null elements");
        }
    }

public:

    const RingType*
    getExteriorRing() const override
    {
        return shell.get();
    }

    RingType*
    getExteriorRing() override
    {
        return shell.get();
    }

    const RingType*
    getInteriorRingN(std::size_t n) const override
    {
        return holes[n].get();
    }

    RingType*
    getInteriorRingN(std::size_t n) override
    {
        return holes[n].get();
    }

    size_t getNumInteriorRing() const override
    {
        return holes.size();
    }

    /**
    * \brief
    * Take ownership of this Surface's exterior ring.
    * After releasing the exterior ring, the Surface should be
    * considered in a moved-from state and should not be accessed,
    * except to release the interior rings (if desired.)
    * @return exterior ring
    */
    std::unique_ptr<RingType>
    releaseExteriorRing()
    {
        return std::move(shell);
    }

    /**
    * \brief
    * Take ownership of this Surfaces's interior rings.
    * After releasing the rings, the Surface should be
    * considered in a moved-from state and should not be accessed,
    * except to release the exterior ring (if desired.)
    * @return vector of rings (may be empty)
    */
    std::vector<std::unique_ptr<RingType>> releaseInteriorRings()
    {
        return std::move(holes);
    }

protected:
    std::unique_ptr<RingType> shell;
    std::vector<std::unique_ptr<RingType>> holes;

};

}
}
