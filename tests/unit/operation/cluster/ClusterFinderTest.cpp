//
// Test Suite for geos::operation::cluster::ClusterFinder classes

// tut
#include <tut/tut.hpp>
// geos
#include <geos/operation/cluster/DBSCANClusterFinder.h>
#include <geos/operation/cluster/GeometryIntersectsClusterFinder.h>
#include <geos/operation/cluster/EnvelopeIntersectsClusterFinder.h>
#include <geos/operation/cluster/GeometryDistanceClusterFinder.h>
#include <geos/io/WKTReader.h>

using geos::geom::Geometry;

template<typename T, typename U>
void ensure_geometry_equals(T&& g1, U&& g2) {
    if (!(g1->equals(&*g2))) {
        tut::ensure_equals(g1->toString(), g2->toString());
    }
}

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_cluster_data {
    geos::io::WKTReader reader;
};

typedef test_group<test_cluster_data> group;
typedef group::object object;

group test_clustertest_group("geos::operation::cluster::ClusterFinder");

template<>
template<>
void object::test<1>
()
{
    using geos::operation::cluster::GeometryIntersectsClusterFinder;

    // Adjacent squares
    auto g = reader.read(
            "GEOMETRYCOLLECTION ("
        "POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0)),"
        "POLYGON ((1 0, 1 1, 2 1, 2 0, 1 0))"
        ")"
    );

    auto clustered = GeometryIntersectsClusterFinder().clusterToCollection(g->clone());

    ensure("one cluster generated", clustered->getNumGeometries() == 1u);
    ensure_geometry_equals(clustered->getGeometryN(0), g);
}

template<>
template<>
void object::test<2>() {
    using geos::operation::cluster::GeometryIntersectsClusterFinder;
    using geos::operation::cluster::GeometryDistanceClusterFinder;

    auto g = reader.read(
            "GEOMETRYCOLLECTION ("
            "LINESTRING (0 0, 1 1),"
            "LINESTRING (5 5, 4 4),"
            "LINESTRING (0 0, -1 -1),"
            "LINESTRING (6 6, 7 7),"
            "POLYGON EMPTY,"
            "POLYGON ((0 0, 4 0, 4 4, 0 4, 0 0))"
            ")"
    );

    {
        auto t1 = GeometryIntersectsClusterFinder().clusterToCollection(g->clone());
        ensure_equals("t1: three clusters generated", t1->getNumGeometries(), 3u);
        ensure_equals(t1->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);

        ensure_geometry_equals(t1->getGeometryN(0),
                               reader.read(
                                       "GEOMETRYCOLLECTION(LINESTRING(0 0,1 1),LINESTRING(5 5,4 4),LINESTRING(0 0,-1 -1),POLYGON((0 0,4 0,4 4,0 4,0 0)))"));
        ensure_equals("t1: first cluster is a GeometryCollection",
                      t1->getGeometryN(0)->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);

        ensure_geometry_equals(t1->getGeometryN(1),
                               reader.read("GEOMETRYCOLLECTION(LINESTRING(6 6,7 7))"));
        ensure_equals("t1: second cluster is a LineString",
                      t1->getGeometryN(1)->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);


        ensure_geometry_equals(t1->getGeometryN(2),
                               reader.read("POLYGON EMPTY"));
        ensure_equals("t1: third cluster is a Polygon",
                      t1->getGeometryN(2)->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
    }

    {
        auto t3 = GeometryDistanceClusterFinder(1.4).clusterToCollection(g->clone());
        ensure_equals("t3: three clusters generated", t3->getNumGeometries(), 3u);
        ensure_equals(t3->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);

        ensure_geometry_equals(t3->getGeometryN(0),
                               reader.read(
                                       "GEOMETRYCOLLECTION(LINESTRING(0 0,1 1),LINESTRING(5 5,4 4),LINESTRING(0 0,-1 -1),POLYGON((0 0,4 0,4 4,0 4,0 0)))"));
        ensure_equals("t3: first cluster is a GeometryCollection",
                      t3->getGeometryN(0)->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);

        ensure_geometry_equals(t3->getGeometryN(1),
                               reader.read("LINESTRING(6 6,7 7)"));
        ensure_equals("t3: second cluster is a LineString",
                      t3->getGeometryN(1)->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);

        ensure_geometry_equals(t3->getGeometryN(2),
                               reader.read("POLYGON EMPTY"));
        ensure_equals("t3: third cluster is a Polygon",
                      t3->getGeometryN(2)->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
    }

    {
        auto t4 = GeometryDistanceClusterFinder(1.5).clusterToCollection(g->clone());
        ensure_equals("t4: two clusters generated", t4->getNumGeometries(), 2u);
        ensure_equals(t4->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);

        ensure_geometry_equals(t4->getGeometryN(0),
                               reader.read(
                                       "GEOMETRYCOLLECTION(LINESTRING(0 0,1 1),LINESTRING(5 5,4 4),LINESTRING(0 0,-1 -1),LINESTRING(6 6,7 7),POLYGON((0 0,4 0,4 4,0 4,0 0)))"));
        ensure_equals("t4: first cluster is a GeometryCollection",
                      t4->getGeometryN(0)->getGeometryTypeId(), geos::geom::GEOS_GEOMETRYCOLLECTION);

        ensure_geometry_equals(t4->getGeometryN(1),
                               reader.read("POLYGON EMPTY"));
        ensure_equals("t4: second cluster is a Polygon",
                      t4->getGeometryN(1)->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
    }
}

template<>
template<>
void object::test<3>() {
    using geos::operation::cluster::DBSCANClusterFinder;

    auto g = reader.read("GEOMETRYCOLLECTION ("
                         "POINT (0 0),"
                         "POINT (-1 0),"
                         "POINT (-1 -0.1),"
                         "POINT (-1 0.1),"
                         "POINT (1 0),"
                         "POINT (2 0),"
                         "POINT (3  0),"
                         "POINT ( 3 -0.1),"
                         "POINT ( 3 0.1)"
                         ")");

    DBSCANClusterFinder c(1.01, 5);

    auto result = c.clusterToCollection(*g);
    ensure_equals("two clusters generated", result->getNumGeometries(), 2u);

    ensure_geometry_equals(result->getGeometryN(0),
                           reader.read("GEOMETRYCOLLECTION ("
                                       "POINT (0 0),"
                                       "POINT (-1 0),"
                                       "POINT (-1 -0.1),"
                                       "POINT (-1 0.1),"
                                       "POINT (1 0))"));

    ensure_geometry_equals(result->getGeometryN(1),
                           reader.read("GEOMETRYCOLLECTION ("
                                       "POINT (2 0),"
                                       "POINT (3  0),"
                                       "POINT ( 3 -0.1),"
                                       "POINT ( 3 0.1))"));
}

template<>
template<>
void object::test<4>() {
    using geos::operation::cluster::DBSCANClusterFinder;

    auto g = reader.read("POINT (1 1)");

    auto result = DBSCANClusterFinder(0, 5).clusterToCollection(*g);
    ensure("minPoints = 5, no cluster generated", result->isEmpty());

    result = DBSCANClusterFinder(0, 1).clusterToCollection(*g);
    ensure("minPoints = 1, single cluster generated", result->getGeometryN(0)->equals(g.get()));
}

template<>
template<>
void object::test<5>() {
    using geos::operation::cluster::DBSCANClusterFinder;

    auto g = reader.read("POINT (1 1)");
    std::vector<const Geometry*> geom_vec = {g.get()};

    auto clusters = DBSCANClusterFinder(0, 5).cluster(geom_vec);
    ensure_equals(clusters.getNumClusters(), 0u);

    auto cluster_id_vec = clusters.getClusterIds(999);
    ensure_equals(cluster_id_vec.size(), 1u);
    ensure_equals(cluster_id_vec[0], 999u);

    clusters = DBSCANClusterFinder(0, 1).cluster(geom_vec);
    ensure_equals(clusters.getNumClusters(), 1u);

    cluster_id_vec = clusters.getClusterIds(999);
    ensure_equals(cluster_id_vec.size(), 1u);
    ensure_equals(cluster_id_vec[0], 0u);
}

} // namespace tut


