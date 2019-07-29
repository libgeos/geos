//
// Test Suite for geos::geom::MultiLineString class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiLineString.h>
#include <geos/io/WKTReader.h>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_multilinestring_data {
    std::unique_ptr<geos::geom::Geometry> empty_mls_;
    std::unique_ptr<geos::geom::Geometry> mls_;

    geos::io::WKTReader reader_;

    test_multilinestring_data() {
        empty_mls_ = reader_.read("MULTILINESTRING EMPTY");
        mls_ = reader_.read("MULTILINESTRING ((0 0, 1 1), (3 3, 4 4))");
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

} // namespace tut

