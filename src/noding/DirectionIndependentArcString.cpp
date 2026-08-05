/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding/DirectionIndependentArcString.h>
#include <geos/noding/ArcString.h>

#include <cstddef>

namespace geos::noding {
static bool
arcStringsSame(const ArcString& as1, const ArcString& as2, bool reverse)
{
    using geom::CircularArc;

    const auto nArcs = as1.getSize();

    if (as2.getSize() != nArcs) {
        return false;
    }

    for (std::size_t i = 0; i < nArcs; i++) {
        const CircularArc& arc1 = as1.getArc(i);
        const CircularArc& arc2 = as2.getArc(reverse ? nArcs - i - 1: i);

        if (reverse) {
            if (arc1.p0() != arc2.p2() || arc1.p2() != arc2.p0()) {
                return false;
            }

            const bool arc1Linear = arc1.getOrientation() == algorithm::Orientation::COLLINEAR;
            const bool arc2Linear = arc2.getOrientation() == algorithm::Orientation::COLLINEAR;

            if (arc1Linear != arc2Linear) {
                return false;
            }

            if (!arc1Linear && arc1.getOrientation() == arc2.getOrientation()) {
                return false;
            }
        }
        else {
            if (arc1.p0() != arc2.p0() || arc1.p2() != arc2.p2()) {
                return false;
            }
            if (arc1.getOrientation() != arc2.getOrientation()) {
                return false;
            }
        }

        if (!arc1.isLinear() && arc1.getCenter() != arc2.getCenter()) {
            return false;
        }
    }

    return true;
}

bool
DirectionIndependentArcString::operator==(const DirectionIndependentArcString& other) const
{
    return arcStringsSame(m_as, other.m_as, false) || arcStringsSame(m_as, other.m_as, true);
}
}

std::size_t
std::hash<geos::noding::DirectionIndependentArcString>::operator()(const geos::noding::DirectionIndependentArcString&
        as) const noexcept
{
    const auto* c1 = &as.getArcString().getArc(0).p0();
    const auto* c2 = &as.getArcString().getArc(as.getArcString().getSize() - 1).p2();

    if (c2->compareTo(*c1) < 0) {
        std::swap(c1, c2);
    }

    size_t h = std::hash<double> {}(c1->x);
    h ^= std::hash<double> {}(c1->y) << 1;
    h ^= std::hash<double> {}(c2->x) << 1;
    h ^= std::hash<double> {}(c2->y) << 1;

    return h;
}
