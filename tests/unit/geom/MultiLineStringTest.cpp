//
// Test Suite for geos::geom::MultiLineString class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiLineString.h>
#include <geos/io/WKTReader.h>

#include "utility.h"

using geos::geom::MultiLineString;

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_multilinestring_data {
    std::unique_ptr<MultiLineString> empty_mls_;
    std::unique_ptr<MultiLineString> mls_;

    geos::io::WKTReader reader_;

    test_multilinestring_data() {
        empty_mls_ = reader_.read<MultiLineString>("MULTILINESTRING EMPTY");
        mls_ = reader_.read<MultiLineString>("MULTILINESTRING ((0 0, 1 1), (3 3, 4 4))");
    }
};

typedef test_group<test_multilinestring_data> group;
typedef group::object object;

group test_multilinestring_group("geos::geom::MultiLineString");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    // getCoordinate() returns nullptr for empty geometry
    ensure(empty_mls_->getCoordinate() == nullptr);
}

// test isDimensionStrict for empty MultiLineString
template<>
template<>
void object::test<2>
()
{
    ensure(empty_mls_->isDimensionStrict(geos::geom::Dimension::L));
    ensure(!empty_mls_->isDimensionStrict(geos::geom::Dimension::A));
}

// test isDimensionStrict for non-empty MultiLineString
template<>
template<>
void object::test<3>
()
{
    ensure(mls_->isDimensionStrict(geos::geom::Dimension::L));
    ensure(!mls_->isDimensionStrict(geos::geom::Dimension::A));
}


// Test of hasDimension()
template<>
template<>
void object::test<4>
()
{
    ensure(!mls_->hasDimension(geos::geom::Dimension::P));
    ensure(mls_->hasDimension(geos::geom::Dimension::L));
    ensure(!mls_->hasDimension(geos::geom::Dimension::A));
}

template<>
template<>
void object::test<5>()
{
    set_test_name("getLinearized()");

    // check that return MultiLineString*, not Geometry*
    std::unique_ptr<MultiLineString> linearized = mls_->getLinearized(1.0);

    ensure_equals_exact_geometry_xyzm(linearized.get(), mls_.get(), 0);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("getCurved");

    auto input = reader_.read<MultiLineString>("MULTILINESTRING ((3 3, 4 4), (-2 0, -1.414 1.414, 0 2, 1.414 1.414, 2 0, 2 3))");

    ensure_equals(input->getCurved(1e-9)->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);

    // check that we return GeometryCollection*, not Geometry*
    std::unique_ptr<geos::geom::GeometryCollection> curved = input->getCurved(2e-3);

    ensure_equals(curved->getGeometryTypeId(), geos::geom::GEOS_MULTICURVE);

    auto expected = reader_.read("MULTICURVE ((3 3, 4 4), COMPOUNDCURVE(CIRCULARSTRING (-2 0, 0 2, 2 0), (2 0, 2 3)))");

    ensure_equals_exact_geometry_xyzm(curved.get(), expected.get(), 1e-3);
}


} // namespace tut

