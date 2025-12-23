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

        if (arc.isLinear()) {
            addLineCoords(points, start, start + 2);
            start += 2;
            continue;
        }

        std::size_t stop = start + 3;
        bool foundArc = false;

        // Continue to consume points until we are no longer on the same arc.
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
            double angle = Angle::angleBetweenOriented(arc.p0(), arc.getCenter(), points.getAt<CoordinateXY>(stop - 1));

            if (angle == 0) {
                angle = 2*M_PI;
            } else if (arc.isCCW()) {
                if (angle < 0) {
                    // Arc is CCW but angle is CW
                    angle = 2*M_PI + angle;
                }
            } else {
                if (angle > 0) {
                    // Arc is CW but angle is CCW
                    angle = 2*M_PI - angle;
                } else {
                    angle = std::abs(angle);
                }
            }

            constexpr int edgesPerQuadrant = 2;
            const std::size_t minArcEdges = static_cast<std::size_t>(std::round(angle * 2 / M_PI * edgesPerQuadrant));
            const std::size_t actualArcEdges = stop - start - 1;

            if (actualArcEdges < minArcEdges) {
                foundArc = false;
            }
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
    p1.z = 0.5*(p0.z + p2.z);
    p1.m = 0.5*(p0.m + p2.m);

    if (!arcCoords) {
        arcCoords = std::make_shared<CoordinateSequence>( points.hasZ(), points.hasM());
        arcCoords->reserve(3);
        arcCoords->add(p0);
    }

    arcCoords->add(p1);
    arcCoords->add(p2);
}

}