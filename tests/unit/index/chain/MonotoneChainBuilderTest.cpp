//
// Test Suite for geos::index::chain::MonotoneChainBuilder class.

#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainBuilder.h>


namespace tut {
// dummy data, not used
struct test_monotonechain_data {
    geos::io::WKTReader wktreader;
    geos::io::WKTWriter wktwriter;

    test_monotonechain_data()
        :
        wktreader()
    {}
};

typedef test_group<test_monotonechain_data> group;
typedef group::object object;

group test_monotonechain_group("geos::index::chain::MonotoneChainBuilder");

//
// Test Cases

// This issue exercised a bug in MonotoneChainBuilder
// https://github.com/libgeos/geos/issues/539
template<>
template<>
void object::test<1>
()
{
    std::vector< std::unique_ptr<geos::index::chain::MonotoneChain> > chain;
    geos::geom::CoordinateArraySequence pts;

    geos::index::chain::MonotoneChainBuilder::getChains(&pts, 0, chain);

    ensure_equals(chain.size(), 0u);
}

} // namespace tut
