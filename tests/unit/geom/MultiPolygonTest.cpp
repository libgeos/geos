//
// Test Suite for geos::geom::MultiPolygon class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/io/WKTReader.h>

#include "utility.h"

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_multipolygon_data {
    geos::io::WKTReader reader_;
    std::unique_ptr<geos::geom::Geometry> empty_mp_;
    std::unique_ptr<geos::geom::Geometry> mp_;

    test_multipolygon_data() {
        empty_mp_ = reader_.read("MULTIPOLYGON EMPTY");
        mp_ = reader_.read("MULTIPOLYGON (((1 1, 2 1, 2 2, 1 1)))");
    }
};

typedef test_group<test_multipolygon_data> group;
typedef group::object object;

group test_multipolygon_group("geos::geom::MultiPolygon");

//
// Test Cases
//

// test getCoordinate() returns nullptr for empty geometry
template<>
template<>
void object::test<1>
()
{
    ensure(empty_mp_->getCoordinate() == nullptr);
}

// test isDimensionStrict for empty MultiPolygon
template<>
template<>
void object::test<2>
()
{
    ensure(empty_mp_->isDimensionStrict(geos::geom::Dimension::A));
    ensure(!empty_mp_->isDimensionStrict(geos::geom::Dimension::L));
}

// test isDimensionStrict for non-empty MultiPolygon
template<>
template<>
void object::test<3>
()
{
    ensure(mp_->isDimensionStrict(geos::geom::Dimension::A));
    ensure(!empty_mp_->isDimensionStrict(geos::geom::Dimension::L));
}

// Test of hasDimension()
template<>
template<>
void object::test<4>
()
{
    ensure(!mp_->hasDimension(geos::geom::Dimension::P));
    ensure(!mp_->hasDimension(geos::geom::Dimension::L));
    ensure(mp_->hasDimension(geos::geom::Dimension::A));
}

template<>
template<>
void object::test<5>()
{
    set_test_name("correct type returned by getLinearized");

    ensure(mp_->getLinearized(0)->getGeometryTypeId() == geos::geom::GeometryTypeId::GEOS_MULTIPOLYGON);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("getCurved");

    auto input = reader_.read(
        "MULTIPOLYGON (((0 0, 10 0, 10 10, 0 10, 0 0), (2 2, 2.292893 2.707107, 3 3, 3.707107 2.707107, 4 2, 2 2)),"
                                     "((20 0, 30 0, 30 10, 20 0)))");

    // Tolerance to small to allow conversion to MultiSurface
    ensure_equals(input->getCurved(1e-12)->getGeometryTypeId(), geos::geom::GeometryTypeId::GEOS_MULTIPOLYGON);

    auto curved = input->getCurved(0.001);

    ensure_equals(curved->getGeometryTypeId(), geos::geom::GeometryTypeId::GEOS_MULTISURFACE);

    auto expected = reader_.read("MULTISURFACE (CURVEPOLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), COMPOUNDCURVE (CIRCULARSTRING(2 2, 3 3, 4 2), (4 2, 2 2))), ((20 0, 30 0, 30 10, 20 0)))");

    ensure_equals_exact_geometry_xyzm(curved.get(), expected.get(), 1e-3);
}


} // namespace tut
