//
// Test Suite for geos::geom::MultiPoint class.

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/MultiPoint.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/ParseException.h>
#include <geos/io/WKTReader.h>
// std
#include <cmath>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_multipoint_data {
    typedef std::unique_ptr<geos::geom::MultiPoint> MultiPointAutoPtr;
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::PrecisionModel pm_;
    geos::geom::GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;

    MultiPointAutoPtr empty_mp_;
    MultiPointPtr mp_;
    const size_t mp_size_;

    test_multipoint_data()
        :
        pm_(1.0), factory_(GeometryFactory::create(&pm_, 0))
        , reader_(factory_.get())
        , empty_mp_(factory_->createMultiPoint()), mp_size_(5)
    {
        // Create non-empty MultiPoint
        auto geo = reader_.read("MULTIPOINT(0 0, 5 5, 10 10, 15 15, 20 20)");
        mp_ = dynamic_cast<MultiPointPtr>(geo.release());
    }

    ~test_multipoint_data()
    {
        factory_->destroyGeometry(mp_);
    }

private:
    // Declare type as noncopyable
    test_multipoint_data(const test_multipoint_data& other) = delete;
    test_multipoint_data& operator=(const test_multipoint_data& rhs) = delete;
};

typedef test_group<test_multipoint_data> group;
typedef group::object object;

group test_multipoint_group("geos::geom::MultiPoint");

//
// Test Cases
//

// Test of user's constructor
template<>
template<>
void object::test<1>
()
{
    const size_t size0 = 0;
    MultiPointAutoPtr mp(factory_->createMultiPoint());

    ensure(mp->isEmpty());
    ensure(mp->isSimple());
    ensure(mp->isValid());
    ensure(mp->getCentroid() == nullptr);
    ensure_equals(mp->getNumPoints(), size0);
    ensure_equals(mp->getNumGeometries(), size0);
}

// Test of copy constructor
template<>
template<>
void object::test<2>
()
{
    const size_t size0 = 0;
    MultiPointAutoPtr copy(dynamic_cast<geos::geom::MultiPoint*>(empty_mp_->clone().release()));
    ensure(nullptr != copy.get());

    ensure(copy->isEmpty());
    ensure(copy->isSimple());
    ensure(copy->isValid());
    ensure(copy->getCentroid() == nullptr);
    ensure_equals(copy->getNumPoints(), size0);
    ensure_equals(copy->getNumGeometries(), size0);
}

// Test of empty MultiPoint constructed by WKTReader
template<>
template<>
void object::test<3>
()
{
    const size_t size0 = 0;
    auto geo = reader_.read("MULTIPOINT EMPTY");
    MultiPointPtr mp = dynamic_cast<MultiPointPtr>(geo.get());

    ensure(mp->isEmpty());
    ensure(mp->isSimple());
    ensure(mp->isValid());
    ensure(mp->getCentroid() == nullptr);
    ensure_equals(mp->getNumPoints(), size0);
    ensure_equals(mp->getNumGeometries(), size0);
}

// Test of isEmpty() for empty MultiPoint
template<>
template<>
void object::test<4>
()
{
    ensure(empty_mp_->isEmpty());
}

// Test of isSimple() for empty MultiPoint
template<>
template<>
void object::test<5>
()
{
    ensure(empty_mp_->isSimple());
}

// Test of isValid() for empty MultiPoint
template<>
template<>
void object::test<6>
()
{
    ensure(empty_mp_->isValid());
}

// Test of getEnvelope() for empty MultiPoint
template<>
template<>
void object::test<7>
()
{
    auto envelope = empty_mp_->getEnvelope();
    ensure(envelope != nullptr);
    ensure(envelope->isEmpty());
}

// Test of getBoundary() for empty MultiPoint
template<>
template<>
void object::test<8>
()
{
    auto boundary = empty_mp_->getBoundary();
    ensure(boundary != nullptr);
    ensure(boundary->isEmpty());
}

// Test of convexHull() for empty MultiPoint
template<>
template<>
void object::test<9>
()
{
    auto hull = empty_mp_->convexHull();
    ensure(hull != nullptr);
    ensure(hull->isEmpty());
}

// Test of getGeometryTypeId() for empty MultiPoint
template<>
template<>
void object::test<10>
()
{
    ensure_equals(empty_mp_->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);
}

// Test of getGeometryType() for empty MultiPoint
template<>
template<>
void object::test<11>
()
{
    const std::string type("MultiPoint");
    ensure_equals(empty_mp_->getGeometryType(), type);
}

// Test of getDimension() for empty MultiPoint
template<>
template<>
void object::test<12>
()
{
    ensure_equals(empty_mp_->getDimension(), geos::geom::Dimension::P);
}

// Test of getBoundaryDimension() for empty MultiPoint
template<>
template<>
void object::test<13>
()
{
    ensure_equals(empty_mp_->getBoundaryDimension(), geos::geom::Dimension::False);
}

// Test of getNumPoints() for empty MultiPoint
template<>
template<>
void object::test<14>
()
{
    ensure_equals(empty_mp_->getNumPoints(), 0u);
}

// Test of getLength() for empty MultiPoint
template<>
template<>
void object::test<15>
()
{
    ensure_equals(empty_mp_->getLength(), 0.0);
}

// Test of getArea() for empty MultiPoint
template<>
template<>
void object::test<16>
()
{
    ensure_equals(empty_mp_->getArea(), 0.0);
}

// Test of isEmpty() for non-empty MultiPoint
template<>
template<>
void object::test<17>
()
{
    ensure(mp_ != nullptr);
    ensure(!mp_->isEmpty());
}

// Test of getEnvelope() for non-empty MultiPoint
template<>
template<>
void object::test<18>
()
{
    ensure(mp_ != nullptr);

    auto envelope = mp_->getEnvelope();
    ensure(envelope != nullptr);
    ensure(!envelope->isEmpty());
    ensure_equals(envelope->getDimension(), geos::geom::Dimension::A);
}

// Test of getBoundary() for non-empty MultiPoint
template<>
template<>
void object::test<19>
()
{
    ensure(mp_ != nullptr);

    auto boundary = mp_->getBoundary();
    ensure(boundary != nullptr);

    // OGC 05-126, Version: 1.1.0, Chapter 6.1.5 MultiPoint
    ensure("[OGC] The boundary of a MultiPoint is the empty set.", boundary->isEmpty());
}

// Test of convexHull() for non-empty MultiPoint
template<>
template<>
void object::test<20>
()
{
    ensure(mp_ != nullptr);

    auto hull = mp_->convexHull();
    ensure(hull != nullptr);
    ensure(!hull->isEmpty());
    ensure_equals(hull->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals(hull->getDimension(), geos::geom::Dimension::L);
}

// Test of getGeometryTypeId() for non-empty MultiPoint
template<>
template<>
void object::test<21>
()
{
    ensure(mp_ != nullptr);
    ensure_equals(mp_->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT);
}

// Test of getGeometryType() for non-empty MultiPoint
template<>
template<>
void object::test<22>
()
{
    ensure(mp_ != nullptr);

    const std::string type("MultiPoint");
    ensure_equals(mp_->getGeometryType(), type);
}

// Test of getDimension() for non-empty MultiPoint
template<>
template<>
void object::test<23>
()
{
    ensure(mp_ != nullptr);
    ensure_equals(mp_->getDimension(), geos::geom::Dimension::P);
}

// Test of getBoundaryDimension() for non-empty MultiPoint
template<>
template<>
void object::test<24>
()
{
    ensure(mp_ != nullptr);
    ensure_equals(mp_->getBoundaryDimension(), geos::geom::Dimension::False);
}

// Test of getNumPoints() for non-empty MultiPoint
template<>
template<>
void object::test<25>
()
{
    ensure(mp_ != nullptr);
    ensure_equals(mp_->getNumPoints(), mp_size_);
}

// Test of getLength() for non-empty MultiPoint
template<>
template<>
void object::test<26>
()
{
    ensure(mp_ != nullptr);
    ensure_equals(mp_->getLength(), 0.0);
}

// Test of getArea() for non-empty MultiPoint
template<>
template<>
void object::test<27>
()
{
    ensure(mp_ != nullptr);
    ensure_equals(mp_->getArea(), 0.0);
}

// Test of ParseException thrown when constructing MultiPoint from invalid WKT
template<>
template<>
void object::test<28>
()
{
    try {
        auto geo = reader_.read("MULTIPOINT(0 0, 5)");
        ensure(geo != nullptr);

        fail("ParseException expected.");
    }
    catch(geos::io::ParseException const& e) {
        const char* msg = e.what(); // ok
        ensure(msg != nullptr);
    }
}

template<>
template<>
void object::test<29>
()
{
    // getCoordinate() returns nullptr for empty geometry
    auto gf = geos::geom::GeometryFactory::create();
    std::unique_ptr<geos::geom::Geometry> g(gf->createMultiPoint());

    ensure(g->getCoordinate() == nullptr);
}

// test isDimensionStrict for empty MultiPoint
template<>
template<>
void object::test<30>
()
{
    ensure(empty_mp_->isDimensionStrict(geos::geom::Dimension::P));
    ensure(!empty_mp_->isDimensionStrict(geos::geom::Dimension::L));
}

// test isDimensionStrict for non-empty MultiPoint
template<>
template<>
void object::test<31>
()
{
    ensure(empty_mp_->isDimensionStrict(geos::geom::Dimension::P));
    ensure(!empty_mp_->isDimensionStrict(geos::geom::Dimension::L));
}

} // namespace tut

