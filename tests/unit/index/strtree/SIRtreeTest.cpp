#include <tut/tut.hpp>
// geos
#include <geos/index/strtree/SIRtree.h>

using namespace geos::index::strtree;

namespace tut {
// dummy data, not used
struct test_sirtree_data {};

using group = test_group<test_sirtree_data>;
using object = group::object;

group test_sirtree_group("geos::index::strtree::SIRtree");

//
// Test Cases
//

// Make sure no memory is leaked.
// See https://trac.osgeo.org/geos/ticket/919
template<>
template<>
void object::test<1>
()
{
    SIRtree t;
    double value = 3;
    t.insert(1, 5, &value);
}


} // namespace tut

