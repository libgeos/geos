/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: util/GeometricShapeFactory.java rev 1.14 (JTS-1.10+)
 * (2009-03-19)
 *
 **********************************************************************/

#include <geos/constants.h>
#include <geos/util/GeometricShapeFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Envelope.h>
#include <geos/util.h>

#include <vector>
#include <cmath>
#include <memory>


using namespace geos::geom;

namespace geos {
namespace util { // geos.util

GeometricShapeFactory::GeometricShapeFactory(const GeometryFactory* factory)
    :
    geomFact(factory),
    precModel(factory->getPrecisionModel()),
    nPts(100)
{
}

void
GeometricShapeFactory::setBase(const CoordinateXY& base)
{
    dim.setBase(base);
}

void
GeometricShapeFactory::setCentre(const CoordinateXY& centre)
{
    dim.setCentre(centre);
}

void
GeometricShapeFactory::setNumPoints(uint32_t nNPts)
{
    nPts = nNPts;
}

void
GeometricShapeFactory::setSize(double size)
{
    dim.setSize(size);
}

void
GeometricShapeFactory::setWidth(double width)
{
    dim.setWidth(width);
}

void
GeometricShapeFactory::setHeight(double height)
{
    dim.setHeight(height);
}

std::unique_ptr<Polygon>
GeometricShapeFactory::createRectangle()
{
    uint32_t i;
    uint32_t ipt = 0;
    uint32_t nSide = nPts / 4;
    if(nSide < 1) {
        nSide = 1;
    }
    std::unique_ptr<Envelope> env(dim.getEnvelope());
    double XsegLen = env->getWidth() / nSide;
    double YsegLen = env->getHeight() / nSide;

    auto vc = detail::make_unique<CoordinateSequence>(4 * nSide + 1);

    for(i = 0; i < nSide; i++) {
        double x = env->getMinX() + i * XsegLen;
        double y = env->getMinY();
        (*vc)[ipt++] = coord(x, y);
    }
    for(i = 0; i < nSide; i++) {
        double x = env->getMaxX();
        double y = env->getMinY() + i * YsegLen;
        (*vc)[ipt++] = coord(x, y);
    }
    for(i = 0; i < nSide; i++) {
        double x = env->getMaxX() - i * XsegLen;
        double y = env->getMaxY();
        (*vc)[ipt++] = coord(x, y);
    }
    for(i = 0; i < nSide; i++) {
        double x = env->getMinX();
        double y = env->getMaxY() - i * YsegLen;
        (*vc)[ipt++] = coord(x, y);
    }
    (*vc)[ipt++] = (*vc)[0];
    auto ring = geomFact->createLinearRing(std::move(vc));
    auto poly = geomFact->createPolygon(std::move(ring));
    return poly;
}

std::unique_ptr<Polygon>
GeometricShapeFactory::createCircle()
{
    std::unique_ptr<Envelope> env(dim.getEnvelope());
    double xRadius = env->getWidth() / 2.0;
    double yRadius = env->getHeight() / 2.0;

    double centreX = env->getMinX() + xRadius;
    double centreY = env->getMinY() + yRadius;
    env.reset();

    auto pts = detail::make_unique<CoordinateSequence>(nPts + 1);
    uint32_t iPt = 0;
    for(uint32_t i = 0; i < nPts; i++) {
        double ang = i * (2 * 3.14159265358979 / nPts);
        double x = xRadius * cos(ang) + centreX;
        double y = yRadius * sin(ang) + centreY;
        (*pts)[iPt++] = coord(x, y);
    }
    (*pts)[iPt++] = (*pts)[0];
    auto ring = geomFact->createLinearRing(std::move(pts));
    auto poly = geomFact->createPolygon(std::move(ring));
    return poly;
}

std::unique_ptr<LineString>
GeometricShapeFactory::createArc(double startAng, double angExtent)
{
    std::unique_ptr<Envelope> env(dim.getEnvelope());
    double xRadius = env->getWidth() / 2.0;
    double yRadius = env->getHeight() / 2.0;

    double centreX = env->getMinX() + xRadius;
    double centreY = env->getMinY() + yRadius;
    env.reset();

    double angSize = angExtent;
    if(angSize <= 0.0 || angSize > 2 * MATH_PI) {
        angSize = 2 * MATH_PI;
    }
    double angInc = angSize / (nPts - 1);

    auto pts = detail::make_unique<CoordinateSequence>(nPts);
    uint32_t iPt = 0;
    for(uint32_t i = 0; i < nPts; i++) {
        double ang = startAng + i * angInc;
        double x = xRadius * cos(ang) + centreX;
        double y = yRadius * sin(ang) + centreY;
        (*pts)[iPt++] = coord(x, y);
    }
    auto line = geomFact->createLineString(std::move(pts));
    return line;
}

std::unique_ptr<Polygon>
GeometricShapeFactory::createArcPolygon(double startAng, double angExtent)
{
    std::unique_ptr<Envelope> env(dim.getEnvelope());
    double xRadius = env->getWidth() / 2.0;
    double yRadius = env->getHeight() / 2.0;

    double centreX = env->getMinX() + xRadius;
    double centreY = env->getMinY() + yRadius;
    env.reset();

    double angSize = angExtent;
    if(angSize <= 0.0 || angSize > 2 * MATH_PI) {
        angSize = 2 * MATH_PI;
    }
    double angInc = angSize / (nPts - 1);

    auto pts = detail::make_unique<CoordinateSequence>(nPts + 2);
    uint32_t iPt = 0;
    (*pts)[iPt++] = coord(centreX, centreY);
    for(uint32_t i = 0; i < nPts; i++) {
        double ang = startAng + i * angInc;
        double x = xRadius * cos(ang) + centreX;
        double y = yRadius * sin(ang) + centreY;
        (*pts)[iPt++] = coord(x, y);
    }
    (*pts)[iPt++] = coord(centreX, centreY);

    auto ring = geomFact->createLinearRing(std::move(pts));

    return geomFact->createPolygon(std::move(ring));
}

GeometricShapeFactory::Dimensions::Dimensions()
    :
    base(CoordinateXY::getNull()),
    centre(CoordinateXY::getNull())
{
}

void
GeometricShapeFactory::Dimensions::setBase(const CoordinateXY& newBase)
{
    base = newBase;
}

void
GeometricShapeFactory::Dimensions::setCentre(const CoordinateXY& newCentre)
{
    centre = newCentre;
}

void
GeometricShapeFactory::Dimensions::setSize(double size)
{
    height = size;
    width = size;
}

void
GeometricShapeFactory::Dimensions::setWidth(double nWidth)
{
    width = nWidth;
}

void
GeometricShapeFactory::Dimensions::setHeight(double nHeight)
{
    height = nHeight;
}

std::unique_ptr<Envelope>
GeometricShapeFactory::Dimensions::getEnvelope() const
{
    if(!base.isNull()) {
        return detail::make_unique<Envelope>(base.x, base.x + width, base.y, base.y + height);
    }
    if(!centre.isNull()) {
        return detail::make_unique<Envelope>(centre.x - width / 2, centre.x + width / 2, centre.y - height / 2, centre.y + height / 2);
    }
    return detail::make_unique<Envelope>(0, width, 0, height);
}

/*protected*/
CoordinateXY
GeometricShapeFactory::coord(double x, double y) const
{
    CoordinateXY ret(x, y);
    precModel->makePrecise(&ret);
    return ret;
}

} // namespace geos.util
} // namespace geos

