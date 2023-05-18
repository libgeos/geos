//
// Test Suite for geos::geom::GeometryFactory class.

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>
// std
#include <vector>
#include <cstring> // std::size_t

using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYM;
using geos::geom::CoordinateXYZM;
using geos::geom::GeometryFactory;
using geos::geom::PrecisionModel;

/*!
 * \brief
 * Write brief comment for tut here.
 *
 * Write detailed description for tut here.
 *
 * \remarks
 * Write remarks for tut here.
 *
 * \see
 * Separate items with the '|' character.
 */
namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_geometryfactory_data {
    // Test data
    const int x_;
    const int y_;
    const int z_;

    const int srid_;
    geos::geom::PrecisionModel pm_;
    geos::geom::GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;

    test_geometryfactory_data()
        : x_(5), y_(10), z_(15), srid_(666), pm_(1.0),
          factory_(geos::geom::GeometryFactory::create(&pm_, srid_)),
          reader_(factory_.get())
    {}
private:
    // Declare type as noncopyable
    test_geometryfactory_data(const test_geometryfactory_data& other) = delete;
    test_geometryfactory_data& operator=(const test_geometryfactory_data& rhs) = delete;
};

typedef test_group<test_geometryfactory_data> group;
typedef group::object object;

group test_geometryfactory_group("geos::geom::GeometryFactory");

//
// Test Cases
//

// Test of default constructor
template<>
template<>
void object::test<1>
()
{
    GeometryFactory::Ptr gf = GeometryFactory::create();

    ensure_equals(gf->getSRID(), 0);
    ensure_equals(gf->getPrecisionModel()->getType(), PrecisionModel::FLOATING);

    auto geo = gf->createEmptyGeometry();
    ensure("createEmptyGeometry() returned null pointer.", geo != nullptr);
    ensure_equals(geo->getSRID(), gf->getSRID());
    ensure_equals(geo->getPrecisionModel()->getType(), PrecisionModel::FLOATING);
}

// Test of user's constructor
template<>
template<>
void object::test<2>
()
{
    PrecisionModel pm(1.0);
    GeometryFactory::Ptr gf = GeometryFactory::create(&pm, srid_);

    ensure_equals(gf->getSRID(), srid_);
    ensure_equals(gf->getPrecisionModel()->getType(), PrecisionModel::FIXED);

    auto geo = gf->createEmptyGeometry();
    ensure("createEmptyGeometry() returned null pointer.", geo != nullptr);
    ensure_equals(geo->getSRID(), gf->getSRID());
    ensure_equals(geo->getSRID(), srid_);
    ensure_equals(geo->getPrecisionModel()->getType(), PrecisionModel::FIXED);
} // test<2>

// Test of user's constructor
template<>
template<>
void object::test<3>
()
{
    GeometryFactory::Ptr gf = GeometryFactory::create();

    ensure_equals(gf->getSRID(), 0);
    ensure_equals(gf->getPrecisionModel()->getType(), PrecisionModel::FLOATING);

    auto geo = gf->createEmptyGeometry();
    ensure("createEmptyGeometry() returned null pointer.", geo != nullptr);
    ensure_equals(geo->getSRID(), gf->getSRID());
    ensure_equals(geo->getPrecisionModel()->getType(), PrecisionModel::FLOATING);
}

// Test of user's constructor
template<>
template<>
void object::test<4>
()
{
    PrecisionModel pm(PrecisionModel::FIXED);
    GeometryFactory::Ptr gf(GeometryFactory::create(&pm));

    ensure_equals(gf->getSRID(), 0);
    ensure_equals(gf->getPrecisionModel()->getType(), PrecisionModel::FIXED);

    auto geo = gf->createEmptyGeometry();
    ensure("createEmptyGeometry() returned null pointer.", geo != nullptr);
    ensure_equals(geo->getSRID(), gf->getSRID());
    ensure_equals(geo->getPrecisionModel()->getType(), PrecisionModel::FIXED);
}

// Test of user's constructor
template<>
template<>
void object::test<5>
()
{
    PrecisionModel pm(PrecisionModel::FIXED);
    GeometryFactory::Ptr gf(GeometryFactory::create(&pm, srid_));

    ensure_equals(gf->getSRID(), srid_);
    ensure_equals(gf->getPrecisionModel()->getType(), PrecisionModel::FIXED);

    auto geo = gf->createEmptyGeometry();
    ensure("createEmptyGeometry() returned null pointer.", geo != nullptr);
    ensure_equals(geo->getSRID(), gf->getSRID());
    ensure_equals(geo->getPrecisionModel()->getType(), PrecisionModel::FIXED);
}

// Test of copy constructor
template<>
template<>
void object::test<6>
()
{
    GeometryFactory::Ptr gf(GeometryFactory::create(*factory_));

    ensure_equals(factory_->getSRID(), gf->getSRID());
    ensure_equals(factory_->getPrecisionModel()->getType(), gf->getPrecisionModel()->getType());
}

// Test of createEmptyGeometry() const
template<>
template<>
void object::test<7>
()
{
    auto geo = factory_->createEmptyGeometry();

    ensure("createEmptyGeometry() returned null pointer.", geo != nullptr);
    ensure("createEmptyGeometry() returned non-empty geometry.", geo->isEmpty());

    // TODO - mloskot
    // http://geos.osgeo.org/pipermail/geos-devel/2006-March/001960.html
    /*
    ensure( geo->isSimple() );
    ensure( geo->isValid() );

    ensure( geo->getCentroid() == 0 );
    ensure( geo->getCoordinate() == 0 );
    */
}

// Test of createPoint() const
template<>
template<>
void object::test<8>
()
{
    auto pt = factory_->createPoint();

    ensure("createPoint() returned null pointer.", pt != nullptr);
    ensure("createPoint() returned non-empty point.", pt->isEmpty());
    ensure(pt->isSimple());
    ensure(pt->isValid());
    ensure(pt->getCentroid()->isEmpty());
    ensure(pt->getCoordinate() == nullptr);

    std::unique_ptr<geos::geom::Geometry> geo;
    geo = pt->getEnvelope();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    geo = pt->getBoundary();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    geo = pt->convexHull();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    ensure_equals(pt->getGeometryTypeId(), geos::geom::GEOS_POINT);
    ensure_equals(pt->getCoordinateDimension(), 2u);
    ensure_equals(pt->getDimension(), geos::geom::Dimension::P);
    ensure_equals(pt->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(pt->getNumPoints(), 0u);
    ensure_equals(pt->getLength(), 0.0);
    ensure_equals(pt->getArea(), 0.0);
    ensure_equals(pt->getSRID(), srid_);
}

// Test of createPoint(const Coordinate &coordinate) const
template<>
template<>
void object::test<9>
()
{
    geos::geom::Coordinate coord(x_, y_, z_);

    auto pt = factory_->createPoint(coord);

    ensure("createPoint() returned null pointer.", pt != nullptr);
    ensure("createPoint() returned empty point.", !pt->isEmpty());
    ensure(pt->isSimple());
    ensure(pt->isValid());
    ensure(pt->getCoordinate() != nullptr);

    Coordinate pcoord;
    pt->getCoordinatesRO()->getAt(0, pcoord);
    ensure_equals(pcoord.x, x_);
    ensure_equals(pcoord.y, y_);
    ensure_equals(pcoord.z, z_);

    std::unique_ptr<geos::geom::Geometry> geo;
    geo = pt->getEnvelope();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = pt->getCentroid();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = pt->getBoundary();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    geo = pt->convexHull();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    ensure_equals(pt->getCoordinateDimension(), 3u);
    ensure_equals(pt->getGeometryTypeId(), geos::geom::GEOS_POINT);
    ensure_equals(pt->getDimension(), geos::geom::Dimension::P);
    ensure_equals(pt->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(pt->getNumPoints(), 1u);
    ensure_equals(pt->getLength(), 0.0);
    ensure_equals(pt->getArea(), 0.0);
}

// Test of createPoint(std::unique_ptr<CoordinateSequence>&& coordinates) const
template<>
template<>
void object::test<10>
()
{
    geos::geom::Coordinate coord(x_, y_, z_);

    auto sequence = geos::detail::make_unique<CoordinateSequence>();

    ensure("sequence is null pointer.", sequence != nullptr);
    sequence->add(coord);

    auto pt = factory_->createPoint(std::move(sequence));

    ensure("createPoint() returned null pointer.", pt != nullptr);
    ensure("createPoint() returned empty point.", !pt->isEmpty());
    ensure(pt->isSimple());
    ensure(pt->isValid());
    ensure(pt->getCoordinate() != nullptr);

    Coordinate pcoord;
    pt->getCoordinatesRO()->getAt(0, pcoord);
    ensure_equals(pcoord.x, x_);
    ensure_equals(pcoord.y, y_);
    ensure_equals(pcoord.z, z_);

    std::unique_ptr<geos::geom::Geometry> geo;
    geo = pt->getEnvelope();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = pt->getCentroid();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = pt->getBoundary();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    geo = pt->convexHull();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    ensure_equals(pt->getGeometryTypeId(), geos::geom::GEOS_POINT);
    ensure_equals(pt->getDimension(), geos::geom::Dimension::P);
    ensure_equals(pt->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(pt->getNumPoints(), 1u);
    ensure_equals(pt->getLength(), 0.0);
    ensure_equals(pt->getArea(), 0.0);
}

// Test of createPoint(const CoordinateSequence &coordinates) const
template<>
template<>
void object::test<11>
()
{
    geos::geom::Coordinate coord(x_, y_, z_);

    geos::geom::CoordinateSequence sequence;
    sequence.add(coord);

    auto pt = factory_->createPoint(sequence);

    ensure("createPoint() returned null pointer.", pt != nullptr);
    ensure("createPoint() returned empty point.", !pt->isEmpty());
    ensure(pt->isSimple());
    ensure(pt->isValid());
    ensure(pt->getCoordinate() != nullptr);

    Coordinate pcoord;
    pt->getCoordinatesRO()->getAt(0, pcoord);
    ensure_equals(pcoord.x, x_);
    ensure_equals(pcoord.y, y_);
    ensure_equals(pcoord.z, z_);

    std::unique_ptr<geos::geom::Geometry> geo;
    geo = pt->getEnvelope();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = pt->getCentroid();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = pt->getBoundary();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    geo = pt->convexHull();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    ensure_equals(pt->getGeometryTypeId(), geos::geom::GEOS_POINT);
    ensure_equals(pt->getDimension(), geos::geom::Dimension::P);
    ensure_equals(pt->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(pt->getNumPoints(), 1u);
    ensure_equals(pt->getLength(), 0.0);
    ensure_equals(pt->getArea(), 0.0);
}

// Test of createLinearRing() const
template<>
template<>
void object::test<12>
()
{
    auto lr = factory_->createLinearRing();

    ensure("createLinearRing() returned null pointer.", lr != nullptr);
    ensure("createLinearRing() returned non-empty point.", lr->isEmpty());
    ensure(lr->isEmpty());
    ensure(lr->isSimple());
    ensure(lr->isValid());
    ensure(lr->getCoordinate() == nullptr);

    // TODO - mloskot
    //http://geos.osgeo.org/pipermail/geos-devel/2006-March/001961.html
    //ensure( lr->isClosed() );

    // TODO - mloskot
    //http://geos.osgeo.org/pipermail/geos-devel/2006-March/001962.html
    //ensure_equals( lr->getStartPoint(), lr->getEndPoint() );

    ensure_equals(lr->getGeometryTypeId(), geos::geom::GEOS_LINEARRING);
    ensure_equals(lr->getDimension(), geos::geom::Dimension::L);
    ensure_equals(lr->getCoordinateDimension(), 2u);
    ensure_equals(lr->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(lr->getNumPoints(), 0u);
    ensure_equals(lr->getLength(), 0.0);
    ensure_equals(lr->getArea(), 0.0);
}

// Test of createLinearRing(std::unique_ptr<CoordinateSequence>&& newCoords) const
template<>
template<>
void object::test<13>
()
{
    const std::size_t size = 5;
    auto coords = geos::detail::make_unique<CoordinateSequence>(size);
    ensure(coords != nullptr);
    ensure_equals(coords->getSize(), size);

    auto lr = factory_->createLinearRing(std::move(coords));
    ensure("createLinearRing() returned null pointer.", lr != nullptr);
    ensure("createLinearRing() returned empty point.", !lr->isEmpty());
    ensure(lr->isSimple());
    ensure(lr->getCoordinate() != nullptr);

    // TODO - mloskot - is this correct?
    //ensure( !lr->isValid() );

    ensure_equals(lr->getGeometryTypeId(), geos::geom::GEOS_LINEARRING);
    ensure_equals(lr->getDimension(), geos::geom::Dimension::L);
    ensure_equals(lr->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(lr->getNumPoints(), size);
    ensure_equals(lr->getLength(), 0.0);
    ensure_equals(lr->getArea(), 0.0);
}

// Test of createLinearRing(const CoordinateSequence& coordinates) const
template<>
template<>
void object::test<14>
()
{
    const std::size_t size = 5;
    geos::geom::CoordinateSequence coords(size);
    ensure_equals(coords.getSize(), size);

    auto lr = factory_->createLinearRing(coords);
    ensure("createLinearRing() returned empty point.", !lr->isEmpty());
    ensure_equals(lr->getNumPoints(), size);
    ensure(lr->isSimple());
    ensure(lr->getCoordinate() != nullptr);

    ensure_equals(lr->getGeometryTypeId(), geos::geom::GEOS_LINEARRING);
    ensure_equals(lr->getDimension(), geos::geom::Dimension::L);
    ensure_equals(lr->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(lr->getNumPoints(), size);
    ensure_equals(lr->getLength(), 0.0);
    ensure_equals(lr->getArea(), 0.0);
}

// Test of createLineString() const
template<>
template<>
void object::test<15>
()
{
    auto line = factory_->createLineString();

    ensure("createLineString() returned null pointer.", line != nullptr);
    ensure("createLineString() returned non-empty point.", line->isEmpty());
    ensure(line->isSimple());
    ensure(line->isValid());
    ensure(line->getCentroid()->isEmpty());

    // TODO - mloskot - waiting for some decision
    // http://geos.osgeo.org/pipermail/geos-devel/2006-March/002006.html
    //ensure( line->getCoordinate() == 0 );

    std::unique_ptr<geos::geom::Geometry> geo;
    geo = line->getEnvelope();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    geo = line->getBoundary();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    geo = line->convexHull();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    ensure_equals(line->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals(line->getDimension(), geos::geom::Dimension::L);
    ensure_equals(line->getCoordinateDimension(), 2u);
    ensure_equals(line->getBoundaryDimension(), geos::geom::Dimension::P);   // empty linestring
    ensure_equals(line->getNumPoints(), 0u);
    ensure_equals(line->getLength(), 0.0);
    ensure_equals(line->getArea(), 0.0);
}

// Test of createLineString(std::unique_ptr<CoordinateSequence>&& coordinates) const
template<>
template<>
void object::test<16>
()
{
    const std::size_t size = 5;
    auto coords = geos::detail::make_unique<CoordinateSequence>(size);
    ensure(coords != nullptr);
    ensure_equals(coords->getSize(), size);

    auto line = factory_->createLineString(std::move(coords));
    ensure("createLineString() returned null pointer.", line != nullptr);
    ensure("createLineString() returned empty point.", !line->isEmpty());
    ensure(line->isSimple());
    ensure(line->getCoordinate() != nullptr);

    // TODO - mloskot - is this correct?
    //ensure( line->isValid() );

    ensure_equals(line->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals(line->getDimension(), geos::geom::Dimension::L);
    ensure_equals(line->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(line->getNumPoints(), size);
    ensure_equals(line->getLength(), 0.0);
    ensure_equals(line->getArea(), 0.0);
}

// Test of createLineString(const CoordinateSequence& coordinates) const
template<>
template<>
void object::test<17>
()
{
    const std::size_t size = 5;
    geos::geom::CoordinateSequence coords(size);
    ensure_equals(coords.getSize(), size);

    auto line = factory_->createLineString(coords);
    ensure("createLineString() returned empty point.", !line->isEmpty());
    ensure_equals(line->getNumPoints(), size);
    ensure(line->isSimple());
    ensure(line->getCoordinate() != nullptr);

    ensure_equals(line->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals(line->getDimension(), geos::geom::Dimension::L);
    ensure_equals(line->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(line->getNumPoints(), size);
    ensure_equals(line->getLength(), 0.0);
    ensure_equals(line->getArea(), 0.0);
}

// Test of createPolygon() const
template<>
template<>
void object::test<18>
()
{
    auto poly = factory_->createPolygon();

    ensure("createPolygon() returned null pointer.", poly != nullptr);
    ensure("createPolygon() returned non-empty point.", poly->isEmpty());
    ensure(poly->isSimple());
    ensure(poly->isValid());
    ensure(poly->getCentroid()->isEmpty());

    // TODO - mloskot - waiting for some decision
    // http://geos.osgeo.org/pipermail/geos-devel/2006-March/002006.html
    //ensure( poly->getCoordinate() == 0 );

    std::unique_ptr<geos::geom::Geometry> geo;
    // TODO - mloskot - waiting for resolution
    // http://geos.osgeo.org/pipermail/geos-devel/2006-March/002011.html
    //geo = poly->getEnvelope();
    //ensure( geo != 0 );
    //ensure( geo->isEmpty() );
    //factory_->destroyGeometry(geo);

    geo = poly->getBoundary();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    geo = poly->convexHull();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    ensure_equals(poly->getCoordinateDimension(), 2u);
    ensure_equals(poly->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
    ensure_equals(poly->getDimension(), geos::geom::Dimension::A);
    ensure_equals(poly->getBoundaryDimension(), geos::geom::Dimension::L);
    ensure_equals(poly->getNumPoints(), 0u);
    ensure_equals(poly->getLength(), 0.0);
    ensure_equals(poly->getArea(), 0.0);
}

// Test of createPolygon(LinearRing* shell, std::vector<Geometry*>* holes) const
template<>
template<>
void object::test<19>
()
{
    using geos::geom::Coordinate;
    const std::size_t size = 7;

    // Create sequence of coordinates
    auto coords = geos::detail::make_unique<CoordinateSequence>(size);
    ensure(coords != nullptr);
    coords->setAt(Coordinate(0, 10), 0);
    coords->setAt(Coordinate(5, 5), 1);
    coords->setAt(Coordinate(10, 5), 2);
    coords->setAt(Coordinate(15, 10), 3);
    coords->setAt(Coordinate(10, 15), 4);
    coords->setAt(Coordinate(5, 15), 5);
    coords->setAt(Coordinate(0, 10), 6);
    ensure_equals(coords->getSize(), size);

    // Create exterior ring
    auto exterior = factory_->createLinearRing(std::move(coords));
    ensure("createLinearRing returned null pointer.", exterior != nullptr);
    ensure("createLinearRing() returned empty point.", !exterior->isEmpty());
    ensure(exterior->isSimple());
    ensure_equals(exterior->getGeometryTypeId(), geos::geom::GEOS_LINEARRING);
    ensure_equals(exterior->getDimension(), geos::geom::Dimension::L);
    ensure_equals(exterior->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(exterior->getNumPoints(), size);
    ensure_equals(exterior->getArea(), 0.0);
    ensure(exterior->getLength() != 0.0);

    // Create polygon
    auto poly = factory_->createPolygon(std::move(exterior));
    ensure("createPolygon returned null pointer.", poly != nullptr);
    ensure("createPolygon() returned empty point.", !poly->isEmpty());
    ensure(poly->isSimple());
    ensure_equals(poly->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
    ensure_equals(poly->getDimension(), geos::geom::Dimension::A);
    ensure_equals(poly->getBoundaryDimension(), geos::geom::Dimension::L);
    ensure_equals(poly->getNumPoints(), size);
    ensure(poly->getArea() != 0.0);
    ensure(poly->getLength() != 0.0);
}

// Test of createPolygon(const LinearRing& shell, const std::vector<Geometry*>& holes) const
template<>
template<>
void object::test<20>
()
{
    using geos::geom::Coordinate;
    const std::size_t exteriorSize = 7;
    const std::size_t interiorSize = 5;

    // Create sequence of coordinates
    auto coords = geos::detail::make_unique<CoordinateSequence>(exteriorSize);
    ensure(coords != nullptr);
    coords->setAt(Coordinate(0, 10), 0);
    coords->setAt(Coordinate(5, 5), 1);
    coords->setAt(Coordinate(10, 5), 2);
    coords->setAt(Coordinate(15, 10), 3);
    coords->setAt(Coordinate(10, 15), 4);
    coords->setAt(Coordinate(5, 15), 5);
    coords->setAt(Coordinate(0, 10), 6);
    ensure_equals(coords->getSize(), exteriorSize);

    // Create exterior ring
    auto exterior = factory_->createLinearRing(std::move(coords));
    ensure("createLinearRing returned null pointer.", exterior != nullptr);
    ensure("createLinearRing() returned empty point.", !exterior->isEmpty());
    ensure(exterior->isSimple());
    ensure_equals(exterior->getGeometryTypeId(), geos::geom::GEOS_LINEARRING);
    ensure_equals(exterior->getDimension(), geos::geom::Dimension::L);
    ensure_equals(exterior->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(exterior->getNumPoints(), exteriorSize);
    ensure_equals(exterior->getArea(), 0.0);
    ensure(exterior->getLength() != 0.0);

    // Create collection of holes
    auto hole = reader_.read<LinearRing>(("LINEARRING(7 7, 12 7, 12 12, 7 12, 7 7)"));
    ensure(hole != nullptr);
    ensure(hole->isRing());
    ensure_equals(hole->getNumPoints(), interiorSize);

    std::vector<decltype(hole)> holes;
    holes.emplace_back(std::move(hole));

    // Create polygon using copy ctor
    auto poly = factory_->createPolygon(std::move(exterior), std::move(holes));
    ensure("createPolygon returned null pointer.", poly != nullptr);
    ensure("createPolygon() returned empty point.", !poly->isEmpty());
    ensure(poly->isSimple());
    ensure_equals(poly->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
    ensure_equals(poly->getDimension(), geos::geom::Dimension::A);
    ensure_equals(poly->getBoundaryDimension(), geos::geom::Dimension::L);
    ensure_equals(poly->getNumPoints(), exteriorSize + interiorSize);
    ensure(poly->getArea() != 0.0);
    ensure(poly->getLength() != 0.0);

    ensure_equals(poly->getNumGeometries(), 1u);
    ensure_equals(poly->getNumInteriorRing(), 1u);
}

// Test of createGeometryCollection() const
template<>
template<>
void object::test<21>
()
{
    auto col = factory_->createGeometryCollection();

    ensure("createGeometryCollection() returned null pointer.", col != nullptr);
    ensure(col->isEmpty());
    ensure(col->isValid());
    ensure(col->getCentroid()->isEmpty());
    ensure_equals(col->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);
    ensure_equals(col->getCoordinateDimension(), 2u);
    ensure_equals(col->getDimension(), geos::geom::Dimension::False);
    ensure_equals(col->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(col->getNumPoints(), 0u);
    ensure_equals(col->getLength(), 0.0);
    ensure_equals(col->getArea(), 0.0);
}

// Test of createGeometryCollection(std::vector<Geometry*>* newGeoms) const
template<>
template<>
void object::test<22>
()
{
    using geos::geom::Coordinate;

    // Buffer for geometries
    std::vector<std::unique_ptr<Geometry>> vec;

    // Add single point
    Coordinate coord(x_, y_, z_);
    std::unique_ptr<Geometry> point(factory_->createPoint(coord));
    ensure(point != nullptr);
    vec.push_back(std::move(point));

    // Add single LineString
    auto coords = geos::detail::make_unique<CoordinateSequence>(3u);
    ensure(coords != nullptr);
    coords->setAt(Coordinate(0, 0), 0);
    coords->setAt(Coordinate(5, 5), 1);
    coords->setAt(Coordinate(10, 5), 2);
    ensure_equals(coords->getSize(), 3u);
    auto line = factory_->createLineString(std::move(coords));
    vec.push_back(std::move(line));

    // Create geometry collection
    auto col = factory_->createGeometryCollection(std::move(vec));
    ensure(col != nullptr);
    ensure_equals(col->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);
    ensure_equals(col->getNumGeometries(), 2u);
}

// Test of createGeometryCollection(const std::vector<const Geometry*>& newGeoms) const
template<>
template<>
void object::test<23>
()
{
    const std::size_t size = 3;
    geos::geom::Coordinate coord(x_, y_, z_);

    std::vector<const geos::geom::Geometry*> vec;

    auto geo1 = factory_->createPoint(coord);
    vec.push_back(geo1.get());

    coord.x *= 2;
    coord.y *= 2;
    coord.z *= 2;
    auto geo2 = factory_->createPoint(coord);
    vec.push_back(geo2.get());

    coord.x *= 3;
    coord.y *= 3;
    coord.z *= 3;
    auto geo3 = factory_->createPoint(coord);
    vec.push_back(geo3.get());

    // Factory creates copy of the vec collection
    auto col = factory_->createGeometryCollection(vec);
    ensure(col != nullptr);
    ensure_equals(col->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);
    ensure_equals(col->getNumGeometries(), size);
}

// Test of createMultiPoint() const
template<>
template<>
void object::test<24>
()
{
    auto mp = factory_->createMultiPoint();

    ensure("createMultiPoint() returned null pointer.", mp != nullptr);
    ensure("createMultiPoint() returned non-empty point.", mp->isEmpty());
    ensure(mp->isSimple());
    ensure(mp->isValid());
    ensure(mp->getCentroid()->isEmpty());

    std::unique_ptr<geos::geom::Geometry> geo;

    // TODO - mloskot - waiting for resolution
    // http://geos.osgeo.org/pipermail/geos-devel/2006-March/002011.html
    //geo = poly->getEnvelope();
    //ensure( geo != 0 );
    //ensure( geo->isEmpty() );
    //factory_->destroyGeometry(geo);

    geo = mp->getBoundary();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    geo = mp->convexHull();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    ensure_equals(mp->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);
    ensure_equals(mp->getCoordinateDimension(), 2u);
    ensure_equals(mp->getDimension(), geos::geom::Dimension::P);
    ensure_equals(mp->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(mp->getNumPoints(), 0u);
    ensure_equals(mp->getLength(), 0.0);
    ensure_equals(mp->getArea(), 0.0);
}

// Test of createMultiPoint(std::vector<std::unique_ptr<Geometry>>&& newPoints) const
template<>
template<>
void object::test<25>
()
{
    const std::size_t size = 3;
    geos::geom::Coordinate coord(x_, y_, z_);

    std::vector<std::unique_ptr<Geometry>> vec;

    auto geo = factory_->createPoint(coord);
    ensure(geo != nullptr);
    vec.push_back(std::move(geo));

    coord.x *= 2;
    coord.y *= 2;
    coord.z *= 2;
    geo = factory_->createPoint(coord);
    ensure(geo != nullptr);
    vec.push_back(std::move(geo));

    coord.x *= 3;
    coord.y *= 3;
    coord.z *= 3;
    geo = factory_->createPoint(coord);
    ensure(geo != nullptr);
    vec.push_back(std::move(geo));

    // Factory creates copy of the vec collection
    auto mp = factory_->createMultiPoint(std::move(vec));
    ensure(mp != nullptr);
    ensure(mp->isValid());
    ensure(mp->isSimple());
    ensure_equals(mp->getNumGeometries(), size);
    ensure_equals(mp->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);
}

// Test of createMultiPoint(const std::vector<const Geometry*>& fromPoints) const
template<>
template<>
void object::test<26>
()
{
    const std::size_t size = 3;
    geos::geom::Coordinate coord(x_, y_, z_);

    std::vector<const geos::geom::Geometry*> vec;

    auto geo1 = factory_->createPoint(coord);
    vec.push_back(geo1.get());

    coord.x *= 2;
    coord.y *= 2;
    coord.z *= 2;
    auto geo2 = factory_->createPoint(coord);
    vec.push_back(geo2.get());

    coord.x *= 3;
    coord.y *= 3;
    coord.z *= 3;
    auto geo3 = factory_->createPoint(coord);
    vec.push_back(geo3.get());

    // Factory creates copy of the vec collection
    auto mp = factory_->createMultiPoint(vec);
    ensure(mp != nullptr);
    ensure(mp->isValid());
    ensure(mp->isSimple());
    ensure_equals(mp->getNumGeometries(), size);
    ensure_equals(mp->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);
}

// Test of createMultiPoint(const CoordinateSequence& fromCoords) const
template<>
template<>
void object::test<27>
()
{
    using geos::geom::Coordinate;
    const std::size_t size = 3;

    // Add collection of coordinates
    geos::geom::CoordinateSequence coords(size);
    coords.setAt(Coordinate(0, 0), 0);
    coords.setAt(Coordinate(5, 5), 1);
    coords.setAt(Coordinate(10, 5), 2);
    ensure_equals(coords.getSize(), size);

    auto mp = factory_->createMultiPoint(coords);
    ensure(mp != nullptr);
    ensure(mp->isValid());
    ensure(mp->isSimple());
    ensure_equals(mp->getNumGeometries(), size);
    ensure_equals(mp->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);
}

// Test of createMultiLineString() const
template<>
template<>
void object::test<28>
()
{
    auto mls = factory_->createMultiLineString();

    ensure("createMultiLineString() returned null pointer.", mls != nullptr);
    ensure("createMultiLineString() returned non-empty point.", mls->isEmpty());
    ensure(mls->isSimple());
    ensure(mls->isValid());
    ensure(mls->getCentroid()->isEmpty());

    std::unique_ptr<geos::geom::Geometry> geo;

    // TODO - mloskot - waiting for resolution
    // http://geos.osgeo.org/pipermail/geos-devel/2006-March/002011.html
    //geo = poly->getEnvelope();
    //ensure( geo != 0 );
    //ensure( geo->isEmpty() );
    //factory_->destroyGeometry(geo);

    geo = mls->getBoundary();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    geo = mls->convexHull();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    ensure_equals(mls->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);
    ensure_equals(mls->getCoordinateDimension(), 2u);
    ensure_equals(mls->getDimension(), geos::geom::Dimension::L);
    ensure_equals(mls->getBoundaryDimension(), geos::geom::Dimension::P);
    ensure_equals(mls->getNumPoints(), 0u);
    ensure_equals(mls->getLength(), 0.0);
    ensure_equals(mls->getArea(), 0.0);
}

// Test of createMultiLineString(std::vector<std::unique_ptr<Geometry>>&& newLines) const
template<>
template<>
void object::test<29>
()
{
    using geos::geom::Coordinate;

    const std::size_t size = 5;
    const std::size_t lineSize = 2;

    std::vector<std::unique_ptr<Geometry>> lines;

    for(std::size_t i = 0; i < size; ++i) {
        const double factor = static_cast<double>(i * i);
        auto coords = geos::detail::make_unique<geos::geom::CoordinateSequence>(lineSize);
        ensure(coords != nullptr);
        coords->setAt(Coordinate(0. + factor, 0. + factor), 0);
        coords->setAt(Coordinate(5. + factor, 5. + factor), 1);
        ensure_equals(coords->getSize(), lineSize);

        auto line = factory_->createLineString(std::move(coords));
        ensure("createLineString() returned empty point.", !line->isEmpty());
        ensure_equals(line->getNumPoints(), lineSize);
        ensure(line->isSimple());
        ensure(line->getCoordinate() != nullptr);
        ensure_equals(line->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);

        lines.push_back(std::move(line));
    }

    auto mls = factory_->createMultiLineString(std::move(lines));
    ensure(mls != nullptr);
    // TODO - mloskot - why isValid() returns false?
    //ensure( mls->isValid() );
    ensure_equals(mls->getNumGeometries(), size);
    ensure_equals(mls->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);
}

// Test of createMultiLineString(const std::vector<Geometry*>& fromLines) const
template<>
template<>
void object::test<30>
()
{
    using geos::geom::Coordinate;

    const std::size_t size = 5;
    const std::size_t lineSize = 2;

    std::vector<std::unique_ptr<geos::geom::Geometry>> lines;

    for(std::size_t i = 0; i < size; ++i) {
        const double factor = static_cast<double>(i * i);
        auto coords = geos::detail::make_unique<CoordinateSequence>(lineSize);
        ensure(coords != nullptr);
        coords->setAt(Coordinate(0. + factor, 0. + factor), 0);
        coords->setAt(Coordinate(5. + factor, 5. + factor), 1);
        ensure_equals(coords->getSize(), lineSize);

        auto line = factory_->createLineString(std::move(coords));
        ensure("createLineString() returned empty point.", !line->isEmpty());
        ensure_equals(line->getNumPoints(), lineSize);
        ensure(line->isSimple());
        ensure(line->getCoordinate() != nullptr);
        ensure_equals(line->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);

        lines.push_back(std::move(line));
    }

    auto mls = factory_->createMultiLineString(std::move(lines));
    ensure(mls != nullptr);
    // TODO - mloskot - why isValid() returns false?
    //ensure( mls->isValid() );
    ensure_equals(mls->getNumGeometries(), size);
    ensure_equals(mls->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);
}

// Test of createMultiPolygon() const
template<>
template<>
void object::test<31>
()
{
    auto mp = factory_->createMultiPolygon();

    ensure_equals(mp->getGeometryTypeId(), geos::geom::GEOS_MULTIPOLYGON);
    ensure_equals(mp->getCoordinateDimension(), 2u);
    ensure_equals(mp->getDimension(), geos::geom::Dimension::A);
}

// Test of createMultiPolygon(std::vector<Geometry>* newPolys) const
template<>
template<>
void object::test<32>
()
{
    // TODO - mloskot
    //inform("Test not implemented!");
}

// Test of createMultiPolygon(const std::vector<Geometry*>& fromPolys) const
template<>
template<>
void object::test<33>
()
{
    // TODO - mloskot
    //inform("Test not implemented!");
}

// Test of buildGeometry(std::vector<Geometry*>* geoms) const
template<>
template<>
void object::test<34>
()
{
    // TODO - mloskot
    //inform("Test not implemented!");
}

// Test of buildGeometry(const std::vector<Geometry*>& geoms)
template<>
template<>
void object::test<35>
()
{
    // TODO - mloskot
    //inform("Test not implemented!");
}

// Test of
// buildGeometry(from, to, takeOwnership) const
template<>
template<>
void object::test<36>
()
{
    typedef std::vector<PointPtr> PointVect;

    const std::size_t size = 3;
    geos::geom::Coordinate coord(x_, y_, z_);

    PointVect vec;

    auto geo1 = factory_->createPoint(coord);
    vec.push_back(geo1.get());

    coord.x *= 2;
    coord.y *= 2;
    coord.z *= 2;
    auto geo2 = factory_->createPoint(coord);
    vec.push_back(geo2.get());

    coord.x *= 3;
    coord.y *= 3;
    coord.z *= 3;
    auto geo3 = factory_->createPoint(coord);
    vec.push_back(geo3.get());

    // Factory creates copy of the vec collection
    auto g = factory_->buildGeometry(vec.begin(), vec.end());
    ensure(g.get() != nullptr);
    ensure_equals(g->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);
    ensure_equals(g->getNumGeometries(), size);
}

// Test of
// createMultiPoint(std::vector<Coordinate> &&)
template<>
template<>
void object::test<37>
()
{
    std::vector<geos::geom::Coordinate> coords;
    coords.emplace_back(1, 1);
    coords.emplace_back(2, 2);

    auto mp = factory_->createMultiPoint(std::move(coords));

    ensure_equals(mp->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);
    ensure_equals(mp->getNumGeometries(), 2u);
}

// Test of createPoint(const Coordinate &coordinate) const
template<>
template<>
void object::test<38>
()
{
    geos::geom::CoordinateXY coord(x_, y_);

    auto pt = factory_->createPoint(coord);

    ensure("createPoint() returned null pointer.", pt != nullptr);
    ensure("createPoint() returned empty point.", !pt->isEmpty());
    ensure(pt->isSimple());
    ensure(pt->isValid());
    ensure(pt->getCoordinate() != nullptr);

    const CoordinateXY* pcoord = pt->getCoordinate();
    ensure(pcoord != nullptr);
    ensure_equals(pcoord->x, x_);
    ensure_equals(pcoord->y, y_);

    std::unique_ptr<geos::geom::Geometry> geo;
    geo = pt->getEnvelope();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = pt->getCentroid();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    geo = pt->getBoundary();
    ensure(geo != nullptr);
    ensure(geo->isEmpty());

    geo = pt->convexHull();
    ensure(geo != nullptr);
    ensure(!geo->isEmpty());

    ensure_equals(pt->getCoordinateDimension(), 2u);
    ensure_equals(pt->getGeometryTypeId(), geos::geom::GEOS_POINT);
    ensure_equals(pt->getDimension(), geos::geom::Dimension::P);
    ensure_equals(pt->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(pt->getNumPoints(), 1u);
    ensure_equals(pt->getLength(), 0.0);
    ensure_equals(pt->getArea(), 0.0);
}

// Test of createPoint for various coordinate types
template<>
template<>
void object::test<39>
()
{
    // XY
    auto pt_xy = factory_->createPoint(CoordinateXY{1, 2});
    CoordinateXY xy_out;
    ensure_equals(pt_xy->getCoordinateDimension(), 2u);
    pt_xy->getCoordinatesRO()->getAt(0, xy_out);
    ensure_equals(xy_out, CoordinateXY{1, 2});
    ensure(!pt_xy->hasZ());
    ensure(!pt_xy->hasM());

    // XYZ
    auto pt_xyz = factory_->createPoint(Coordinate{1, 2, 3});
    Coordinate xyz_out;
    pt_xyz->getCoordinatesRO()->getAt(0, xyz_out);
    ensure_equals(pt_xyz->getCoordinateDimension(), 3u);
    ensure_equals_xyz(xyz_out, Coordinate{1, 2, 3});
    ensure(pt_xyz->hasZ());
    ensure(!pt_xyz->hasM());

    // XYM
    auto pt_xym = factory_->createPoint(CoordinateXYM{1, 2, 3});
    CoordinateXYM xym_out;
    pt_xym->getCoordinatesRO()->getAt(0, xym_out);
    ensure_equals(pt_xym->getCoordinateDimension(), 3u);
    ensure_equals_xym(xym_out, CoordinateXYM{1, 2, 3});
    ensure(!pt_xym->hasZ());
    ensure(pt_xym->hasM());

    // XYZM
    auto pt_xyzm = factory_->createPoint(CoordinateXYZM{1, 2, 3, 4});
    CoordinateXYZM xyzm_out;
    pt_xyzm->getCoordinatesRO()->getAt(0, xyzm_out);
    ensure_equals(pt_xyzm->getCoordinateDimension(), 4u);
    ensure_equals_xyzm(xyzm_out, CoordinateXYZM{1, 2, 3, 4});
    ensure(pt_xyzm->hasZ());
    ensure(pt_xyzm->hasM());
}

// createMultiPoint(CoordinateSequence) preserves dimension
template<>
template<>
void object::test<40>
()
{
    // XY
    CoordinateSequence xy_seq{CoordinateXY{1,2}, CoordinateXY{3,4}};
    auto mp_xy = factory_->createMultiPoint(xy_seq);
    ensure_equals(mp_xy->getCoordinateDimension(), 2u);
    ensure_equals(mp_xy->getNumGeometries(), 2u);
    ensure(!mp_xy->hasZ());
    ensure(!mp_xy->hasM());

    // XYZ
    CoordinateSequence xyz_seq{Coordinate{1,2,3}, Coordinate{4,5,6}};
    auto mp_xyz = factory_->createMultiPoint(xyz_seq);
    ensure_equals(mp_xyz->getCoordinateDimension(), 3u);
    ensure_equals(mp_xy->getNumGeometries(), 2u);
    ensure(mp_xyz->hasZ());
    ensure(!mp_xyz->hasM());

    // XYM
    CoordinateSequence xym_seq{CoordinateXYM{1,2,3}, CoordinateXYM{4,5,6}};
    auto mp_xym = factory_->createMultiPoint(xym_seq);
    ensure_equals(mp_xym->getCoordinateDimension(), 3u);
    ensure_equals(mp_xy->getNumGeometries(), 2u);
    ensure(!mp_xym->hasZ());
    ensure(mp_xym->hasM());

    // XYZM
    CoordinateSequence xyzm_seq{CoordinateXYZM{1,2,3,4}, CoordinateXYZM{5,6,7,8}};
    auto mp_xyzm = factory_->createMultiPoint(xyzm_seq);
    ensure_equals(mp_xyzm->getCoordinateDimension(), 4u);
    ensure_equals(mp_xy->getNumGeometries(), 2u);
    ensure(mp_xyzm->hasZ());
    ensure(mp_xyzm->hasM());
}


// Test createEmpty and createMulti
template<>
template<>
void object::test<41>
()
{
    auto g1 = factory_->createEmpty(geos::geom::GEOS_MULTIPOINT);
    auto g2 = factory_->createEmpty(geos::geom::GEOS_POINT);
    auto mg1 = factory_->createMulti(std::move(g1));
    auto mg2 = factory_->createMulti(std::move(g2));
    ensure(mg1->isEmpty());
    ensure(mg2->isEmpty());
    g1 = reader_.read("POINT(1 1)");
    g2 = reader_.read("MULTIPOINT((1 1))");
    mg1 = factory_->createMulti(std::move(g1));
    mg2 = factory_->createMulti(std::move(g2));
    g2 = reader_.read("MULTIPOINT((1 1))");
    ensure_equals_geometry(g2.get(), mg1.get());
    ensure_equals_geometry(g2.get(), mg2.get());
}


} // namespace tut
