/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025-2026 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/Angle.h>
#include <geos/algorithm/CurveBuilder.h>

#include <geos/geom/CircularArc.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>

namespace geos::algorithm {

using geom::CircularArc;
using geom::CoordinateSequence;
using geom::CoordinateXY;
using geom::CoordinateXYZM;
using geom::GeometryFactory;
using geom::LineString;

CurveBuilder::CurveBuilder(const GeometryFactory &p_factory)
    : factory(p_factory) {}

std::unique_ptr<geom::Curve>
CurveBuilder::getCurved(const LineString &ls, double distanceTolerance) {
    CurveBuilder cb(*ls.getFactory());
    return cb.compute(ls, distanceTolerance);
}

std::unique_ptr<geom::Curve>
CurveBuilder::compute(const LineString& ls, double distanceTolerance) {
    const auto& points = *ls.getSharedCoordinates();

    std::size_t start = 0;

    while (start + 1 < points.getSize()) {

        if (start + 2 >= points.getSize()) {
            addLineCoords(points, start, start + 1);
            break;
        }

        CircularArc arc(points, start);

        if (arc.isLinear() || arc.getAngle() > 2*maxSpacingRadians) {
            addLineCoords(points, start, start + 2);
            start += 2;
            continue;
        }

        std::size_t stop = start + 3;
        bool foundArc = false;

        // Continue to consume points until we are no longer on the same arc.
        // We need to find at least one additional point that fits on this arc to consider this an arc.
        while (stop < points.getSize()) {
            const CoordinateXY& pt = points.getAt<CoordinateXY>(stop);

            const double distance = pt.distance(arc.getCenter());

            // Does the radius match?
            if (std::abs(distance - arc.getRadius()) > distanceTolerance) {
                break;
            }

            const CoordinateXY& prev1 = points.getAt<CoordinateXY>(stop - 1);
            const CoordinateXY& prev2 = points.getAt<CoordinateXY>(stop - 2);

            // Is the angle similar to, or less than, the previous ones?
            const double prevAngle = Angle::angleBetween(arc.p0(), arc.p1(), arc.p2());
            const double currAngle = Angle::angleBetween(prev2, prev1, pt);

            double angleTolerance = prevAngle*1e-3;

            if (std::abs(currAngle - prevAngle) > angleTolerance) {
                break;
            }

            foundArc = true;
            stop++;
        }

        if (foundArc) {
            addArc(arc, stop - 1);
            start = stop - 1;
        } else {
            addLineCoords(points, start, start + 1);
            start++;
        }

    }

    finishArc();
    finishLine();

    if (curves.size() == 1) {
        return std::move(curves.front());
    }

    return factory.createCompoundCurve(std::move(curves));
}

void
CurveBuilder::addLineCoords(const CoordinateSequence& points, std::size_t from, std::size_t to)
{
    finishArc();

    if (lineCoords) {
        lineCoords->add(points, from + 1, to);
    } else {
        lineCoords = std::make_shared<CoordinateSequence>(0, points.hasZ(), points.hasM());
        lineCoords->add(points, from, to);
    }
}

void
CurveBuilder::finishArc()
{
    if (arcCoords) {
        curves.push_back(factory.createCircularString(arcCoords));
        arcCoords.reset();
    }
}

void
CurveBuilder::finishLine() {
    if (lineCoords) {
        curves.push_back(factory.createLineString(lineCoords));
        lineCoords.reset();
    }
}

void
CurveBuilder::addArc(const CircularArc& arc, std::size_t stop) {
    finishLine();

    const CoordinateSequence& points = *arc.getCoordinateSequence();
    std::size_t start = arc.getCoordinatePosition();

    double xSum = 0.0;
    double ySum = 0.0;
    std::size_t nArcApproximations = 0;

    for (std::size_t i = start; i <= stop - 2; i++) {
        CircularArc a(points, i);
        const CoordinateXY& center = a.getCenter();

        xSum += center.x;
        ySum += center.y;
        nArcApproximations++;
    }

    const CoordinateXY averageCenter = { xSum / static_cast<double>(nArcApproximations), ySum / static_cast<double>(nArcApproximations) };

    CoordinateXYZM p0, p1, p2;
    points.getAt(start, p0);
    points.getAt(stop, p2);

    const double averageRadius = 0.5*(p0.distance(averageCenter) + p2.distance(averageCenter));

    p1 = CircularArcs::getMidpoint(p0, p2, averageCenter, averageRadius, arc.isCCW());

    if (points.hasZ() || points.hasM()) {
        auto nPoints = stop - start + 1;

        if (nPoints % 2) {
            // We have an odd number of vertices, so the central vertex should be the same
            // as the control point in the original arc.
            auto midpointIndex = start + nPoints / 2;
            p1.z = points.getZ(midpointIndex);
            p1.m = points.getM(midpointIndex);
        } else {
            // We have an even number of vertices. Calculate Z/M of the control
            // point in the original arc.
            CoordinateXYZM a, b;
            points.getAt(start + nPoints / 2 - 1, a);
            points.getAt(start + nPoints / 2, b);

            const double thetaA = CircularArcs::getAngle(a, averageCenter);
            const double theta0 = CircularArcs::getAngle(p0, averageCenter);
            const double theta1 = CircularArcs::getAngle(p1, averageCenter);

            // Assumption: point a has the same angle fraction over [p0, p1] that b has over [p2, p1]
            const double f = arc.isCCW() ? Angle::fractionCCW(thetaA, theta0, theta1) : 1 - Angle::fractionCCW(thetaA, theta1, theta0);

            p1.z = (a.z + b.z - p0.z*(1 - f) - p2.z*(1 - f)) / (2 * f);
            p1.m = (a.m + b.m - p0.m*(1 - f) - p2.m*(1 - f)) / (2 * f);
        }
    }

    if (!arcCoords) {
        arcCoords = std::make_shared<CoordinateSequence>(0,  points.hasZ(), points.hasM());
        arcCoords->reserve(3);
        arcCoords->add(p0);
    }

    arcCoords->add(p1);
    arcCoords->add(p2);
}

}
