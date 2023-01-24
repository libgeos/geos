//
// Test Suite for geos::geom::Polygon class.

#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/Polygon.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Point.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>
// std
#include <cmath>
#include <memory>
#include <string>

using geos::geom::CoordinateSequence;

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_polygon_data {
    // Typedefs used as short names by test cases
    typedef std::unique_ptr<geos::geom::Geometry> GeometryAutoPtr;
    typedef std::unique_ptr<geos::geom::Polygon> PolygonAutoPtr;
    typedef geos::geom::GeometryFactory GeometryFactory;

    using Geometry = geos::geom::Geometry;
    using Envelope = geos::geom::Envelope;
    using Polygon = geos::geom::Polygon;

    geos::geom::PrecisionModel pm_;
    GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;

    PolygonAutoPtr empty_poly_;
    PolygonAutoPtr empty_poly_z_;
    PolygonAutoPtr empty_poly_m_;
    PolygonAutoPtr empty_poly_zm_;

    PolygonAutoPtr poly_;
    PolygonAutoPtr poly_z_;
    PolygonAutoPtr poly_m_;
    PolygonAutoPtr poly_zm_;
    const std::size_t poly_size_;

    test_polygon_data()
        : pm_(1)
        , factory_(GeometryFactory::create(&pm_, 0))
        , reader_(factory_.get())
        , empty_poly_(factory_->createPolygon())
        , empty_poly_z_(factory_->createPolygon(factory_->createLinearRing(geos::detail::make_unique<CoordinateSequence>(0u, true, false))))
        , empty_poly_m_(factory_->createPolygon(factory_->createLinearRing(geos::detail::make_unique<CoordinateSequence>(0u, false, true))))
        , empty_poly_zm_(factory_->createPolygon(factory_->createLinearRing(geos::detail::make_unique<CoordinateSequence>(0u, true, true))))
        , poly_size_(7)
    {
        // Create non-empty Polygon
        poly_ = reader_.read<Polygon>("POLYGON((0 10, 5 5, 10 5, 15 10, 10 15, 5 15, 0 10))");
        poly_z_ = reader_.read<Polygon>("POLYGON Z ((0 10 1, 5 5 2, 10 5 3, 15 10 4, 10 15 5, 5 15 6, 0 10 1))");
        poly_m_ = reader_.read<Polygon>("POLYGON M ((0 10 1, 5 5 2, 10 5 3, 15 10 4, 10 15 5, 5 15 6, 0 10 1))");
        poly_zm_ = reader_.read<Polygon>("POLYGON ZM ((0 10 1 10, 5 5 2 11, 10 5 3 12, 15 10 4 15, 10 15 5 16, 5 15 6 17, 0 10 1 10))");
    }

private:
    // Declare type as noncopyable
    test_polygon_data(const test_polygon_data& other) = delete;
    test_polygon_data& operator=(const test_polygon_data& rhs) = delete;
};

typedef test_group<test_polygon_data> group;
typedef group::object object;

group test_polygon_group("geos::geom::Polygon");

//
// Test Cases
//

// Test of user's constructor to create non-empty valid Polygon
template<>
template<>
void object::test<1>
()
{
    using geos::geom::Coordinate;

    // Create non-empty Coordinate sequence for Exterior LinearRing
    const std::size_t size = 7;
    auto coords = geos::detail::make_unique<CoordinateSequence>();
    ensure("sequence is null pointer.", coords != nullptr);

    coords->add(Coordinate(0, 10));
    coords->add(Coordinate(5, 5));
    coords->add(Coordinate(10, 5));
    coords->add(Coordinate(15, 10));
    coords->add(Coordinate(10, 15));
    coords->add(Coordinate(5, 15));
    coords->add(Coordinate(0, 10));

    ensure_equals(coords->size(), size);

    try {
        // Create non-empty LinearRing instance
        geos::geom::LinearRing ring(std::move(coords), *factory_);
        ensure(!ring.isEmpty());
        ensure(ring.isClosed());
        ensure(ring.isRing());
        ensure(ring.isSimple());

        // Exterior (clone is required here because Polygon takes ownership)
        auto exterior = ring.clone();

        // Create non-empty Polygon
        auto poly= factory_->createPolygon(std::move(exterior));

        ensure(!poly->isEmpty());
        ensure(poly->isSimple());
        ensure(poly->isValid());

        ensure_equals(poly->getNumGeometries(), 1u);
        ensure_equals(poly->getNumInteriorRing(), 0u);
        ensure_equals(poly->getNumPoints(), size);
    }
    catch(geos::util::IllegalArgumentException const& e) {
        fail(e.what());
    }

}

// Test of copy constructor
template<>
template<>
void object::test<2>
()
{
    GeometryAutoPtr copy(empty_poly_->clone());

    ensure(copy->isEmpty());
}

// Test of isEmpty() for empty Polygon
template<>
template<>
void object::test<3>
()
{
    ensure(empty_poly_->isEmpty());
}

// Test of isSimple() for empty Polygon
template<>
template<>
void object::test<4>
()
{
    ensure(empty_poly_->isSimple());
}

// Test of isValid() for empty Polygon
template<>
template<>
void object::test<5>
()
{
    //inform("Test is waiting for Bug #87 resolution.");

    // TODO - mloskot - is empty valid or not?
    //ensure( !ring.isValid() );
}


// Test of getEnvelope() for empty Polygon
template<>
template<>
void object::test<6>
()
{
    //inform( "Test waiting for resolution of getEnvelope() issue." );
    //http://geos.osgeo.org/pipermail/geos-devel/2006-April/002123.html

    //EnvelopeCPtr envelope = empty_poly_->getEnvelopeInternal();
    //ensure( envelope != 0 );
    //ensure( envelope->isNull() );
}

// Test of getBoundary() for empty Polygon
template<>
template<>
void object::test<7>
()
{
    auto boundary = empty_poly_->getBoundary();
    ensure(boundary != nullptr);
    ensure(boundary->isEmpty());
}

// Test of convexHull() for empty Polygon
template<>
template<>
void object::test<8>
()
{
    auto hull = empty_poly_->convexHull();
    ensure(hull != nullptr);
    ensure(hull->isEmpty());
}

// Test of getGeometryTypeId() for empty Polygon
template<>
template<>
void object::test<9>
()
{
    ensure_equals(empty_poly_->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
}

// Test of getDimension() for empty Polygon
template<>
template<>
void object::test<10>
()
{
    ensure_equals(empty_poly_->getDimension(), geos::geom::Dimension::A);
}

// Test of getBoundaryDimension() for empty Polygon
template<>
template<>
void object::test<11>
()
{
    ensure_equals(empty_poly_->getBoundaryDimension(), geos::geom::Dimension::L);
}

// Test of getNumPoints() for empty Polygon
template<>
template<>
void object::test<12>
()
{
    ensure_equals(empty_poly_->getNumPoints(), 0u);
}

// Test of getLength() for empty Polygon
template<>
template<>
void object::test<13>
()
{
    ensure_equals(empty_poly_->getLength(), 0.0);
}

// Test of getArea() for empty Polygon
template<>
template<>
void object::test<14>
()
{
    ensure_equals(empty_poly_->getArea(), 0.0);
}

// Test of isEmpty() for non-empty Polygon
template<>
template<>
void object::test<15>
()
{
    ensure(poly_ != nullptr);
    ensure(!poly_->isEmpty());
}

// Test of getEnvelope() for non-empty Polygon
template<>
template<>
void object::test<17>
()
{
    ensure(poly_ != nullptr);

    auto envelope = poly_->getEnvelope();
    ensure(envelope != nullptr);
    ensure(!envelope->isEmpty());
    ensure_equals(envelope->getDimension(), geos::geom::Dimension::A);
}

// Test of getBoundary() for non-empty Polygon
template<>
template<>
void object::test<18>
()
{
    ensure(poly_ != nullptr);

    auto boundary = poly_->getBoundary();
    ensure(boundary != nullptr);

    // OGC 05-126, Version: 1.1.0, Chapter 6.1.10 Surface
    ensure("[OGC] The boundary of Polygin is the set of closed Curves.", !boundary->isEmpty());
}

// Test of convexHull() for non-empty Polygon
template<>
template<>
void object::test<19>
()
{
    ensure(poly_ != nullptr);

    auto hull = poly_->convexHull();
    ensure(hull != nullptr);
    ensure(!hull->isEmpty());
    ensure_equals(hull->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
    ensure_equals(hull->getDimension(), geos::geom::Dimension::A);
}

// Test of getGeometryTypeId() for non-empty Polygon
template<>
template<>
void object::test<20>
()
{
    ensure(poly_ != nullptr);
    ensure_equals(poly_->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
}

// Test of getDimension() for non-empty Polygon
template<>
template<>
void object::test<21>
()
{
    ensure(poly_ != nullptr);
    ensure_equals(poly_->getDimension(), geos::geom::Dimension::A);
}

// Test of getBoundaryDimension() for non-empty Polygon
template<>
template<>
void object::test<22>
()
{
    ensure(poly_ != nullptr);
    ensure_equals(poly_->getBoundaryDimension(), geos::geom::Dimension::L);
}

// Test of getNumPoints() for non-empty Polygon
template<>
template<>
void object::test<23>
()
{
    ensure(poly_ != nullptr);
    ensure_equals(poly_->getNumPoints(), poly_size_);
}

// Test of getLength() for non-empty Polygon
template<>
template<>
void object::test<24>
()
{
    ensure(poly_ != nullptr);
    ensure(poly_->getLength() != 0.0);

    const double tolerance = 0.0001;
    const double expected = 38.284271247461902;
    const double diff = std::fabs(poly_->getLength() - expected);
    ensure(diff <= tolerance);
}

// Test of getArea() for non-empty Polygon
template<>
template<>
void object::test<25>
()
{
    ensure(poly_ != nullptr);
    ensure(poly_->getArea() != 0.0);
}

// Test of getCoordinates() for non-empty Polygon
template<>
template<>
void object::test<26>
()
{
    ensure(poly_ != nullptr);

    // Caller takes ownership of 'coords'
    auto coords = poly_->getCoordinates();
    ensure(coords != nullptr);
    ensure(!coords->isEmpty());
    ensure_equals(coords->getSize(), poly_->getNumPoints());
    ensure_equals(poly_->hasZ(), coords->hasZ());
    ensure_equals(poly_->hasM(), coords->hasM());

    auto coords_z = poly_z_->getCoordinates();
    ensure_equals(poly_z_->hasZ(), coords_z->hasZ());
    ensure_equals(poly_z_->hasM(), coords_z->hasM());

    auto coords_m = poly_m_->getCoordinates();
    ensure_equals(poly_m_->hasZ(), coords_m->hasZ());
    ensure_equals(poly_m_->hasM(), coords_m->hasM());

    auto coords_zm = poly_zm_->getCoordinates();
    ensure_equals(poly_zm_->hasZ(), coords_zm->hasZ());
    ensure_equals(poly_zm_->hasM(), coords_zm->hasM());
}

// Test of clone() and equals() for non-empty Polygon
template<>
template<>
void object::test<27>
()
{
    ensure(poly_ != nullptr);

    auto geo = poly_->clone();
    ensure(geo != nullptr);
    ensure(geo->equals(poly_.get()));
}

// Test of getExteriorRing() for non-empty Polygon
template<>
template<>
void object::test<28>
()
{
    ensure(poly_ != nullptr);

    LineStringCPtr ring = poly_->getExteriorRing();
    ensure(ring != nullptr);
    ensure(ring->isRing());
    ensure_equals(ring->getNumPoints(), poly_size_);
}

// Test of getNumInteriorRing() for non-empty Polygon but without interior rings
template<>
template<>
void object::test<29>
()
{
    ensure(poly_ != nullptr);
    ensure_equals(poly_->getNumInteriorRing(), 0u);
}

// Test of getInteriorRingN() for non-empty Polygon with interior rings
template<>
template<>
void object::test<30>
()
{
    const std::size_t holesNum = 1;

    auto geo = reader_.read("POLYGON ((0 0, 100 0, 100 100, 0 100, 0 0), (1 1, 1 10, 10 10, 10 1, 1 1) )");
    ensure(geo != nullptr);
    ensure_equals(geo->getGeometryTypeId(), geos::geom::GEOS_POLYGON);

    PolygonPtr poly = dynamic_cast<PolygonPtr>(geo.get());
    ensure(poly != nullptr);
    ensure_equals(poly->getNumInteriorRing(), holesNum);

    LineStringCPtr interior = poly->getInteriorRingN(0);
    ensure(interior != nullptr);
    ensure(interior->isRing());

    ensure_equals(interior->getGeometryTypeId(), geos::geom::GEOS_LINEARRING);
}

// Test of getCoordinate() for non-empty Polygon
template<>
template<>
void object::test<31>
()
{
    ensure(poly_ != nullptr);
    // "POLYGON((0 10, 5 5, 10 5, 15 10, 10 15, 5 15, 0 10))"

    const auto* coord = poly_->getCoordinate();
    ensure(coord != nullptr);
    ensure_equals(coord->x, 0);
    ensure_equals(coord->y, 10);
}

// Test of getCoordinates() for non-empty Polygon
template<>
template<>
void object::test<32>
()
{
    ensure(poly_ != nullptr);
    // "POLYGON((0 10, 5 5, 10 5, 15 10, 10 15, 5 15, 0 10))"

    auto coords = poly_->getCoordinates();
    ensure(coords != nullptr);
    ensure_equals(coords->getSize(), poly_size_);

    // Check first and last coordinates
    const std::size_t lastPos = poly_size_ - 1;
    ensure_equals(coords->getAt(0), coords->getAt(lastPos));

    // Check coordinate	from the middle of ring
    const int middlePos = 3;
    ensure_equals(coords->getAt(middlePos).x, 15);
    ensure_equals(coords->getAt(middlePos).y, 10);
}

// Test of getGeometryType() for non-empty Polygon
template<>
template<>
void object::test<33>
()
{
    ensure(poly_ != nullptr);

    const std::string type("Polygon");
    ensure_equals(poly_->getGeometryType(), type);
}

// Test of Point* getCentroid() const for empty Polygon
template<>
template<>
void object::test<34>
()
{
    auto point = empty_poly_->getCentroid();
    ensure("getCentroid returned nullptr", point != nullptr);
    ensure("getCentroid(POLYGON EMPTY) did not return POINT EMPTY)", point->isEmpty());
}

// Test of Geometry::getCentroid(Coordinate& ret) const for empty Polygon
template<>
template<>
void object::test<35>
()
{
    geos::geom::Coordinate centroid;
    bool isCentroid = empty_poly_->getCentroid(centroid);
    ensure(!isCentroid);
}

// Test of getCentroid() for non-empty Polygon
template<>
template<>
void object::test<36>
()
{
    auto point = poly_->getCentroid();
    ensure(point != nullptr);
    ensure(!point->isEmpty());
    ensure_equals(point->getGeometryTypeId(), geos::geom::GEOS_POINT);
}

// Test of Geometry::getCentroid(Coordinate& ret) const for non-empty Polygon
template<>
template<>
void object::test<37>
()
{
    geos::geom::Coordinate centroid;
    bool isCentroid = poly_->getCentroid(centroid);
    ensure(isCentroid);
}

// Test of comparison of centroids returned by two versions of getCentroid() for non-empty Polygon
template<>
template<>
void object::test<38>
()
{
    // First centroid
    auto point = poly_->getCentroid();
    ensure(point != nullptr);
    ensure(!point->isEmpty());
    ensure_equals(point->getGeometryTypeId(), geos::geom::GEOS_POINT);

    const auto* pointCoord = point->getCoordinate();
    ensure(pointCoord != nullptr);
    geos::geom::Coordinate pointCentr(*pointCoord);

    // Second centroid
    geos::geom::Coordinate coordCentr;
    bool isCentroid = poly_->getCentroid(coordCentr);
    ensure(isCentroid);

    // Comparison of two centroids
    ensure_equals("Check Polygon::getCentroid() functions.", coordCentr, pointCentr);
}

// Test of Geometry::buffer(0) with convex polygon
template<>
template<>
void object::test<39>
()
{
    std::unique_ptr<geos::geom::Geometry> gBuffer(poly_->buffer(0));
    ensure_not(gBuffer->isEmpty());
    ensure(gBuffer->isValid());
    ensure_equals(gBuffer->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
    ensure(gBuffer->getNumPoints() == poly_->getNumPoints());
}

template<>
template<>
void object::test<40>
()
{
    // getCoordinate() returns nullptr for empty geometry
    auto gf = geos::geom::GeometryFactory::create();
    std::unique_ptr<geos::geom::Geometry> g(gf->createPolygon());

    ensure(g->getCoordinate() == nullptr);
}

// test isDimensionStrict for empty Polygon
template<>
template<>
void object::test<41>
()
{
    ensure(empty_poly_->isDimensionStrict(geos::geom::Dimension::A));
    ensure(!empty_poly_->isDimensionStrict(geos::geom::Dimension::L));
}

// test isDimensionStrict for non-empty Polygon
template<>
template<>
void object::test<42>
()
{
    ensure(poly_->isDimensionStrict(geos::geom::Dimension::A));
    ensure(!poly_->isDimensionStrict(geos::geom::Dimension::L));
}


// test compareToSameClass for polygons including holes
template<>
template<>
void object::test<43>
()
{
    auto poly = reader_.read("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))");

    ensure("polygon should equal self",
        poly->compareTo(poly.get()) == 0);

    auto poly2 = reader_.read("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (2 2, 2 3, 3 3, 3 2, 2 2))");
    ensure("polygons with different holes but same shell are not equal",
        poly->compareTo(poly2.get()) != 0);

    auto poly3 = reader_.read("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");
    ensure("polygons with and without holes are not equal",
        poly->compareTo(poly3.get()) != 0);
}

template<>
template<>
void object::test<44>()
{
    auto poly = reader_.read<Polygon>("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (2 2, 2 3, 3 3, 3 2, 2 2))");
    auto env = poly->getEnvelopeInternal();
    ensure_equals(*env, Envelope(0,10, 0, 10));

    auto shell = poly->releaseExteriorRing();
    ensure_equals(*shell->getEnvelopeInternal(), Envelope(0, 10, 0, 10));

    auto holes = poly->releaseInteriorRings();
    ensure_equals(holes.size(), 1u);
}

// Test of hasDimension()
template<>
template<>
void object::test<45>
()
{
    ensure(!poly_->hasDimension(geos::geom::Dimension::P));
    ensure(!poly_->hasDimension(geos::geom::Dimension::L));
    ensure(poly_->hasDimension(geos::geom::Dimension::A));
}

// Test of getCoordinates for empty polygons
template<>
template<>
void object::test<46>
()
{
    ensure(!poly_->getCoordinates()->hasZ());
    ensure(!poly_->getCoordinates()->hasM());

    ensure(!empty_poly_->getCoordinates()->hasZ());
    ensure(!empty_poly_->getCoordinates()->hasM());

    ensure(poly_z_->getCoordinates()->hasZ());
    ensure(!poly_z_->getCoordinates()->hasM());

    ensure(empty_poly_z_->getCoordinates()->hasZ());
    ensure(!empty_poly_z_->getCoordinates()->hasM());

    ensure(!poly_m_->getCoordinates()->hasZ());
    ensure(poly_m_->getCoordinates()->hasM());

    ensure(!empty_poly_m_->getCoordinates()->hasZ());
    ensure(empty_poly_m_->getCoordinates()->hasM());

    ensure(poly_zm_->getCoordinates()->hasZ());
    ensure(poly_zm_->getCoordinates()->hasM());

    ensure(empty_poly_zm_->getCoordinates()->hasZ());
    ensure(empty_poly_zm_->getCoordinates()->hasM());
}

// Test of hasZ, hasM
template<>
template<>
void object::test<47>
()
{
    ensure(!poly_->hasZ());
    ensure(!poly_->hasM());
    ensure(!empty_poly_->hasZ());
    ensure(!empty_poly_->hasM());

    ensure(poly_z_->hasZ());
    ensure(!poly_z_->hasM());
    ensure(empty_poly_z_->hasZ());
    ensure(!empty_poly_z_->hasM());

    ensure(!poly_m_->hasZ());
    ensure(poly_m_->hasM());
    ensure(!empty_poly_m_->hasZ());
    ensure(empty_poly_m_->hasM());

    ensure(poly_zm_->hasZ());
    ensure(poly_zm_->hasM());
    ensure(empty_poly_zm_->hasZ());
    ensure(empty_poly_zm_->hasM());
}

} // namespace tut
