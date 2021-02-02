#include <tut/tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/index/strtree/TemplateSTRtree.h>
#include <geos/index/ItemVisitor.h>
#include <geos/io/WKTReader.h>

#include <iostream>

using namespace geos;
using geos::index::strtree::TemplateSTRtree;
using geos::geom::Geometry;

namespace tut {
// dummy data, not used
struct test_templatestrtree_data {
    struct Grid {
        double x0 = 0;
        double y0 = 0;
        double dx = 1;
        double dy = 1;
        std::size_t nx = 10;
        std::size_t ny = 10;

        geom::Envelope getEnvelope() const {
            return {x0, x0+dx*static_cast<double>(nx),
                    y0, y0+dy*static_cast<double>(ny)};
        }
    };

    static std::vector<std::unique_ptr<geom::Point>> pointGrid(const Grid & grid) {
        std::vector<std::unique_ptr<geom::Point>> ret;

        auto gf = geom::GeometryFactory::create();
        for (std::size_t i = 0; i < grid.nx; ++i) {
            for (std::size_t j = 0; j < grid.ny; ++j) {
                geom::Coordinate c(grid.x0 + grid.dx*static_cast<double>(i),
                                   grid.y0 + grid.dy*static_cast<double>(j));
                geom::Point* pt = gf->createPoint(c);
                ret.emplace_back(pt);
            }
        }

        return ret;
    }

    template<typename TreeItemType, typename VectorItemType>
    static TemplateSTRtree<TreeItemType> makeTree(const std::vector<VectorItemType> & items) {
        TemplateSTRtree<TreeItemType> t(10);
        for (auto& g : items) {
            t.insert(g.get());
        }
        return t;
    }

#if 0
    template<typename TreeItemType>
    static TemplateSTRtree<TreeItemType> makeTree(const std::vector<TreeItemType> & items) {
        TemplateSTRtree<TreeItemType> t(10);
        for (auto& g : items) {
            t.insert(g);
        }
        return t;
    }
#endif
};

    using group = test_group<test_templatestrtree_data>;
using object = group::object;
group test_templatestrtree_group("geos::index::strtree::TemplateSTRtree");

//
// Test Cases
//

template<>
template<>
void object::test<1>() {
    Grid grid;
    grid.x0 = grid.y0 = 0;
    grid.dx = grid.dy = 1;
    grid.nx = grid.ny = 10;

    auto geoms = pointGrid(grid);
    auto tree = makeTree<const geom::Point*>(geoms);

    // Query by collecting items into a vector
    geom::Envelope qe(-0.5, 1.5, -0.5, 1.5);
    std::vector<const geom::Point*> matches;
    tree.query(qe, matches);
    ensure(matches.size() == 4);
}

template<>
template<>
void object::test<2>() {
    Grid grid;
    grid.x0 = grid.y0 = 0;
    grid.dx = grid.dy = 1;
    grid.nx = grid.ny = 10;

    auto geoms = pointGrid(grid);
    auto tree = makeTree<const geom::Point*>(geoms);

    // Query using visitor
    class SimpleTestVisitor : public index::ItemVisitor {
        public:
            std::size_t count;

            SimpleTestVisitor()
                : count(0)
                {}

            void
            visitItem(void* item) override
            {
                auto pt = static_cast<geom::Point*>(item);
                if (!pt->isEmpty())
                    count++;
            }
    };
    SimpleTestVisitor vis;
    geom::Envelope qe(-0.5, 1.5, -0.5, 1.5);
    tree.query(&qe, vis);
    ensure(vis.count == 4);
}

template<>
template<>
void object::test<3>
()
{
    Grid grid1, grid2;
    grid1.x0 = grid1.y0 = 0;
    grid1.dx = grid1.dy = 1;
    grid1.nx = grid1.ny = 10;

    grid2.x0 = grid2.y0 = 11;
    grid2.dx = grid2.dy = 1;
    grid2.nx = grid2.ny = 10;

    auto geoms1 = pointGrid(grid1);
    auto geoms2 = pointGrid(grid2);

    auto tree1 = makeTree<const geom::Point*>(geoms1);
    auto tree2 = makeTree<const geom::Point*>(geoms2);

    struct GeometryDistance {
        double operator()(const Geometry* a, const Geometry* b) {
            return a->distance(b);
        };
    };
    auto rslt = tree1.nearestNeighbour<GeometryDistance>(tree2);

    ensure_equals(rslt.first->getX(), 9.0);
    ensure_equals(rslt.first->getY(), 9.0);
    ensure_equals(rslt.second->getX(), 11.0);
    ensure_equals(rslt.second->getY(), 11.0);
}

template<>
template<>
void object::test<4>
()
{
    auto gf = geom::GeometryFactory::create();
    geos::io::WKTReader wkt(*gf);
    std::vector<std::unique_ptr<geom::Geometry>> geoms;
    geoms.emplace_back(wkt.read("LINESTRING(0 0, 10 10)"));
    geoms.emplace_back(wkt.read("LINESTRING(5 5, 15 15)"));
    geoms.emplace_back(wkt.read("LINESTRING(10 10, 20 20)"));
    geoms.emplace_back(wkt.read("LINESTRING(15 15, 25 25)"));

    auto tree = makeTree<Geometry*>(geoms);

    const std::size_t leaf_before = tree.getRoot()->getNumLeafNodes();
    const std::size_t all_before = tree.getRoot()->getNumNodes();
    ensure_equals(leaf_before, 4u);
    ensure_equals(all_before, 5u);

    tree.remove(geoms[3]->getEnvelopeInternal(), geoms[3].get());

    const std::size_t leaf_after = tree.getRoot()->getNumLeafNodes();
    const std::size_t all_after = tree.getRoot()->getNumNodes();
    ensure_equals(leaf_after, 3u);
    ensure_equals(all_after, 4u);
}

template<>
template<>
void object::test<5>
()
{
    Grid grid;
    grid.x0 = grid.y0 = 0;
    grid.dx = grid.dy = 1;
    grid.nx = grid.ny = 20;

    auto geoms = pointGrid(grid);

    // storing integers instead of geometry pointers
    TemplateSTRtree<size_t> tree;
    for (std::size_t i = 0; i < geoms.size(); i++) {
        tree.insert(*geoms[i]->getEnvelopeInternal(), i);
    }

    // Query into vector
    std::vector<size_t> hits;
    geom::Envelope queryEnv(2.5, 4.5, 2.5, 4.5);
    tree.query(queryEnv, hits);
    ensure_equals(hits.size(), 4u);

    // Get items in tree order
    {
        std::vector<size_t> orderedItems(tree.items().begin(), tree.items().end());
        ensure_equals(orderedItems.size(), geoms.size());
    }

    // Remove an item and get items in tree order
    {
        auto removed = tree.remove(grid.getEnvelope(), 17);
        ensure(removed);
        std::vector<size_t> orderedItems(tree.items().begin(), tree.items().end());
        ensure_equals(orderedItems.size(), geoms.size() - 1);
    }
}


} // namespace tut

