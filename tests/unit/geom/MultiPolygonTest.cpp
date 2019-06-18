//
// Test Suite for geos::geom::MultiPolygon class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/io/WKTReader.h>

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

} // namespace tut
