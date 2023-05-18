// tut
#include <tut/tut.hpp>

#include <geos/operation/cluster/DisjointOperation.h>
#include <geos/operation/cluster/GeometryDistanceClusterFinder.h>
#include <geos/io/WKTReader.h>

#include <utility.h>

using geos::operation::cluster::DisjointOperation;

namespace tut {

struct test_disjointoperation_data {
    geos::io::WKTReader reader_;
};

typedef test_group<test_disjointoperation_data> group;
typedef group::object object;

group test_disjointoperationtest_group("geos::operation::cluster::DisjointOperation");

template<>
template<>
void object::test<1>
()
{
    using geos::operation::cluster::GeometryDistanceClusterFinder;

    auto g = reader_.read("MULTIPOINT ((0 0), (1 0), (1 1), (2 2), (3 2), (3 3))");

    GeometryDistanceClusterFinder finder(1.0);

    // convert each cluster into an envelope
    // return each envelope as a MultiPolygon to verify that a nested collection is not created
    auto boxes = DisjointOperation(finder).processDisjointSubsets(*g, [](const Geometry& geom) {
        std::vector<std::unique_ptr<Geometry>> envelopes;
        envelopes.emplace_back(geom.getEnvelope());
        return geom.getFactory()->createMultiPolygon(std::move(envelopes));
    });

    auto expected = reader_.read("MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((2 2, 3 2, 3 3, 2 3, 2 2)))");

    ensure_equals_geometry(boxes.get(), expected.get());
}

}
