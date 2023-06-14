/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: geom/Geometry.java rev. 1.112
 *
 **********************************************************************/

#include <geos/geom/HeuristicOverlay.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Point.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/algorithm/Centroid.h>
#include <geos/algorithm/InteriorPointPoint.h>
#include <geos/algorithm/InteriorPointLine.h>
#include <geos/algorithm/InteriorPointArea.h>
#include <geos/algorithm/ConvexHull.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/operation/intersection/Rectangle.h>
#include <geos/operation/intersection/RectangleIntersection.h>
#include <geos/operation/predicate/RectangleContains.h>
#include <geos/operation/predicate/RectangleIntersects.h>
#include <geos/operation/relate/RelateOp.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/operation/union/UnaryUnionOp.h>
#include <geos/operation/buffer/BufferOp.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/operation/valid/IsSimpleOp.h>
#include <geos/operation/overlayng/OverlayNGRobust.h>
#include <geos/operation/overlayng/OverlayUtil.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTWriter.h>
#include <geos/version.h>

#include <algorithm>
#include <string>
#include <typeinfo>
#include <vector>
#include <cassert>
#include <memory>

#ifdef _MSC_VER
#  ifdef MSVC_USE_VLD
#    include <vld.h>
#  endif
#endif

#define SHORTCIRCUIT_PREDICATES 1
//#define USE_RECTANGLE_INTERSECTION 1

using namespace geos::algorithm;
using namespace geos::operation::valid;
using namespace geos::operation::relate;
using namespace geos::operation::buffer;
using namespace geos::operation::distance;
using namespace geos::operation;
using geos::operation::overlayng::OverlayNG;


namespace geos {
namespace geom { // geos::geom


/*
 * Return current GEOS version
 */
std::string
geosversion()
{
    return GEOS_VERSION;
}

/*
 * Return the version of JTS this GEOS
 * release has been ported from.
 */
std::string
jtsport()
{
    return GEOS_JTS_PORT;
}

Geometry::GeometryChangedFilter Geometry::geometryChangedFilter;

Geometry::Geometry(const GeometryFactory* newFactory)
    :
    _factory(newFactory),
    _userData(nullptr)
{
    if(_factory == nullptr) {
        _factory = GeometryFactory::getDefaultInstance();
    }
    SRID = _factory->getSRID();
    _factory->addRef();
}

Geometry::Geometry(const Geometry& geom)
    :
    SRID(geom.getSRID()),
    _factory(geom._factory),
    _userData(nullptr)
{
    _factory->addRef();
}

bool
Geometry::hasNullElements(const CoordinateSequence* list)
{
    std::size_t npts = list->getSize();
    for(std::size_t i = 0; i < npts; ++i) {
        if(list->getAt(i).isNull()) {
            return true;
        }
    }
    return false;
}

/* public */
bool
Geometry::isWithinDistance(const Geometry* geom, double cDistance) const
{
    return DistanceOp::isWithinDistance(*this, *geom, cDistance);
}

/*public*/
std::unique_ptr<Point>
Geometry::getCentroid() const
{
    Coordinate centPt;
    if(! getCentroid(centPt)) {
        return getFactory()->createPoint(getCoordinateDimension());
    }

    // We don't use createPointFromInternalCoord here
    // because ::getCentroid() takes care about rounding
    return std::unique_ptr<Point>(getFactory()->createPoint(centPt));
}

/* public */
bool
Geometry::isMixedDimension() const
{
    Dimension::DimensionType baseDim = Dimension::DONTCARE;
    return isMixedDimension(&baseDim);
}

/* public */
bool
Geometry::isMixedDimension(Dimension::DimensionType* baseDim) const
{
    if (isCollection()) {
        for (std::size_t i = 0; i < getNumGeometries(); i++) {
            if (getGeometryN(i)->isMixedDimension(baseDim))
                return true;
        }
        return false;
    }
    else {
        if (*baseDim == Dimension::DONTCARE) {
            *baseDim = getDimension();
            return false;
        }
        return *baseDim != getDimension();
    }
}

/*public*/
bool
Geometry::getCentroid(CoordinateXY& ret) const
{
    if(isEmpty()) {
        return false;
    }
    if(! Centroid::getCentroid(*this, ret)) {
        return false;
    }
    getPrecisionModel()->makePrecise(ret); // not in JTS
    return true;
}

std::unique_ptr<Point>
Geometry::getInteriorPoint() const
{
    Coordinate interiorPt;
    int dim = getDimension();
    if(dim == 0) {
        InteriorPointPoint intPt(this);
        if(! intPt.getInteriorPoint(interiorPt)) {
            return getFactory()->createPoint(getCoordinateDimension()); // POINT EMPTY
        }
    }
    else if(dim == 1) {
        InteriorPointLine intPt(this);
        if(! intPt.getInteriorPoint(interiorPt)) {
            return getFactory()->createPoint(getCoordinateDimension()); // POINT EMPTY
        }
    }
    else {
        InteriorPointArea intPt(this);
        if(! intPt.getInteriorPoint(interiorPt)) {
            return getFactory()->createPoint(getCoordinateDimension()); // POINT EMPTY
        }
    }
    std::unique_ptr<Point> p(getFactory()->createPointFromInternalCoord(&interiorPt, this));
    return p;
}

/**
 * Notifies this Geometry that its Coordinates have been changed by an external
 * party (using a CoordinateFilter, for example). The Geometry will flush
 * and/or update any information it has cached (such as its {@link Envelope} ).
 */
void
Geometry::geometryChanged()
{
    apply_rw(&geometryChangedFilter);
}

bool
Geometry::isValid() const
{
    return IsValidOp(this).isValid();
}

std::unique_ptr<Geometry>
Geometry::getEnvelope() const
{
    return std::unique_ptr<Geometry>(getFactory()->toGeometry(getEnvelopeInternal()));
}

bool
Geometry::disjoint(const Geometry* g) const
{
    return !intersects(g);
}

bool
Geometry::touches(const Geometry* g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
    // short-circuit test
    if(! getEnvelopeInternal()->intersects(g->getEnvelopeInternal())) {
        return false;
    }
#endif
    std::unique_ptr<IntersectionMatrix> im(relate(g));
    bool res = im->isTouches(getDimension(), g->getDimension());
    return res;
}

bool
Geometry::intersects(const Geometry* g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
    // short-circuit test
    if(! getEnvelopeInternal()->intersects(g->getEnvelopeInternal())) {
        return false;
    }
#endif

    /*
     * TODO: (MD) Add optimizations:
     *
     * - for P-A case:
     * If P is in env(A), test for point-in-poly
     *
     * - for A-A case:
     * If env(A1).overlaps(env(A2))
     * test for overlaps via point-in-poly first (both ways)
     * Possibly optimize selection of point to test by finding point of A1
     * closest to centre of env(A2).
     * (Is there a test where we shouldn't bother - e.g. if env A
     * is much smaller than env B, maybe there's no point in testing
     * pt(B) in env(A)?
     */

    // optimization for rectangle arguments
    if(isRectangle()) {
        const Polygon* p = detail::down_cast<const Polygon*>(this);
        return predicate::RectangleIntersects::intersects(*p, *g);
    }
    if(g->isRectangle()) {
        const Polygon* p = detail::down_cast<const Polygon*>(g);
        return predicate::RectangleIntersects::intersects(*p, *this);
    }

    if (getGeometryTypeId() == GEOS_GEOMETRYCOLLECTION) {
        auto im = relate(g);
        bool res = im->isIntersects();
        return res;
    } else {
        return prep::PreparedGeometryFactory::prepare(this)->intersects(g);
    }
}

/*public*/
bool
Geometry::covers(const Geometry* g) const
{
    // optimization - lower dimension cannot cover areas
    if(g->getDimension() == 2 && getDimension() < 2) {
        return false;
    }

    // optimization - P cannot cover a non-zero-length L
    // Note that a point can cover a zero-length lineal geometry
    if(g->getDimension() == 1 && getDimension() < 1 && g->getLength() > 0.0) {
        return false;
    }

#ifdef SHORTCIRCUIT_PREDICATES
    // short-circuit test
    if(! getEnvelopeInternal()->covers(g->getEnvelopeInternal())) {
        return false;
    }
#endif

    // optimization for rectangle arguments
    if(isRectangle()) {
        // since we have already tested that the test envelope
        // is covered
        return true;
    }

    std::unique_ptr<IntersectionMatrix> im(relate(g));
    return im->isCovers();
}


bool
Geometry::crosses(const Geometry* g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
    // short-circuit test
    if(! getEnvelopeInternal()->intersects(g->getEnvelopeInternal())) {
        return false;
    }
#endif
    std::unique_ptr<IntersectionMatrix> im(relate(g));
    bool res = im->isCrosses(getDimension(), g->getDimension());
    return res;
}

bool
Geometry::within(const Geometry* g) const
{
    return g->contains(this);
}

bool
Geometry::contains(const Geometry* g) const
{
    // optimization - lower dimension cannot contain areas
    if(g->getDimension() == 2 && getDimension() < 2) {
        return false;
    }

    // optimization - P cannot contain a non-zero-length L
    // Note that a point can contain a zero-length lineal geometry,
    // since the line has no boundary due to Mod-2 Boundary Rule
    if(g->getDimension() == 1 && getDimension() < 1 && g->getLength() > 0.0) {
        return false;
    }

#ifdef SHORTCIRCUIT_PREDICATES
    // short-circuit test
    if(! getEnvelopeInternal()->contains(g->getEnvelopeInternal())) {
        return false;
    }
#endif

    // optimization for rectangle arguments
    if(isRectangle()) {
        const Polygon* p = detail::down_cast<const Polygon*>(this);
        return predicate::RectangleContains::contains(*p, *g);
    }
    // Incorrect: contains is not commutative
    //if (g->isRectangle()) {
    //	return predicate::RectangleContains::contains((const Polygon&)*g, *this);
    //}

    std::unique_ptr<IntersectionMatrix> im(relate(g));
    bool res = im->isContains();
    return res;
}

bool
Geometry::overlaps(const Geometry* g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
    // short-circuit test
    if(! getEnvelopeInternal()->intersects(g->getEnvelopeInternal())) {
        return false;
    }
#endif
    std::unique_ptr<IntersectionMatrix> im(relate(g));
    bool res = im->isOverlaps(getDimension(), g->getDimension());
    return res;
}

bool
Geometry::relate(const Geometry* g, const std::string& intersectionPattern) const
{
    std::unique_ptr<IntersectionMatrix> im(relate(g));
    bool res = im->matches(intersectionPattern);
    return res;
}

bool
Geometry::equals(const Geometry* g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
    // short-circuit test
    if(! getEnvelopeInternal()->equals(g->getEnvelopeInternal())) {
        return false;
    }
#endif

    if(isEmpty()) {
        return g->isEmpty();
    }
    else if(g->isEmpty()) {
        return isEmpty();
    }

    std::unique_ptr<IntersectionMatrix> im(relate(g));
    bool res = im->isEquals(getDimension(), g->getDimension());
    return res;
}

std::unique_ptr<IntersectionMatrix>
Geometry::relate(const Geometry* other) const
{
    return RelateOp::relate(this, other);
}

std::unique_ptr<IntersectionMatrix>
Geometry::relate(const Geometry& other) const
{
    return relate(&other);
}

std::string
Geometry::toString() const
{
    return toText();
}

std::ostream&
operator<< (std::ostream& os, const Geometry& geom)
{
    io::WKBWriter writer;
    writer.writeHEX(geom, os);
    return os;
}

std::string
Geometry::toText() const
{
    io::WKTWriter writer;
    // To enable "GEOS<3.12" outputs, uncomment the following:
    // writer.setTrim(false);
    // writer.setOutputDimension(2);
    return writer.write(this);
}

std::unique_ptr<Geometry>
Geometry::buffer(double p_distance) const
{
    return std::unique_ptr<Geometry>(BufferOp::bufferOp(this, p_distance));
}

std::unique_ptr<Geometry>
Geometry::buffer(double p_distance, int quadrantSegments) const
{
    return std::unique_ptr<Geometry>(BufferOp::bufferOp(this, p_distance, quadrantSegments));
}

std::unique_ptr<Geometry>
Geometry::buffer(double p_distance, int quadrantSegments, int endCapStyle) const
{
    return std::unique_ptr<Geometry>(BufferOp::bufferOp(this, p_distance, quadrantSegments, endCapStyle));
}

std::unique_ptr<Geometry>
Geometry::convexHull() const
{
    return ConvexHull(this).getConvexHull();
}

std::unique_ptr<Geometry>
Geometry::intersection(const Geometry* other) const
{
    /*
     * TODO: MD - add optimization for P-A case using Point-In-Polygon
     */

#ifdef USE_RECTANGLE_INTERSECTION
    // optimization for rectangle arguments
    using operation::intersection::Rectangle;
    using operation::intersection::RectangleIntersection;
    if(isRectangle()) {
        const Envelope* env = getEnvelopeInternal();
        Rectangle rect(env->getMinX(), env->getMinY(),
                       env->getMaxX(), env->getMaxY());
        return RectangleIntersection::clip(*other, rect).release();
    }
    if(other->isRectangle()) {
        const Envelope* env = other->getEnvelopeInternal();
        Rectangle rect(env->getMinX(), env->getMinY(),
                       env->getMaxX(), env->getMaxY());
        return RectangleIntersection::clip(*this, rect).release();
    }
#endif

    return HeuristicOverlay(this, other, OverlayNG::INTERSECTION);
}

std::unique_ptr<Geometry>
Geometry::Union(const Geometry* other) const
{
#ifdef SHORTCIRCUIT_PREDICATES
    // if envelopes are disjoint return a MULTI geom or
    // a geometrycollection
    if(! getEnvelopeInternal()->intersects(other->getEnvelopeInternal())) {
//cerr<<"SHORTCIRCUITED-UNION engaged"<<endl;
        const GeometryCollection* coll;

        std::size_t ngeomsThis = getNumGeometries();
        std::size_t ngeomsOther = other->getNumGeometries();

        // Allocated for ownership transfer
        std::vector<std::unique_ptr<Geometry>> v;
        v.reserve(ngeomsThis + ngeomsOther);


        if(nullptr != (coll = dynamic_cast<const GeometryCollection*>(this))) {
            for(std::size_t i = 0; i < ngeomsThis; ++i) {
                v.push_back(coll->getGeometryN(i)->clone());
            }
        }
        else {
            v.push_back(this->clone());
        }

        if(nullptr != (coll = dynamic_cast<const GeometryCollection*>(other))) {
            for(std::size_t i = 0; i < ngeomsOther; ++i) {
                v.push_back(coll->getGeometryN(i)->clone());
            }
        }
        else {
            v.push_back(other->clone());
        }

        return _factory->buildGeometry(std::move(v));
    }
#endif

    return HeuristicOverlay(this, other, OverlayNG::UNION);
}

/* public */
Geometry::Ptr
Geometry::Union() const
{
    using geos::operation::geounion::UnaryUnionOp;
    return UnaryUnionOp::Union(*this);
}

std::unique_ptr<Geometry>
Geometry::difference(const Geometry* other) const
{
    return HeuristicOverlay(this, other, OverlayNG::DIFFERENCE);
}

std::unique_ptr<Geometry>
Geometry::symDifference(const Geometry* other) const
{
    // if envelopes are disjoint return a MULTI geom or
    // a geometrycollection
    if(! getEnvelopeInternal()->intersects(other->getEnvelopeInternal()) && !(isEmpty() && other->isEmpty())) {
        const GeometryCollection* coll;

        std::size_t ngeomsThis = getNumGeometries();
        std::size_t ngeomsOther = other->getNumGeometries();

        // Allocated for ownership transfer
        std::vector<std::unique_ptr<Geometry>> v;
        v.reserve(ngeomsThis + ngeomsOther);


        if(nullptr != (coll = dynamic_cast<const GeometryCollection*>(this))) {
            for(std::size_t i = 0; i < ngeomsThis; ++i) {
                v.push_back(coll->getGeometryN(i)->clone());
            }
        }
        else {
            v.push_back(this->clone());
        }

        if(nullptr != (coll = dynamic_cast<const GeometryCollection*>(other))) {
            for(std::size_t i = 0; i < ngeomsOther; ++i) {
                v.push_back(coll->getGeometryN(i)->clone());
            }
        }
        else {
            v.push_back(other->clone());
        }

        return _factory->buildGeometry(std::move(v));
    }

    return HeuristicOverlay(this, other, OverlayNG::SYMDIFFERENCE);
}

int
Geometry::compareTo(const Geometry* geom) const
{
    // compare to self
    if(this == geom) {
        return 0;
    }

    if(getSortIndex() != geom->getSortIndex()) {
        int diff = getSortIndex() - geom->getSortIndex();
        return (diff > 0) - (diff < 0); // signum()
    }
    if(isEmpty() && geom->isEmpty()) {
        return 0;
    }
    if(isEmpty()) {
        return -1;
    }
    if(geom->isEmpty()) {
        return 1;
    }
    return compareToSameClass(geom);
}

bool
Geometry::isEquivalentClass(const Geometry* other) const
{
    if(typeid(*this) == typeid(*other)) {
        return true;
    }
    else {
        return false;
    }
}

/*public static*/
void
Geometry::checkNotGeometryCollection(const Geometry* g)
//throw(IllegalArgumentException *)
{
    if(g->getSortIndex() == SORTINDEX_GEOMETRYCOLLECTION) {
        throw  geos::util::IllegalArgumentException("This method does not support GeometryCollection arguments\n");
    }
}


void
Geometry::GeometryChangedFilter::filter_rw(Geometry* geom)
{
    geom->geometryChangedAction();
}

int
Geometry::compare(std::vector<Coordinate> a, std::vector<Coordinate> b) const
{
    std::size_t i = 0;
    std::size_t j = 0;
    while(i < a.size() && j < b.size()) {
        Coordinate& aCoord = a[i];
        Coordinate& bCoord = b[j];
        int comparison = aCoord.compareTo(bCoord);
        if(comparison != 0) {
            return comparison;
        }
        i++;
        j++;
    }
    if(i < a.size()) {
        return 1;
    }
    if(j < b.size()) {
        return -1;
    }
    return 0;
}

int
Geometry::compare(std::vector<Geometry*> a, std::vector<Geometry*> b) const
{
    std::size_t i = 0;
    std::size_t j = 0;
    while(i < a.size() && j < b.size()) {
        Geometry* aGeom = a[i];
        Geometry* bGeom = b[j];
        int comparison = aGeom->compareTo(bGeom);
        if(comparison != 0) {
            return comparison;
        }
        i++;
        j++;
    }
    if(i < a.size()) {
        return 1;
    }
    if(j < b.size()) {
        return -1;
    }
    return 0;
}

int
Geometry::compare(const std::vector<std::unique_ptr<Geometry>> & a,
        const std::vector<std::unique_ptr<Geometry>> & b) const
{
    std::size_t i = 0;
    std::size_t j = 0;
    while(i < a.size() && j < b.size()) {
        Geometry* aGeom = a[i].get();
        Geometry* bGeom = b[j].get();
        int comparison = aGeom->compareTo(bGeom);
        if(comparison != 0) {
            return comparison;
        }
        i++;
        j++;
    }
    if(i < a.size()) {
        return 1;
    }
    if(j < b.size()) {
        return -1;
    }
    return 0;
}

/**
 *  Returns the minimum distance between this Geometry
 *  and the other Geometry
 *
 * @param  other  the Geometry from which to compute the distance
 */
double
Geometry::distance(const Geometry* other) const
{
    return DistanceOp::distance(this, other);
}

/**
 *  Returns the area of this <code>Geometry</code>.
 *  Areal Geometries have a non-zero area.
 *  They override this function to compute the area.
 *  Others return 0.0
 *
 * @return the area of the Geometry
 */
double
Geometry::getArea() const
{
    return 0.0;
}

/**
 *  Returns the length of this <code>Geometry</code>.
 *  Linear geometries return their length.
 *  Areal geometries return their perimeter.
 *  They override this function to compute the area.
 *  Others return 0.0
 *
 * @return the length of the Geometry
 */
double
Geometry::getLength() const
{
    return 0.0;
}

Geometry::~Geometry()
{
    _factory->dropRef();
}

bool
GeometryGreaterThen::operator()(const Geometry* first, const Geometry* second)
{
    if(first->compareTo(second) > 0) {
        return true;
    }
    else {
        return false;
    }
}

bool
Geometry::equal(const CoordinateXY& a, const CoordinateXY& b,
                double tolerance) const
{
    if(tolerance == 0) {
        return a == b; // 2D only !!!
    }
    //double dist=a.distance(b);
    return a.distance(b) <= tolerance;
}

void
Geometry::apply_ro(GeometryFilter* filter) const
{
    filter->filter_ro(this);
}

void
Geometry::apply_rw(GeometryFilter* filter)
{
    filter->filter_rw(this);
}

void
Geometry::apply_ro(GeometryComponentFilter* filter) const
{
    filter->filter_ro(this);
}

void
Geometry::apply_rw(GeometryComponentFilter* filter)
{
    filter->filter_rw(this);
}

bool
Geometry::isSimple() const
{
    operation::valid::IsSimpleOp op(*this);
    return op.isSimple();
}

/* public */
const PrecisionModel*
Geometry::getPrecisionModel() const
{
    return _factory->getPrecisionModel();
}

} // namespace geos::geom
} // namespace geos


