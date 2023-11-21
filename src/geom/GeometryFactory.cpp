/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/GeometryFactory.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/util/CoordinateOperation.h>
#include <geos/geom/util/GeometryEditor.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>

#include <cassert>
#include <vector>
#include <typeinfo>
#include <cmath>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif

namespace geos {
namespace geom { // geos::geom

namespace {

class gfCoordinateOperation: public util::CoordinateOperation {
    using CoordinateOperation::edit;
public:
    std::unique_ptr<CoordinateSequence>
    edit(const CoordinateSequence* coordSeq,
         const Geometry*) override
    {
        return detail::make_unique<CoordinateSequence>(*coordSeq);
    }
};

} // anonymous namespace



/*protected*/
GeometryFactory::GeometryFactory()
    :
    SRID(0)
    , _refCount(0), _autoDestroy(false)
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::GeometryFactory()" << std::endl;
    std::cerr << "\tcreate PrecisionModel[" << &precisionModel << "]" << std::endl;
#endif
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create()
{
    return GeometryFactory::Ptr(new GeometryFactory());
}

/*protected*/
GeometryFactory::GeometryFactory(const PrecisionModel* pm)
    :
    SRID(0)
    , _refCount(0), _autoDestroy(false)
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::GeometryFactory(PrecisionModel[" << pm << "])" << std::endl;
#endif
    if(pm) {
        precisionModel = *pm;
    }
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create(const PrecisionModel* pm)
{
    return GeometryFactory::Ptr(
               new GeometryFactory(pm)
           );
}

/*protected*/
GeometryFactory::GeometryFactory(const PrecisionModel* pm, int newSRID)
    : SRID(newSRID)
    , _refCount(0)
    , _autoDestroy(false)
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::GeometryFactory(PrecisionModel[" << pm << "], SRID)" <<
              std::endl;
#endif
    if(pm) {
        precisionModel = *pm;
    }
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create(const PrecisionModel* pm, int newSRID)
{
    return GeometryFactory::Ptr(
               new GeometryFactory(pm, newSRID)
           );
}

/*protected*/
GeometryFactory::GeometryFactory(const GeometryFactory& gf)
    : precisionModel(gf.precisionModel)
    , SRID(gf.SRID)
    , _refCount(0)
    , _autoDestroy(false)
{}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create(const GeometryFactory& gf)
{
    return GeometryFactory::Ptr(
               new GeometryFactory(gf)
           );
}

/*public virtual*/
GeometryFactory::~GeometryFactory()
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::~GeometryFactory()" << std::endl;
#endif
}

/*public*/
std::unique_ptr<Point>
GeometryFactory::createPointFromInternalCoord(const Coordinate* coord,
        const Geometry* exemplar)
{
    assert(coord);
    Coordinate newcoord = *coord;
    exemplar->getPrecisionModel()->makePrecise(&newcoord);
    return exemplar->getFactory()->createPoint(newcoord);
}


/*public*/
std::unique_ptr<Geometry>
GeometryFactory::toGeometry(const Envelope* envelope) const
{
    CoordinateXY coord;

    if(envelope->isNull()) {
        return createPoint();
    }
    if(envelope->getMinX() == envelope->getMaxX() && envelope->getMinY() == envelope->getMaxY()) {
        coord.x = envelope->getMinX();
        coord.y = envelope->getMinY();
        return std::unique_ptr<Geometry>(createPoint(coord));
    }

    auto cl = detail::make_unique<CoordinateSequence>(5u, false, false, false);

    coord.x = envelope->getMinX();
    coord.y = envelope->getMinY();
    cl->setAt(coord, 0);

    coord.x = envelope->getMaxX();
    coord.y = envelope->getMinY();
    cl->setAt(coord, 1);

    coord.x = envelope->getMaxX();
    coord.y = envelope->getMaxY();
    cl->setAt(coord, 2);

    coord.x = envelope->getMinX();
    coord.y = envelope->getMaxY();
    cl->setAt(coord, 3);

    coord.x = envelope->getMinX();
    coord.y = envelope->getMinY();
    cl->setAt(coord, 4);

    return createPolygon(createLinearRing(std::move(cl)));
}

/*public*/
std::unique_ptr<Point>
GeometryFactory::createPoint(std::size_t coordinateDimension) const
{
    CoordinateSequence seq(0u, coordinateDimension);
    return std::unique_ptr<Point>(new Point(std::move(seq), this));
}

/*public*/
std::unique_ptr<Point>
GeometryFactory::createPoint(std::unique_ptr<CoordinateSequence>&& coords) const
{
    if (!coords) {
        return createPoint();
    } else if ((*coords).isNullPoint()) {
        return createPoint((*coords).getDimension());
    }
    return std::unique_ptr<Point>(new Point(std::move(*coords), this));
}

std::unique_ptr<Point>
GeometryFactory::createPoint(const CoordinateXY& coordinate) const
{
    if(coordinate.isNull()) {
        return createPoint(2);
    }
    else {
        return std::unique_ptr<Point>(new Point(coordinate, this));
    }
}

/*public*/
std::unique_ptr<Point>
GeometryFactory::createPoint(const Coordinate& coordinate) const
{
    if(coordinate.isNull()) {
        return createPoint(3);
    }
    else {
        return std::unique_ptr<Point>(new Point(coordinate, this));
    }
}

std::unique_ptr<Point>
GeometryFactory::createPoint(const CoordinateXYM& coordinate) const
{
    if(coordinate.isNull()) {
        return createPoint(4);  // can't do XYM!
    }
    else {
        return std::unique_ptr<Point>(new Point(coordinate, this));
    }
}

std::unique_ptr<Point>
GeometryFactory::createPoint(const CoordinateXYZM& coordinate) const
{
    if(coordinate.isNull()) {
        return createPoint(4);
    }
    else {
        return std::unique_ptr<Point>(new Point(coordinate, this));
    }
}

/*public*/
std::unique_ptr<Point>
GeometryFactory::createPoint(const CoordinateSequence& fromCoords) const
{
    if (fromCoords.isNullPoint()) {
        return createPoint(fromCoords.getDimension());
    }
    CoordinateSequence newCoords(fromCoords);
    return std::unique_ptr<Point>(new Point(std::move(newCoords), this));

}

/*public*/
std::unique_ptr<MultiLineString>
GeometryFactory::createMultiLineString() const
{
    return createMultiLineString(std::vector<std::unique_ptr<Geometry>>());
}

/*public*/
std::unique_ptr<MultiLineString>
GeometryFactory::createMultiLineString(const std::vector<const Geometry*>& fromLines)
const
{
    std::vector<std::unique_ptr<Geometry>> newGeoms(fromLines.size());

    for(std::size_t i = 0; i < fromLines.size(); i++) {
        auto line = dynamic_cast<const LineString*>(fromLines[i]);

        if(!line) {
            throw geos::util::IllegalArgumentException("createMultiLineString called with a vector containing non-LineStrings");
        }

        newGeoms[i].reset(new LineString(*line));
    }

    return createMultiLineString(std::move(newGeoms));
}

std::unique_ptr<MultiLineString>
GeometryFactory::createMultiLineString(std::vector<std::unique_ptr<LineString>> && fromLines) const {
    // Can't use make_unique because constructor is protected
    return std::unique_ptr<MultiLineString>(new MultiLineString(std::move(fromLines), *this));
}

std::unique_ptr<MultiLineString>
GeometryFactory::createMultiLineString(std::vector<std::unique_ptr<Geometry>> && fromLines) const {
    // Can't use make_unique because constructor is protected
    return std::unique_ptr<MultiLineString>(new MultiLineString(std::move(fromLines), *this));
}

/*public*/
std::unique_ptr<GeometryCollection>
GeometryFactory::createGeometryCollection() const
{
    return createGeometryCollection(std::vector<std::unique_ptr<Geometry>>());
}

/*public*/
std::unique_ptr<Geometry>
GeometryFactory::createEmptyGeometry() const
{
    return createGeometryCollection();
}

/*public*/
std::unique_ptr<GeometryCollection>
GeometryFactory::createGeometryCollection(const std::vector<const Geometry*>& fromGeoms) const
{
    std::vector<std::unique_ptr<Geometry>> newGeoms(fromGeoms.size());

    for(std::size_t i = 0; i < fromGeoms.size(); i++) {
        newGeoms[i] = fromGeoms[i]->clone();
    }

    return createGeometryCollection(std::move(newGeoms));
}

/*public*/
std::unique_ptr<MultiPolygon>
GeometryFactory::createMultiPolygon() const
{
    return createMultiPolygon(std::vector<std::unique_ptr<Polygon>>());
}

/*public*/
std::unique_ptr<MultiPolygon>
GeometryFactory::createMultiPolygon(std::vector<std::unique_ptr<Polygon>> && newPolys) const
{
    // Can't use make_unique because constructor is protected
    return std::unique_ptr<MultiPolygon>(new MultiPolygon(std::move(newPolys), *this));
}

std::unique_ptr<MultiPolygon>
GeometryFactory::createMultiPolygon(std::vector<std::unique_ptr<Geometry>> && newPolys) const
{
    // Can't use make_unique because constructor is protected
    return std::unique_ptr<MultiPolygon>(new MultiPolygon(std::move(newPolys), *this));
}

/*public*/
std::unique_ptr<MultiPolygon>
GeometryFactory::createMultiPolygon(const std::vector<const Geometry*>& fromPolys) const
{
    std::vector<std::unique_ptr<Geometry>> newGeoms(fromPolys.size());

    for(std::size_t i = 0; i < fromPolys.size(); i++) {
        newGeoms[i] = fromPolys[i]->clone();
    }

    return createMultiPolygon(std::move(newGeoms));
}

/*public*/
std::unique_ptr<LinearRing>
GeometryFactory::createLinearRing(std::size_t coordinateDimension) const
{
    // Can't use make_unique with protected constructor
    auto cs = detail::make_unique<CoordinateSequence>(0u, coordinateDimension);
    return std::unique_ptr<LinearRing>(new LinearRing(std::move(cs), *this));
}

std::unique_ptr<LinearRing>
GeometryFactory::createLinearRing(CoordinateSequence::Ptr && newCoords) const
{
    // Can't use make_unique with protected constructor
    return std::unique_ptr<LinearRing>(new LinearRing(std::move(newCoords), *this));
}

/*public*/
std::unique_ptr<LinearRing>
GeometryFactory::createLinearRing(const CoordinateSequence& fromCoords) const
{
    return createLinearRing(fromCoords.clone());
}

/*public*/
std::unique_ptr<MultiPoint>
GeometryFactory::createMultiPoint(std::vector<std::unique_ptr<Point>> && newPoints) const
{
    return std::unique_ptr<MultiPoint>(new MultiPoint(std::move(newPoints), *this));
}

std::unique_ptr<MultiPoint>
GeometryFactory::createMultiPoint(std::vector<std::unique_ptr<Geometry>> && newPoints) const
{
    return std::unique_ptr<MultiPoint>(new MultiPoint(std::move(newPoints), *this));
}

/*public*/
std::unique_ptr<MultiPoint>
GeometryFactory::createMultiPoint(const std::vector<const Geometry*>& fromPoints) const
{
    std::vector<std::unique_ptr<Geometry>> newGeoms(fromPoints.size());
    for(std::size_t i = 0; i < fromPoints.size(); i++) {
        newGeoms[i] = fromPoints[i]->clone();
    }

    return createMultiPoint(std::move(newGeoms));
}

/*public*/
std::unique_ptr<MultiPoint>
GeometryFactory::createMultiPoint() const
{
    return std::unique_ptr<MultiPoint>(new MultiPoint(std::vector<std::unique_ptr<Geometry>>(), *this));
}

/*public*/
std::unique_ptr<MultiPoint>
GeometryFactory::createMultiPoint(const CoordinateSequence& fromCoords) const
{
    std::size_t npts = fromCoords.getSize();
    std::vector<std::unique_ptr<Geometry>> pts;
    pts.reserve(npts);

    fromCoords.forEach([&pts, this](const auto& coord) -> void {
        pts.push_back(this->createPoint(coord));
    });

    return createMultiPoint(std::move(pts));
}

/*public*/
std::unique_ptr<Polygon>
GeometryFactory::createPolygon(std::size_t coordinateDimension) const
{
    auto cs = detail::make_unique<CoordinateSequence>(0u, coordinateDimension);
    auto lr = createLinearRing(std::move(cs));
    return createPolygon(std::move(lr));
}

std::unique_ptr<Polygon>
GeometryFactory::createPolygon(std::unique_ptr<LinearRing> && shell)
const
{
    // Can't use make_unique with protected constructor
    return std::unique_ptr<Polygon>(new Polygon(std::move(shell), *this));
}

/*public*/
std::unique_ptr<Polygon>
GeometryFactory::createPolygon(CoordinateSequence && coords)
const
{
    auto cs = detail::make_unique<CoordinateSequence>(std::move(coords));
    std::unique_ptr<geom::LinearRing> lr = createLinearRing(std::move(cs));
    std::unique_ptr<geom::Polygon> ply = createPolygon(std::move(lr));
    return ply;
}


std::unique_ptr<Polygon>
GeometryFactory::createPolygon(std::unique_ptr<LinearRing> && shell, std::vector<std::unique_ptr<LinearRing>> && holes)
const
{
    // Can't use make_unique with protected constructor
    return std::unique_ptr<Polygon>(new Polygon(std::move(shell), std::move(holes), *this));
}

/*public*/
Polygon*
GeometryFactory::createPolygon(const LinearRing& shell, const std::vector<LinearRing*>& holes)
const
{
    std::unique_ptr<LinearRing> newRing(new LinearRing(shell));

    std::vector<std::unique_ptr<LinearRing>> newHoles(holes.size());

    for(std::size_t i = 0; i < holes.size(); i++) {
        newHoles[i].reset(new LinearRing(*holes[i]));
    }

    return new Polygon(std::move(newRing), std::move(newHoles), *this);
}

/*public*/
std::unique_ptr<LineString>
GeometryFactory::createLineString(std::size_t coordinateDimension) const
{
    auto cs = detail::make_unique<CoordinateSequence>(0u, coordinateDimension);
    return createLineString(std::move(cs));
}

/*public*/
std::unique_ptr<LineString>
GeometryFactory::createLineString(const LineString& ls) const
{
    // Can't use make_unique with protected constructor
    return std::unique_ptr<LineString>(new LineString(ls));
}

/*public*/
std::unique_ptr<LineString>
GeometryFactory::createLineString(CoordinateSequence::Ptr && newCoords)
const
{
    if (!newCoords)
        return createLineString();
    // Can't use make_unique with protected constructor
    return std::unique_ptr<LineString>(new LineString(std::move(newCoords), *this));
}

/*public*/
std::unique_ptr<LineString>
GeometryFactory::createLineString(const CoordinateSequence& fromCoords)
const
{
    // Can't use make_unique with protected constructor
    return std::unique_ptr<LineString>(new LineString(fromCoords.clone(), *this));
}

/*public*/
std::unique_ptr<Geometry>
GeometryFactory::createEmpty(int dimension) const
{
    switch (dimension) {
        case -1: return createGeometryCollection();
        case 0: return createPoint();
        case 1: return createLineString();
        case 2: return createPolygon();
        default:
            throw geos::util::IllegalArgumentException("Invalid dimension");
    }
}

/*public*/
std::unique_ptr<Geometry>
GeometryFactory::createEmpty(GeometryTypeId typeId) const
{
    switch (typeId) {
        case GEOS_POINT: return createPoint();
        case GEOS_LINESTRING: return createLineString();
        case GEOS_POLYGON: return createPolygon();
        case GEOS_MULTIPOINT: return createMultiPoint();
        case GEOS_MULTILINESTRING: return createMultiLineString();
        case GEOS_MULTIPOLYGON: return createMultiPolygon();
        case GEOS_GEOMETRYCOLLECTION: return createGeometryCollection();
        default:
            throw geos::util::IllegalArgumentException("Invalid GeometryTypeId");
    }
}

/*public*/
std::unique_ptr<Geometry>
GeometryFactory::createMulti(std::unique_ptr<Geometry> && geom) const
{
    GeometryTypeId typeId = geom->getGeometryTypeId();

    // Already a collection? Done!
    if (geom->isCollection())
        return std::move(geom);

    if (geom->isEmpty()) {
        return geom->getFactory()->createEmpty(Geometry::multiTypeId(typeId));
    }

    std::vector<std::unique_ptr<Geometry>> subgeoms;
    const GeometryFactory* gf = geom->getFactory();
    subgeoms.push_back(std::move(geom));
    switch (typeId) {
        case GEOS_POINT:
            return gf->createMultiPoint(std::move(subgeoms));
        case GEOS_LINESTRING:
            return gf->createMultiLineString(std::move(subgeoms));
        case GEOS_POLYGON:
            return gf->createMultiPolygon(std::move(subgeoms));
        default:
            throw geos::util::IllegalArgumentException("Unsupported GeometryTypeId");
    }
}

template<typename T>
GeometryTypeId commonType(const T& geoms) {
    if (geoms.empty()) {
        return GEOS_GEOMETRYCOLLECTION;
    }

    if (geoms.size() == 1) {
        return geoms[0]->getGeometryTypeId();
    }

    GeometryTypeId type = geoms[0]->getGeometryTypeId();
    for (std::size_t i = 1; i < geoms.size(); i++) {
        if (geoms[i]->getGeometryTypeId() != type) {
            return GEOS_GEOMETRYCOLLECTION;
        }
    }

    switch(geoms[0]->getGeometryTypeId()) {
        case GEOS_POINT: return GEOS_MULTIPOINT;
        case GEOS_LINEARRING:
        case GEOS_LINESTRING: return GEOS_MULTILINESTRING;
        case GEOS_POLYGON: return GEOS_MULTIPOLYGON;
        default: return GEOS_GEOMETRYCOLLECTION;
    }
}

std::unique_ptr<Geometry>
GeometryFactory::buildGeometry(std::vector<std::unique_ptr<Geometry>> && geoms) const
{
    if (geoms.empty()) {
        return createGeometryCollection();
    }

    if (geoms.size() == 1) {
        return std::move(geoms[0]);
    }

    auto resultType = commonType(geoms);

    switch(resultType) {
        case GEOS_MULTIPOINT: return createMultiPoint(std::move(geoms));
        case GEOS_MULTILINESTRING: return createMultiLineString(std::move(geoms));
        case GEOS_MULTIPOLYGON: return createMultiPolygon(std::move(geoms));
        default: return createGeometryCollection(std::move(geoms));
    }
}

std::unique_ptr<Geometry>
GeometryFactory::buildGeometry(std::vector<std::unique_ptr<Point>> && geoms) const
{
    if (geoms.empty()) {
        return createGeometryCollection();
    }

    if (geoms.size() == 1) {
        return std::move(geoms[0]);
    }

    return createMultiPoint(std::move(geoms));
}

std::unique_ptr<Geometry>
GeometryFactory::buildGeometry(std::vector<std::unique_ptr<LineString>> && geoms) const
{
    if (geoms.empty()) {
        return createGeometryCollection();
    }

    if (geoms.size() == 1) {
        return std::move(geoms[0]);
    }

    return createMultiLineString(std::move(geoms));
}

std::unique_ptr<Geometry>
GeometryFactory::buildGeometry(std::vector<std::unique_ptr<Polygon>> && geoms) const
{
    if (geoms.empty()) {
        return createGeometryCollection();
    }

    if (geoms.size() == 1) {
        return std::move(geoms[0]);
    }

    return createMultiPolygon(std::move(geoms));
}

/*public*/
std::unique_ptr<Geometry>
GeometryFactory::buildGeometry(const std::vector<const Geometry*>& fromGeoms) const
{
    if(fromGeoms.empty()) {
        return createGeometryCollection();
    }

    if(fromGeoms.size() == 1) {
        return fromGeoms[0]->clone();
    }

    auto resultType = commonType(fromGeoms);

    switch(resultType) {
        case GEOS_MULTIPOINT: return createMultiPoint(fromGeoms);
        case GEOS_MULTILINESTRING: return createMultiLineString(fromGeoms);
        case GEOS_MULTIPOLYGON: return createMultiPolygon(fromGeoms);
        default: return createGeometryCollection(fromGeoms);
    }
}

/*public*/
std::unique_ptr<Geometry>
GeometryFactory::createGeometry(const Geometry* g) const
{
    // could this be cached to make this more efficient? Or maybe it isn't enough overhead to bother
    //return g->clone(); <-- a simple clone() wouldn't change the factory to `this`
    util::GeometryEditor editor(this);
    gfCoordinateOperation coordOp;
    return editor.edit(g, &coordOp);
}

/*public*/
void
GeometryFactory::destroyGeometry(Geometry* g) const
{
    delete g;
}

/*public static*/
const GeometryFactory*
GeometryFactory::getDefaultInstance()
{
    static GeometryFactory defInstance;
    return &defInstance;
}

/*private*/
void
GeometryFactory::addRef() const
{
    ++_refCount;
}

/*private*/
void
GeometryFactory::dropRef() const
{
    if(! --_refCount) {
        if(_autoDestroy) {
            delete this;
        }
    }
}

void
GeometryFactory::destroy()
{
    assert(!_autoDestroy); // don't call me twice !
    _autoDestroy = true;
    if(! _refCount) {
        delete this;
    }
}

} // namespace geos::geom
} // namespace geos
