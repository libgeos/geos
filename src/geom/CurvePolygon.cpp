/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/Area.h>
#include <geos/geom/Curve.h>
#include <geos/geom/CurvePolygon.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiCurve.h>
#include <geos/geom/SimpleCurve.h>
#include <geos/util/UnsupportedOperationException.h>

namespace geos {
namespace geom {

    std::unique_ptr<CoordinateSequence>
    CurvePolygon::getCoordinates() const
    {
        auto coordinates = shell->getCoordinates();
        for (const auto& hole : holes) {
            if (auto simpleHole = dynamic_cast<const SimpleCurve*>(hole.get())) {
                coordinates->add(*simpleHole->getCoordinatesRO());
            } else {
                coordinates->add(*hole->getCoordinates());
            }
        }
        return coordinates;
    }

    std::string CurvePolygon::getGeometryType() const {
        return "CurvePolygon";
    }

    GeometryTypeId CurvePolygon::getGeometryTypeId() const {
        return GEOS_CURVEPOLYGON;
    }

    static std::unique_ptr<Geometry>
    getRingForBoundary(const Geometry* ring)
    {
        // Convert LinearRing -> LineString
        if (ring->getGeometryTypeId() == GEOS_LINEARRING) {
            return ring->getFactory()->createLineString(*static_cast<const LineString*>(ring));
        }
        return ring->clone();
    }

    std::unique_ptr<Geometry>
    CurvePolygon::getBoundary() const {

        const GeometryFactory* gf = getFactory();

        if(isEmpty()) {
            return gf->createMultiCurve();
        }

        if(holes.empty()) {
            return getRingForBoundary(shell.get());
        }

        std::vector<std::unique_ptr<Geometry>> rings(holes.size() + 1);

        rings[0] = getRingForBoundary(shell.get());
        for(std::size_t i = 0; i < holes.size(); ++i) {
            rings[i + 1] = getRingForBoundary(holes[i].get());
        }

        return getFactory()->createMultiCurve(std::move(rings));
    }

    void
    CurvePolygon::normalize()
    {
        shell->normalize();
        for(auto& lr : holes) {
            lr->normalize();
            lr = lr->reverse();
        }
        std::sort(holes.begin(), holes.end(), [](const auto& a, const auto& b) {
            return a->compareTo(b.get()) > 0;
        });
    }

    double CurvePolygon::getArea() const {
        double sum = algorithm::Area::ofClosedCurve(*shell);
        for (const auto& hole : holes) {
            sum -= algorithm::Area::ofClosedCurve(*hole);
        }
        return sum;
    }

    std::unique_ptr<Polygon>
    CurvePolygon::getLinearized(double spacingDegrees) const {
        return std::unique_ptr<Polygon>(getLinearizedImpl(spacingDegrees));
    }

    Polygon*
    CurvePolygon::getLinearizedImpl(double spacingDegrees) const {
        const auto& gfact = *getFactory();

        auto linShell = gfact.createLinearRing(shell->getLinearized(spacingDegrees)->getSharedCoordinates());

        if (holes.empty()) {
            return getFactory()->createPolygon(std::move(linShell)).release();
        }

        std::vector<std::unique_ptr<LinearRing>> linHoles(holes.size());
        for (size_t i = 0; i < holes.size(); i++) {
            linHoles[i] = gfact.createLinearRing(holes[i]->getLinearized(spacingDegrees)->getSharedCoordinates());
        }
        return getFactory()->createPolygon(std::move(linShell), std::move(linHoles)).release();
    }

    bool CurvePolygon::hasCurvedComponents() const {
        if (shell->hasCurvedComponents()) {
            return true;
        }
        for (const auto& hole : holes) {
            if (hole->hasCurvedComponents()) {
                return true;
            }
        }
        return false;
    }

    CurvePolygon*
    CurvePolygon::cloneImpl() const {
        return new CurvePolygon(*this);
    }

    Geometry*
    CurvePolygon::reverseImpl() const {
        std::unique_ptr<Curve> revShell(static_cast<Curve*>(shell->reverse().release()));
        std::vector<std::unique_ptr<Curve>> revHoles(holes.size());
        for (std::size_t i = 0; i < revHoles.size(); i++) {
            revHoles[i].reset(static_cast<Curve*>(holes[i]->reverse().release()));
        }
        return new CurvePolygon(std::move(revShell), std::move(revHoles), *getFactory());
    }

}
}
