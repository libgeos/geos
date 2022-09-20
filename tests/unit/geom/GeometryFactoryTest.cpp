//
// Test Suite for geos::geom::GeometryFactory class.

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/CoordinateSequenceFactory.h>
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
// std
#include <vector>
#include <cstring> // std::size_t

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
    using geos::geom::GeometryFactory;
    GeometryFactory::Ptr gf = GeometryFactory::create();

    ensure_equals(gf->getSRID(), 0);
    ensure_equals(gf->getPrecisionModel()->getType(), geos::geom::PrecisionModel::FLOATING);

    auto geo = gf->createEmptyGeometry();
    ensure("createEmptyGeometry() returned null pointer.", geo != nullptr);
    ensure_equals(geo->getSRID(), gf->getSRID());
    ensure_equals(geo->getPrecisionModel()->getType(), geos::geom::PrecisionModel::FLOATING);
}

// Test of user's constructor
template<>
template<>
void object::test<2>
()
{
    using geos::geom::GeometryFactory;
    using geos::geom::PrecisionModel;
    using geos::geom::CoordinateArraySequenceFactory;

    CoordinateArraySequenceFactory csf;

    {
        PrecisionModel pm(1.0);
        GeometryFactory::Ptr gf = GeometryFactory::create(&pm, srid_, &csf);

        ensure_equals(gf->getSRID(), srid_);
        ensure_equals(gf->getPrecisionModel()->getType(), geos::geom::PrecisionModel::FIXED);

        ensure_equals(&csf, gf->getCoordinateSequenceFactory());

        auto geo = gf->createEmptyGeometry();
        ensure("createEmptyGeometry() returned null pointer.", geo != nullptr);
        ensure_equals(geo->getSRID(), gf->getSRID());
        ensure_equals(geo->getPrecisionModel()->getType(), geos::geom::PrecisionModel::FIXED);
    }
    // csf lifetime must exceed lifetime of the GeometryFactory instance

} // test<2>

// Test of user's constructor
template<>
template<>
void object::test<3>
()
{
    using geos::geom::GeometryFactory;
    using geos::geom::CoordinateArraySequenceFactory;

    CoordinateArraySequenceFactory csf;

    {
        GeometryFactory::Ptr gf = GeometryFactory::create(&csf);

        ensure_equals(gf->getSRID(), 0);
        ensure_equals(gf->getPrecisionModel()->getType(), geos::geom::PrecisionModel::FLOATING);

        auto geo = gf->createEmptyGeometry();
        ensure("createEmptyGeometry() returned null pointer.", geo != nullptr);
        ensure_equals(geo->getSRID(), gf->getSRID());
        ensure_equals(geo->getPrecisionModel()->getType(), geos::geom::PrecisionModel::FLOATING);
    }
    // csf lifetime must exceed lifetime of the GeometryFactory instance
}

// Test of user's constructor
template<>
template<>
void object::test<4>
()
{
    using geos::geom::PrecisionModel;
    using geos::geom::GeometryFactory;

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
    using geos::geom::PrecisionModel;
    using geos::geom::GeometryFactory;

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
    using geos::geom::GeometryFactory;
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
    ensure(pt->getCentroid() == nullptr);
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
    ensure_equals(pt->getDimension(), geos::geom::Dimension::P);
    ensure_equals(pt->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(pt->getNumPoints(), 0u);
    ensure_equals(pt->getLength(), 0.0);
    ensure_equals(pt->getArea(), 0.0);
}

// Test of createPoint(const Coordinate &coordinate) const
template<>
template<>
void object::test<9>
()
{
    geos::geom::Coordinate coord(x_, y_, z_);

    PointPtr pt = factory_->createPoint(coord);

    ensure("createPoint() returned null pointer.", pt != nullptr);
    ensure("createPoint() returned empty point.", !pt->isEmpty());
    ensure(pt->isSimple());
    ensure(pt->isValid());
    ensure(pt->getCoordinate() != nullptr);

    CoordinateCPtr pcoord = pt->getCoordinate();
    ensure(pcoord != nullptr);
    ensure_equals(pcoord->x, x_);
    ensure_equals(pcoord->y, y_);
    ensure_equals(pcoord->z, z_);

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

    // FREE MEMORY
    factory_->destroyGeometry(pt);
}

// Test of createPoint(CoordinateSequence *coordinates) const
template<>
template<>
void object::test<10>
()
{
    geos::geom::Coordinate coord(x_, y_, z_);

    CoordArrayPtr sequence = new geos::geom::CoordinateArraySequence();

    ensure("sequence is null pointer.", sequence != nullptr);
    sequence->add(coord);

    PointPtr pt = factory_->createPoint(sequence);

    ensure("createPoint() returned null pointer.", pt != nullptr);
    ensure("createPoint() returned empty point.", !pt->isEmpty());
    ensure(pt->isSimple());
    ensure(pt->isValid());
    ensure(pt->getCoordinate() != nullptr);

    CoordinateCPtr pcoord = pt->getCoordinate();
    ensure(pcoord != nullptr);
    ensure_equals(pcoord->x, x_);
    ensure_equals(pcoord->y, y_);
    ensure_equals(pcoord->z, z_);

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

    // FREE MEMORY
    factory_->destroyGeometry(pt);
}

// Test of createPoint(const CoordinateSequence &coordinates) const
template<>
template<>
void object::test<11>
()
{
    geos::geom::Coordinate coord(x_, y_, z_);

    geos::geom::CoordinateArraySequence sequence;
    sequence.add(coord);

    PointPtr pt = factory_->createPoint(sequence);

    ensure("createPoint() returned null pointer.", pt != nullptr);
    ensure("createPoint() returned empty point.", !pt->isEmpty());
    ensure(pt->isSimple());
    ensure(pt->isValid());
    ensure(pt->getCoordinate() != nullptr);

    CoordinateCPtr pcoord = pt->getCoordinate();
    ensure(pcoord != nullptr);
    ensure_equals(pcoord->x, x_);
    ensure_equals(pcoord->y, y_);
    ensure_equals(pcoord->z, z_);

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

    // FREE MEMORY
    factory_->destroyGeometry(pt);
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
    ensure_equals(lr->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(lr->getNumPoints(), 0u);
    ensure_equals(lr->getLength(), 0.0);
    ensure_equals(lr->getArea(), 0.0);
}

// Test of createLinearRing(CoordinateSequence* newCoords) const
template<>
template<>
void object::test<13>
()
{
    const std::size_t size = 5;
    CoordArrayPtr coords = new geos::geom::CoordinateArraySequence(size);
    ensure(coords != nullptr);
    ensure_equals(coords->getSize(), size);

    LinearRingPtr lr = factory_->createLinearRing(coords);
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

    // FREE MEMORY
    factory_->destroyGeometry(lr);
}

// Test of createLinearRing(const CoordinateSequence& coordinates) const
template<>
template<>
void object::test<14>
()
{
    const std::size_t size = 5;
    geos::geom::CoordinateArraySequence coords(size);
    ensure_equals(coords.getSize(), size);

    LinearRingPtr lr = factory_->createLinearRing(coords);
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

    // FREE MEMORY
    factory_->destroyGeometry(lr);
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
    ensure(line->getCentroid() == nullptr);

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
    ensure_equals(line->getBoundaryDimension(), geos::geom::Dimension::P);   // empty linestring
    ensure_equals(line->getNumPoints(), 0u);
    ensure_equals(line->getLength(), 0.0);
    ensure_equals(line->getArea(), 0.0);
}

// Test of createLineString(CoordinateSequence* coordinates) const
template<>
template<>
void object::test<16>
()
{
    const std::size_t size = 5;
    CoordArrayPtr coords = new geos::geom::CoordinateArraySequence(size);
    ensure(coords != nullptr);
    ensure_equals(coords->getSize(), size);

    LineStringPtr line = factory_->createLineString(coords);
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

    // FREE MEMORY
    factory_->destroyGeometry(line);
}

// Test of createLineString(const CoordinateSequence& coordinates) const
template<>
template<>
void object::test<17>
()
{
    const std::size_t size = 5;
    geos::geom::CoordinateArraySequence coords(size);
    ensure_equals(coords.getSize(), size);

    LineStringPtr line = factory_->createLineString(coords);
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

    // FREE MEMORY
    factory_->destroyGeometry(line);
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
    ensure(poly->getCentroid() == nullptr);

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

    // Create sequence of coordiantes
    CoordArrayPtr coords = new geos::geom::CoordinateArraySequence(size);
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
    LinearRingPtr exterior = factory_->createLinearRing(coords);
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
    PolygonPtr poly = factory_->createPolygon(exterior, nullptr);
    ensure("createPolygon returned null pointer.", poly != nullptr);
    ensure("createPolygon() returned empty point.", !poly->isEmpty());
    ensure(poly->isSimple());
    ensure_equals(poly->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
    ensure_equals(poly->getDimension(), geos::geom::Dimension::A);
    ensure_equals(poly->getBoundaryDimension(), geos::geom::Dimension::L);
    ensure_equals(poly->getNumPoints(), size);
    ensure(poly->getArea() != 0.0);
    ensure(poly->getLength() != 0.0);

    // FREE MEMORY
    factory_->destroyGeometry(poly);
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

    // Create sequence of coordiantes
    CoordArrayPtr coords = new geos::geom::CoordinateArraySequence(exteriorSize);
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
    LinearRingPtr exterior = factory_->createLinearRing(coords);
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
    auto geo = reader_.read(("LINEARRING(7 7, 12 7, 12 12, 7 12, 7 7)"));
    ensure(geo != nullptr);

    LinearRingPtr hole = dynamic_cast<LinearRingPtr>(geo.release());
    ensure(hole != nullptr);
    ensure(hole->isRing());
    ensure_equals(hole->getNumPoints(), interiorSize);

    // REMEMBER TO DEALLOCATE THIS COLLECTION
    std::vector<LinearRingPtr> holes;
    holes.push_back(hole);

    // Create polygon using copy ctor
    PolygonPtr poly = factory_->createPolygon((*exterior), holes);
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

    // FREE MEMORY
    for(auto& h : holes) {
        delete h;
    }
    holes.clear();

    factory_->destroyGeometry(exterior);
    factory_->destroyGeometry(poly);
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
    ensure(col->getCentroid() == nullptr);
    ensure_equals(col->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);
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
    std::vector<GeometryPtr>* vec = new std::vector<GeometryPtr>();

    // Add single point
    Coordinate coord(x_, y_, z_);
    GeometryPtr point = factory_->createPoint(coord);
    ensure(point != nullptr);
    vec->push_back(point);

    // Add single LineString
    CoordArrayPtr coords = new geos::geom::CoordinateArraySequence(3);
    ensure(coords != nullptr);
    coords->setAt(Coordinate(0, 0), 0);
    coords->setAt(Coordinate(5, 5), 1);
    coords->setAt(Coordinate(10, 5), 2);
    ensure_equals(coords->getSize(), 3u);
    GeometryPtr line = factory_->createLineString(coords);
    vec->push_back(line);

    // Create geometry collection
    GeometryColPtr col = factory_->createGeometryCollection(vec);
    ensure(coords != nullptr);
    ensure_equals(col->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);
    ensure_equals(col->getNumGeometries(), 2u);

    // FREE MEMORY
    factory_->destroyGeometry(col);
}

// Test of createGeometryCollection(const std::vector<Geometry*>& newGeoms) const
template<>
template<>
void object::test<23>
()
{
    const std::size_t size = 3;
    geos::geom::Coordinate coord(x_, y_, z_);

    std::vector<const geos::geom::Geometry*> vec;

    GeometryPtr geo = nullptr;
    geo = factory_->createPoint(coord);
    vec.push_back(geo);

    coord.x *= 2;
    coord.y *= 2;
    coord.z *= 2;
    geo = factory_->createPoint(coord);
    vec.push_back(geo);

    coord.x *= 3;
    coord.y *= 3;
    coord.z *= 3;
    geo = factory_->createPoint(coord);
    vec.push_back(geo);

    // Factory creates copy of the vec collection
    GeometryColPtr col = factory_->createGeometryCollection(vec);
    ensure(col != nullptr);
    ensure_equals(col->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);
    ensure_equals(col->getNumGeometries(), size);

    // FREE MEMORY
    factory_->destroyGeometry(col);
    for(auto& g : vec) {
        delete g;
    }
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
    ensure(mp->getCentroid() == nullptr);

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
    ensure_equals(mp->getDimension(), geos::geom::Dimension::P);
    ensure_equals(mp->getBoundaryDimension(), geos::geom::Dimension::False);
    ensure_equals(mp->getNumPoints(), 0u);
    ensure_equals(mp->getLength(), 0.0);
    ensure_equals(mp->getArea(), 0.0);
}

// Test of createMultiPoint(std::vector<Geometry*>* newPoints) const
template<>
template<>
void object::test<25>
()
{
    const std::size_t size = 3;
    geos::geom::Coordinate coord(x_, y_, z_);

    std::vector<GeometryPtr>* vec = new std::vector<GeometryPtr>();

    GeometryPtr geo = nullptr;
    geo = factory_->createPoint(coord);
    ensure(geo != nullptr);
    vec->push_back(geo);

    coord.x *= 2;
    coord.y *= 2;
    coord.z *= 2;
    geo = factory_->createPoint(coord);
    ensure(geo != nullptr);
    vec->push_back(geo);

    coord.x *= 3;
    coord.y *= 3;
    coord.z *= 3;
    geo = factory_->createPoint(coord);
    ensure(geo != nullptr);
    vec->push_back(geo);

    // Factory creates copy of the vec collection
    MultiPointPtr mp = factory_->createMultiPoint(vec);
    ensure(mp != nullptr);
    ensure(mp->isValid());
    ensure(mp->isSimple());
    ensure_equals(mp->getNumGeometries(), size);
    ensure_equals(mp->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);

    // FREE MEMORY
    factory_->destroyGeometry(mp);
}

// Test of createMultiPoint(const std::vector<Geometry*>& fromPoints) const
template<>
template<>
void object::test<26>
()
{
    const std::size_t size = 3;
    geos::geom::Coordinate coord(x_, y_, z_);

    std::vector<const geos::geom::Geometry*> vec;

    GeometryPtr geo = nullptr;
    geo = factory_->createPoint(coord);
    vec.push_back(geo);

    coord.x *= 2;
    coord.y *= 2;
    coord.z *= 2;
    geo = factory_->createPoint(coord);
    vec.push_back(geo);

    coord.x *= 3;
    coord.y *= 3;
    coord.z *= 3;
    geo = factory_->createPoint(coord);
    vec.push_back(geo);

    // Factory creates copy of the vec collection
    MultiPointPtr mp = factory_->createMultiPoint(vec);
    ensure(mp != nullptr);
    ensure(mp->isValid());
    ensure(mp->isSimple());
    ensure_equals(mp->getNumGeometries(), size);
    ensure_equals(mp->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);

    // FREE MEMORY
    factory_->destroyGeometry(mp);
    for(auto& g : vec) {
        delete g;
    }
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
    geos::geom::CoordinateArraySequence coords(size);
    coords.setAt(Coordinate(0, 0), 0);
    coords.setAt(Coordinate(5, 5), 1);
    coords.setAt(Coordinate(10, 5), 2);
    ensure_equals(coords.getSize(), size);

    MultiPointPtr mp = factory_->createMultiPoint(coords);
    ensure(mp != nullptr);
    ensure(mp->isValid());
    ensure(mp->isSimple());
    ensure_equals(mp->getNumGeometries(), size);
    ensure_equals(mp->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);

    // FREE MEMORY
    factory_->destroyGeometry(mp);
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
    ensure(mls->getCentroid() == nullptr);

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
    ensure_equals(mls->getDimension(), geos::geom::Dimension::L);
    ensure_equals(mls->getBoundaryDimension(), geos::geom::Dimension::P);
    ensure_equals(mls->getNumPoints(), 0u);
    ensure_equals(mls->getLength(), 0.0);
    ensure_equals(mls->getArea(), 0.0);
}

// Test of createMultiLineString(std::vector<Geometry*>* newLines) const
template<>
template<>
void object::test<29>
()
{
    using geos::geom::Coordinate;

    const std::size_t size = 5;
    const std::size_t lineSize = 2;

    std::vector<GeometryPtr>* lines = new std::vector<GeometryPtr>();

    for(std::size_t i = 0; i < size; ++i) {
        const double factor = static_cast<double>(i * i);
        CoordArrayPtr coords = new geos::geom::CoordinateArraySequence(lineSize);
        ensure(coords != nullptr);
        coords->setAt(Coordinate(0. + factor, 0. + factor), 0);
        coords->setAt(Coordinate(5. + factor, 5. + factor), 1);
        ensure_equals(coords->getSize(), lineSize);

        LineStringPtr line = factory_->createLineString(coords);
        ensure("createLineString() returned empty point.", !line->isEmpty());
        ensure_equals(line->getNumPoints(), lineSize);
        ensure(line->isSimple());
        ensure(line->getCoordinate() != nullptr);
        ensure_equals(line->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);

        lines->push_back(line);
    }

    MultiLineStringPtr mls = factory_->createMultiLineString(lines);
    ensure(mls != nullptr);
    // TODO - mloskot - why isValid() returns false?
    //ensure( mls->isValid() );
    ensure_equals(mls->getNumGeometries(), size);
    ensure_equals(mls->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);

    // FREE MEMORY
    factory_->destroyGeometry(mls);
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

    std::vector<const geos::geom::Geometry*> lines;

    for(std::size_t i = 0; i < size; ++i) {
        const double factor = static_cast<double>(i * i);
        CoordArrayPtr coords = new geos::geom::CoordinateArraySequence(lineSize);
        ensure(coords != nullptr);
        coords->setAt(Coordinate(0. + factor, 0. + factor), 0);
        coords->setAt(Coordinate(5. + factor, 5. + factor), 1);
        ensure_equals(coords->getSize(), lineSize);

        LineStringPtr line = factory_->createLineString(coords);
        ensure("createLineString() returned empty point.", !line->isEmpty());
        ensure_equals(line->getNumPoints(), lineSize);
        ensure(line->isSimple());
        ensure(line->getCoordinate() != nullptr);
        ensure_equals(line->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);

        lines.push_back(line);
    }

    MultiLineStringPtr mls = factory_->createMultiLineString(lines);
    ensure(mls != nullptr);
    // TODO - mloskot - why isValid() returns false?
    //ensure( mls->isValid() );
    ensure_equals(mls->getNumGeometries(), size);
    ensure_equals(mls->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);

    // FREE MEMORY
    factory_->destroyGeometry(mls);
    for(auto& g : lines) {
        delete g;
    }
}

// Test of createMultiPolygon() const
template<>
template<>
void object::test<31>
()
{
    // TODO - mloskot
    //inform("Test not implemented!");
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
    typedef std::unique_ptr<geos::geom::Geometry> GeometryAutoPtr;
    typedef std::vector<PointPtr> PointVect;

    const std::size_t size = 3;
    geos::geom::Coordinate coord(x_, y_, z_);

    PointVect vec;

    PointPtr geo = nullptr;
    geo = factory_->createPoint(coord);
    vec.push_back(geo);

    coord.x *= 2;
    coord.y *= 2;
    coord.z *= 2;
    geo = factory_->createPoint(coord);
    vec.push_back(geo);

    coord.x *= 3;
    coord.y *= 3;
    coord.z *= 3;
    geo = factory_->createPoint(coord);
    vec.push_back(geo);

    // Factory creates copy of the vec collection
    GeometryAutoPtr g = factory_->buildGeometry(vec.begin(), vec.end());
    ensure(g.get() != nullptr);
    ensure_equals(g->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);
    ensure_equals(g->getNumGeometries(), size);

    // FREE MEMORY
    PointVect::const_iterator it;
    for(it = vec.begin(); it != vec.end(); ++it) {
        delete(*it);
    }
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

} // namespace tut
