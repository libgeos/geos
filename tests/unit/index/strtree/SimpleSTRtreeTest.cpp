#include <tut/tut.hpp>
// geos
#include <geos/index/strtree/SimpleSTRtree.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>

using namespace geos;

namespace tut {
// dummy data, not used
struct test_simplestrtree_data {};

using group = test_group<test_simplestrtree_data>;
using object = group::object;
group test_simplestrtree_group("geos::index::strtree::SimpleSTRtree");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    index::strtree::SimpleSTRtree t(10);
    std::vector<std::unique_ptr<geom::Geometry>> geoms;
    const int gridSize = 10;

    auto gf = geom::GeometryFactory::create();
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            geom::Coordinate c((double)i, (double)j);
            geom::Point *pt = gf->createPoint(c);
            geoms.emplace_back(pt);
            t.insert(pt);
        }
    }

    geom::Envelope qe(-0.5, 1.5, -0.5, 1.5);
    std::vector<void*> matches;
    t.query(&qe, matches);
    std::cout << matches.size() << std::endl;
    ensure(matches.size() == 4);

    // std::cout << t << std::endl;
}


} // namespace tut

