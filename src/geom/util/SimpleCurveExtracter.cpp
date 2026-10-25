/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 ISciences LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/CompoundCurve.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/SimpleCurve.h>
#include <geos/geom/util/SimpleCurveExtracter.h>
#include <geos/util.h>

#include <vector>

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

SimpleCurveExtracter::SimpleCurveExtracter(std::vector<const SimpleCurve*>& newComps)
    :
    comps(newComps)
{}

void
SimpleCurveExtracter::getCurves(const Geometry& geom, std::vector<const SimpleCurve*>& ret)
{
    if (geom.getDimension() == Dimension::P) {
        return;
    }

    SimpleCurveExtracter lce(ret);
    geom.apply_ro(&lce);
}

void
SimpleCurveExtracter::filter_ro(const Geometry* geom)
{
    if (geom->isEmpty()) {
        return;
    }

    const auto typ = geom->getGeometryTypeId();

    if (typ == GEOS_LINEARRING || typ == GEOS_LINESTRING || typ == GEOS_CIRCULARSTRING || typ == GEOS_ELLIPTICALCURVE) {
        comps.push_back(detail::down_cast<const SimpleCurve*>(geom));
    }

    if (typ == GEOS_COMPOUNDCURVE) {
        const CompoundCurve* cc = detail::down_cast<const CompoundCurve*>(geom);
        for (std::size_t i = 0; i < cc->getNumCurves(); i++) {
            comps.push_back(cc->getCurveN(i));
        }
    }
}

}
}
}
