#include <tut/tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/index/strtree/SimpleSTRtree.h>
#include <geos/index/strtree/GeometryItemDistance.h>
#include <geos/index/ItemVisitor.h>


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
            geom::Point* pt = gf->createPoint(c);
            geoms.emplace_back(pt);
            t.insert(pt);
        }
    }

    // std::cout << t << std::endl;

    geom::Envelope qe(-0.5, 1.5, -0.5, 1.5);
    std::vector<void*> matches;
    t.query(&qe, matches);
    // std::cout << matches.size() << std::endl;
    ensure(matches.size() == 4);

    class SimpleTestVisitor: public index::ItemVisitor {
        public:
            std::size_t count;

            SimpleTestVisitor()
                : count(0)
                {}

            void
            visitItem(void* item) override
            {
                geom::Point* pt = static_cast<geom::Point*>(item);
                if (!pt->isEmpty())
                    count++;
                // std::cout << pt << std::endl;
            }
    };

    SimpleTestVisitor vis;
    t.query(&qe, vis);
    ensure(vis.count == 4);
}


template<>
template<>
void object::test<2>
()
{
    const int gridSize = 10;
    index::strtree::SimpleSTRtree t1(10);
    index::strtree::SimpleSTRtree t2(10);
    std::vector<std::unique_ptr<geom::Geometry>> geoms;

    auto gf = geom::GeometryFactory::create();
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            geom::Coordinate c1((double)i, (double)j);
            geom::Coordinate c2((double)(i+gridSize+1), (double)(j+gridSize+1));
            geom::Point *pt1 = gf->createPoint(c1);
            geom::Point *pt2 = gf->createPoint(c2);
            geoms.emplace_back(pt1);
            geoms.emplace_back(pt2);
            t1.insert(pt1);
            t2.insert(pt2);
        }
    }

    std::pair<const void*, const void*> rslt;
    index::strtree::GeometryItemDistance gi;
    rslt = t1.nearestNeighbour(t2, &gi);

    const geom::Point* g1 = static_cast<const geom::Point*>(rslt.first);
    const geom::Point* g2 = static_cast<const geom::Point*>(rslt.second);

    // std::cout << *g1 << std::endl;
    // std::cout << *g2 << std::endl;

    ensure_equals(g1->getX(), 9.0);
    ensure_equals(g1->getY(), 9.0);
    ensure_equals(g2->getX(), 11.0);
    ensure_equals(g2->getY(), 11.0);
}


} // namespace tut

