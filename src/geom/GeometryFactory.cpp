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
#include <geos/geom/CoordinateArraySequenceFactory.h>
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
#include <geos/util.h>

#include <cassert>
#include <vector>
#include <typeinfo>
#include <cmath>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

#ifndef GEOS_INLINE
# include <geos/geom/GeometryFactory.inl>
#endif

using namespace std;

namespace geos {
namespace geom { // geos::geom

namespace {

class gfCoordinateOperation: public util::CoordinateOperation {
    using CoordinateOperation::edit;
    const CoordinateSequenceFactory* _gsf;
public:
    gfCoordinateOperation(const CoordinateSequenceFactory* gsf)
        : _gsf(gsf)
    {}
    std::unique_ptr<CoordinateSequence>
    edit(const CoordinateSequence* coordSeq,
         const Geometry*) override
    {
        return _gsf->create(*coordSeq);
    }
};

} // anonymous namespace



/*protected*/
GeometryFactory::GeometryFactory()
    :
    SRID(0),
    coordinateListFactory(CoordinateArraySequenceFactory::instance())
    , _refCount(0), _autoDestroy(false)
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::GeometryFactory()" << std::endl;
    std::cerr << "\tcreate PrecisionModel[" << precisionModel << "]" << std::endl;
#endif
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create()
{
    return GeometryFactory::Ptr(new GeometryFactory());
}

/*protected*/
GeometryFactory::GeometryFactory(const PrecisionModel* pm, int newSRID,
                                 CoordinateSequenceFactory* nCoordinateSequenceFactory)
    :
    SRID(newSRID)
    , _refCount(0), _autoDestroy(false)
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::GeometryFactory(PrecisionModel[" << pm << "], SRID)" <<
              std::endl;
#endif
    if(pm) {
        precisionModel = *pm;
    }

    if(! nCoordinateSequenceFactory) {
        coordinateListFactory = CoordinateArraySequenceFactory::instance();
    }
    else {
        coordinateListFactory = nCoordinateSequenceFactory;
    }
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create(const PrecisionModel* pm, int newSRID,
                        CoordinateSequenceFactory* nCoordinateSequenceFactory)
{
    return GeometryFactory::Ptr(
               new GeometryFactory(pm, newSRID, nCoordinateSequenceFactory)
           );
}

/*protected*/
GeometryFactory::GeometryFactory(
    CoordinateSequenceFactory* nCoordinateSequenceFactory)
    :
    SRID(0)
    , _refCount(0), _autoDestroy(false)
{
#if GEOS_DEBUG
    std::cerr << "GEOS_DEBUG: GeometryFactory[" << this << "]::GeometryFactory(CoordinateSequenceFactory[" <<
              nCoordinateSequenceFactory << "])" << std::endl;
#endif
    if(! nCoordinateSequenceFactory) {
        coordinateListFactory = CoordinateArraySequenceFactory::instance();
    }
    else {
        coordinateListFactory = nCoordinateSequenceFactory;
    }
}

/*public static*/
GeometryFactory::Ptr
GeometryFactory::create(
    CoordinateSequenceFactory* nCoordinateSequenceFactory)
{
    return GeometryFactory::Ptr(
               new GeometryFactory(nCoordinateSequenceFactory)
           );
}

/*protected*/
GeometryFactory::GeometryFactory(const PrecisionModel* pm)
    :
    SRID(0),
    coordinateListFactory(CoordinateArraySequenceFactory::instance())
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
    :
    SRID(newSRID),
    coordinateListFactory(CoordinateArraySequenceFactory::instance())
    , _refCount(0), _autoDestroy(false)
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
GeometryFactory::GeometryFactory(const GeometryFactory& gf) :
precisionModel(gf.precisionModel),
SRID(gf.SRID),
coordinateListFactory(gf.coordinateListFactory),
_refCount(0),
_autoDestroy(false)
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
Point*
GeometryFactory::createPointFromInternalCoord(const Coordinate* coord,
        const Geometry* exemplar) const
{
    assert(coord);
    Coordinate newcoord = *coord;
    exemplar->getPrecisionModel()->makePrecise(&newcoord);
    return exemplar->getFactory()->createPoint(newcoord);
}


/*public*/
Geometry*
GeometryFactory::toGeometry(const Envelope* envelope) const
{
    Coordinate coord;

    if(envelope->isNull()) {
        return createPoint();
    }
    if(envelope->getMinX() == envelope->getMaxX() && envelope->getMinY() == envelope->getMaxY()) {
        coord.x = envelope->getMinX();
        coord.y = envelope->getMinY();
        return createPoint(coord);
    }
    auto cl = CoordinateArraySequenceFactory::instance()->
                             create((size_t) 5, 2);

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

    Polygon* p = createPolygon(createLinearRing(cl.release()), nullptr);
    return p;
}

/*public*/
const PrecisionModel*
GeometryFactory::getPrecisionModel() const
{
    return &precisionModel;
}

/*public*/
Point*
GeometryFactory::createPoint() const
{
    return new Point(nullptr, this);
}

/*public*/
Point*
GeometryFactory::createPoint(const Coordinate& coordinate) const
{
    if(coordinate.isNull()) {
        return createPoint();
    }
    else {
        return new Point(coordinate, this);
    }
}

/*public*/
Point*
GeometryFactory::createPoint(CoordinateSequence* newCoords) const
{
    return new Point(newCoords, this);
}

/*public*/
Point*
GeometryFactory::createPoint(const CoordinateSequence& fromCoords) const
{
    auto newCoords = fromCoords.clone();
    return new Point(newCoords.release(), this);

}

/*public*/
MultiLineString*
GeometryFactory::createMultiLineString() const
{
    return new MultiLineString(nullptr, this);
}

/*public*/
MultiLineString*
GeometryFactory::createMultiLineString(vector<Geometry*>* newLines)
const
{
    return new MultiLineString(newLines, this);
}

/*public*/
MultiLineString*
GeometryFactory::createMultiLineString(const std::vector<const Geometry*>& fromLines)
const
{
    std::vector<std::unique_ptr<Geometry>> newGeoms(fromLines.size());

    for(size_t i = 0; i < fromLines.size(); i++) {
        auto line = dynamic_cast<const LineString*>(fromLines[i]);

        if(!line) {
            throw geos::util::IllegalArgumentException("createMultiLineString called with a vector containing non-LineStrings");
        }

        newGeoms[i].reset(new LineString(*line));
    }

    return new MultiLineString(std::move(newGeoms), *this);
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
GeometryCollection*
GeometryFactory::createGeometryCollection() const
{
    return new GeometryCollection(nullptr, this);
}

/*public*/
Geometry*
GeometryFactory::createEmptyGeometry() const
{
    return new GeometryCollection(nullptr, this);
}

/*public*/
GeometryCollection*
GeometryFactory::createGeometryCollection(vector<Geometry*>* newGeoms) const
{
    return new GeometryCollection(newGeoms, this);
}

std::unique_ptr<GeometryCollection>
GeometryFactory::createGeometryCollection(std::vector<std::unique_ptr<geos::geom::Geometry>> && newGeoms) const {
    // Can't use make_unique because constructor is protected
    return std::unique_ptr<GeometryCollection>(new GeometryCollection(std::move(newGeoms), *this));
}

/*public*/
GeometryCollection*
GeometryFactory::createGeometryCollection(const std::vector<const Geometry*>& fromGeoms) const
{
    std::vector<std::unique_ptr<Geometry>> newGeoms(fromGeoms.size());

    for(size_t i = 0; i < fromGeoms.size(); i++) {
        newGeoms[i] = fromGeoms[i]->clone();
    }

    return new GeometryCollection(std::move(newGeoms), *this);
}

/*public*/
MultiPolygon*
GeometryFactory::createMultiPolygon() const
{
    return new MultiPolygon(nullptr, this);
}

/*public*/
MultiPolygon*
GeometryFactory::createMultiPolygon(vector<Geometry*>* newPolys) const
{
    return new MultiPolygon(newPolys, this);
}

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
MultiPolygon*
GeometryFactory::createMultiPolygon(const std::vector<const Geometry*>& fromPolys) const
{
    std::vector<std::unique_ptr<Geometry>> newGeoms(fromPolys.size());

    for(size_t i = 0; i < fromPolys.size(); i++) {
        newGeoms[i] = fromPolys[i]->clone();
    }

    return new MultiPolygon(std::move(newGeoms), *this);
}

/*public*/
LinearRing*
GeometryFactory::createLinearRing() const
{
    return new LinearRing(nullptr, this);
}

/*public*/
LinearRing*
GeometryFactory::createLinearRing(CoordinateSequence* newCoords) const
{
    return new LinearRing(newCoords, this);
}

std::unique_ptr<LinearRing>
GeometryFactory::createLinearRing(CoordinateSequence::Ptr && newCoords) const
{
    // Can't use make_unique with protected constructor
    return std::unique_ptr<LinearRing>(new LinearRing(std::move(newCoords), *this));
}

/*public*/
LinearRing*
GeometryFactory::createLinearRing(const CoordinateSequence& fromCoords) const
{
    auto newCoords = fromCoords.clone();
    LinearRing* g = nullptr;
    // construction failure will delete newCoords
    g = new LinearRing(newCoords.release(), this);
    return g;
}

/*public*/
MultiPoint*
GeometryFactory::createMultiPoint(vector<Geometry*>* newPoints) const
{
    return new MultiPoint(newPoints, this);
}

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
MultiPoint*
GeometryFactory::createMultiPoint(const vector<const Geometry*>& fromPoints) const
{
    std::vector<std::unique_ptr<Geometry>> newGeoms(fromPoints.size());
    for(size_t i = 0; i < fromPoints.size(); i++) {
        newGeoms[i] = fromPoints[i]->clone();
    }

    return new MultiPoint(std::move(newGeoms), *this);
}

/*public*/
MultiPoint*
GeometryFactory::createMultiPoint() const
{
    return new MultiPoint(nullptr, this);
}

/*public*/
MultiPoint*
GeometryFactory::createMultiPoint(const CoordinateSequence& fromCoords) const
{
    size_t npts = fromCoords.getSize();
    vector<std::unique_ptr<Geometry>> pts(npts);

    for(size_t i = 0; i < npts; ++i) {
        pts[i].reset(createPoint(fromCoords.getAt(i)));
    }

    return new MultiPoint(std::move(pts), *this);
}

/*public*/
MultiPoint*
GeometryFactory::createMultiPoint(const std::vector<Coordinate>& fromCoords) const
{
    size_t npts = fromCoords.size();
    std::vector<std::unique_ptr<Geometry>> pts(npts);

    for(size_t i = 0; i < npts; ++i) {
        pts[i].reset(createPoint(fromCoords[i]));
    }

    return new MultiPoint(std::move(pts), *this);
}

/*public*/
Polygon*
GeometryFactory::createPolygon() const
{
    return new Polygon(nullptr, nullptr, this);
}

/*public*/
Polygon*
GeometryFactory::createPolygon(LinearRing* shell, vector<LinearRing*>* holes)
const
{
    return new Polygon(shell, holes, this);
}

std::unique_ptr<Polygon>
GeometryFactory::createPolygon(std::unique_ptr<LinearRing> && shell)
const
{
    // Can't use make_unique with protected constructor
    return std::unique_ptr<Polygon>(new Polygon(std::move(shell), *this));
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

    for(size_t i = 0; i < holes.size(); i++) {
        newHoles[i].reset(new LinearRing(*holes[i]));
    }

    return new Polygon(std::move(newRing), std::move(newHoles), *this);
}

/*public*/
LineString*
GeometryFactory::createLineString() const
{
    return new LineString(nullptr, this);
}

/*public*/
std::unique_ptr<LineString>
GeometryFactory::createLineString(const LineString& ls) const
{
    // Can't use make_unique with protected constructor
    return std::unique_ptr<LineString>(new LineString(ls));
}

/*public*/
LineString*
GeometryFactory::createLineString(CoordinateSequence* newCoords)
const
{
    return new LineString(newCoords, this);
}

/*public*/
std::unique_ptr<LineString>
GeometryFactory::createLineString(CoordinateSequence::Ptr && newCoords)
const
{
    // Can't use make_unique with protected constructor
    return std::unique_ptr<LineString>(new LineString(std::move(newCoords), *this));
}

/*public*/
LineString*
GeometryFactory::createLineString(const CoordinateSequence& fromCoords)
const
{
    auto newCoords = fromCoords.clone();
    LineString* g = nullptr;
    // construction failure will delete newCoords
    g = new LineString(newCoords.release(), this);
    return g;
}

/*public*/
Geometry*
GeometryFactory::buildGeometry(vector<Geometry*>* newGeoms) const
{
    if(newGeoms->empty()) {
        // we do not need the vector anymore
        delete newGeoms;
        return createGeometryCollection();
    }

    bool isHeterogeneous = false;
    bool hasGeometryCollection = false;
    GeometryTypeId type = (*newGeoms)[0]->getGeometryTypeId();

    for(Geometry* gp : *newGeoms) {
        GeometryTypeId geometryType = gp->getGeometryTypeId();
        if(type != geometryType) {
            isHeterogeneous = true;
        }
        if(geometryType == GEOS_GEOMETRYCOLLECTION) {
            hasGeometryCollection = true;
        }
    }

    if(isHeterogeneous || hasGeometryCollection) {
        return createGeometryCollection(newGeoms);
    }

    // At this point we know the collection is not hetereogenous.
    bool isCollection = newGeoms->size() > 1;
    if(isCollection) {
        if(type == GEOS_POLYGON) {
            return createMultiPolygon(newGeoms);
        }
        else if(type == GEOS_LINESTRING) {
            return createMultiLineString(newGeoms);
        }
        else if(type == GEOS_LINEARRING) {
            return createMultiLineString(newGeoms);
        }
        else if(type == GEOS_POINT) {
            return createMultiPoint(newGeoms);
        }
        else {
            return createGeometryCollection(newGeoms);
        }
    }

    // since this is not a collection we can delete vector
    Geometry* geom0 = (*newGeoms)[0];
    delete newGeoms;
    return geom0;
}

/*public*/
Geometry*
GeometryFactory::buildGeometry(const vector<const Geometry*>& fromGeoms) const
{
    size_t geomsSize = fromGeoms.size();
    if(geomsSize == 0) {
        return createGeometryCollection();
    }

    if(geomsSize == 1) {
        return fromGeoms[0]->clone().release();
    }

    bool isHeterogeneous = false;
    GeometryTypeId type = fromGeoms[0]->getGeometryTypeId();

    for(const Geometry* gp : fromGeoms) {
        GeometryTypeId geometryType = gp->getGeometryTypeId();
        if(type != geometryType) {
            isHeterogeneous = true;
        }
    }

    if(isHeterogeneous) {
        return createGeometryCollection(fromGeoms);
    }

    if(type == GEOS_POLYGON) {
        return createMultiPolygon(fromGeoms);
    }
    else if(type == GEOS_LINESTRING) {
        return createMultiLineString(fromGeoms);
    }
    else if(type == GEOS_LINEARRING) {
        return createMultiLineString(fromGeoms);
    }
    else if(type == GEOS_POINT) {
        return createMultiPoint(fromGeoms);
    }

    throw geos::util::GEOSException("GeometryFactory::buildGeometry encountered an unknown geometry type!");
}

/*public*/
Geometry*
GeometryFactory::createGeometry(const Geometry* g) const
{
    // could this be cached to make this more efficient? Or maybe it isn't enough overhead to bother
    //return g->clone();
    util::GeometryEditor editor(this);
    gfCoordinateOperation coordOp(coordinateListFactory);
    return editor.edit(g, &coordOp).release();
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
    static GeometryFactory* defInstance = new GeometryFactory();
    return defInstance;
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
