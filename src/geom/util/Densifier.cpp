/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2010 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/polygonize/Polygonizer.java rev. 1.6 (JTS-1.10)
 *
 **********************************************************************/

#include <cmath>

#include <geos/geom/util/Densifier.h>
#include <geos/geom/util/GeometryFixer.h>
#include <geos/geom/CoordinateList.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/util/Interrupt.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>

#include <limits>
#include <vector>

using namespace geos::geom;
using namespace geos::geom::util;

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

/* geom::util::Densifier::DensifyTransformer */
Densifier::DensifyTransformer::DensifyTransformer(double distTol):
    distanceTolerance(distTol)
{}

CoordinateSequence::Ptr
Densifier::DensifyTransformer::transformCoordinates(const CoordinateSequence* coords, const Geometry* parent)
{
    Coordinate::Vect emptyPts;
    Coordinate::Vect inputPts;
    auto newPts = Densifier::densifyPoints(*coords, distanceTolerance, parent->getPrecisionModel());

    if(const LineString* ls = dynamic_cast<const LineString*>(parent)) {
        if(ls->getNumPoints() <= 1) {
            newPts->clear();
        }
    }

    return newPts;
}

Geometry::Ptr
Densifier::DensifyTransformer::transformPolygon(const Polygon* geom, const Geometry* parent)
{
    Geometry::Ptr roughGeom = GeometryTransformer::transformPolygon(geom, parent);
    // don't try and correct if the parent is going to do this
    if(parent && parent->getGeometryTypeId() == GEOS_MULTIPOLYGON) {
        return roughGeom;
    }
    Geometry::Ptr validGeom(createValidArea(roughGeom.get()));
    return validGeom;
}

Geometry::Ptr
Densifier::DensifyTransformer::transformMultiPolygon(const MultiPolygon* geom, const Geometry* parent)
{
    Geometry::Ptr roughGeom = GeometryTransformer::transformMultiPolygon(geom, parent);
    Geometry::Ptr validGeom(createValidArea(roughGeom.get()));
    return validGeom;
}

std::unique_ptr<Geometry>
Densifier::DensifyTransformer::createValidArea(const Geometry* roughAreaGeom)
{
    if (roughAreaGeom->isValid())
        return Geometry::Ptr(roughAreaGeom->clone());
    return GeometryFixer::fix(roughAreaGeom);
}

/* util::Densifier */

Densifier::Densifier(const Geometry* geom):
    inputGeom(geom)
{}

std::unique_ptr<CoordinateSequence>
Densifier::densifyPoints(const CoordinateSequence& pts, double distanceTolerance, const PrecisionModel* precModel)
{
    auto coordList = detail::make_unique<CoordinateSequence>(0, pts.hasZ(), pts.hasM());

    pts.forEachSegment([&coordList, distanceTolerance, precModel](const auto& p0, const auto& p1) {
        coordList->add(p0, false);

        const double len = p0.distance(p1);
        const double densifiedSegCountDbl = std::ceil(len / distanceTolerance);
        if(densifiedSegCountDbl > std::numeric_limits<int>::max()) {
            throw geos::util::GEOSException(
                "Tolerance is too small compared to geometry length");
        }

        const int densifiedSegCount = static_cast<int>(densifiedSegCountDbl);
        if(densifiedSegCount > 1) {
            double densifiedSegLen = len / densifiedSegCount;
            for(int j = 1; j < densifiedSegCount; j++) {
                double segFract = (j * densifiedSegLen) / len;
                auto p = LineSegment::pointAlong(segFract, p0, p1);
                precModel->makePrecise(p);
                coordList->add(p, false);
            }
        }
        else {
            // no densification required; insert the last coordinate and continue
            coordList->add(p1, false);
        }

    });
    pts.applyAt(pts.size() - 1, [&coordList](const auto& pt) {
        coordList->add(pt, false);
    });

    return coordList;
}

/**
 * Densifies a geometry using a given distance tolerance,
 * and respecting the input geometry's {@link PrecisionModel}.
 *
 * @param geom the geometry to densify
 * @param distanceTolerance the distance tolerance to densify
 * @return the densified geometry
 */
Geometry::Ptr
Densifier::densify(const Geometry* geom, double distTol)
{
    util::Densifier densifier(geom);
    densifier.setDistanceTolerance(distTol);
    return densifier.getResultGeometry();
}

void
Densifier::setDistanceTolerance(double tol)
{
    // Test written to catch NaN as well
    if(!(tol > 0.0)) {
        throw geos::util::IllegalArgumentException("Tolerance must be positive");
    }
    distanceTolerance = tol;
}

Geometry::Ptr
Densifier::getResultGeometry() const
{
    if (inputGeom->isEmpty()) {
        return inputGeom->clone();
    }

    DensifyTransformer dt(distanceTolerance);
    return dt.transform(inputGeom);
}


} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos
