//
// Test Suite for C-API GEOSCluster*

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capicluster_data : public capitest::utility {
    static GEOSGeometry* construct_clusters(const GEOSGeometry* input,
                                            unsigned numClusters,
                                            unsigned* clusterIds)
    {
        GEOSGeometry* input_cloned = GEOSGeom_clone(input);
        unsigned ngeoms;
        GEOSGeometry** components = GEOSGeom_releaseCollection(input_cloned, &ngeoms);
        GEOSGeom_destroy(input_cloned);

        // collect the indices associated with each cluster
        std::vector<std::vector<unsigned>> cluster_component_ids(numClusters);
        for (unsigned i = 0; i < ngeoms; i++) {
            cluster_component_ids[clusterIds[i]].push_back(i);
        }

        std::vector<GEOSGeometry*> cluster_geoms(numClusters);

        // assemble a GeometryCollection for each cluster
        for (unsigned cluster_id = 0; cluster_id < numClusters; cluster_id++) {
            const std::vector<unsigned> component_ids = cluster_component_ids[cluster_id];

            std::vector<GEOSGeometry*> cluster_component_geoms(component_ids.size());
            for (std::size_t i = 0; i < component_ids.size(); i++) {
                cluster_component_geoms[i] = components[component_ids[i]];
            }

            cluster_geoms[cluster_id] = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION,
                                                                  cluster_component_geoms.data(),
                                                                  static_cast<unsigned>(cluster_component_geoms.size()));
        }

        GEOSFree(components);

        // combine the clusters into a single nested GeometryCollection
        return GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION,
                                         cluster_geoms.data(),
                                         static_cast<unsigned>(cluster_geoms.size()));
    }
};

typedef test_group<test_capicluster_data> group;
typedef group::object object;

group test_capicluster_group("capi::GEOSCluster");

//
// Test Cases
//
template<>
template<> void object::test<1>
()
{
    input_ = fromWKT(
                 "GEOMETRYCOLLECTION ("
                 "POINT (0 1),"
                 "LINESTRING (0 0, 0 0.1),"
                 "LINESTRING (0 0, 1.0 1.0),"
                 "POINT (0.9 1.0),"
                 "POINT (0 7))");

    {
        unsigned numClusters = 123;
        unsigned* cluster_ids = GEOSClusterEnvelopeIntersects(input_, &numClusters);
        ensure_equals("two clusters by envelope intersection", numClusters, 2u);

        GEOSGeometry* geom_result = construct_clusters(input_, numClusters, cluster_ids);
        GEOSGeometry* geom_expected = fromWKT("GEOMETRYCOLLECTION ("
                                              "  GEOMETRYCOLLECTION ("
                                              "    POINT (0 1),"
                                              "    LINESTRING (0 0, 0 0.1),"
                                              "    LINESTRING (0 0, 1.0 1.0),"
                                              "    POINT (0.9 1.0)),"
                                              "  GEOMETRYCOLLECTION ("
                                              "    POINT (0 7)))");

        ensure_geometry_equals_identical(geom_expected, geom_result);
        GEOSGeom_destroy(geom_result);
        GEOSGeom_destroy(geom_expected);

        GEOSFree(cluster_ids);
    }

    {
        unsigned numClusters = 123;
        unsigned* cluster_ids = GEOSClusterEnvelopeDistance(input_, 6, &numClusters);
        ensure_equals("one cluster by envelope distance", numClusters, 1u);

        GEOSFree(cluster_ids);
    }

    {
        unsigned numClusters = 123;
        unsigned* cluster_ids = GEOSClusterGeometryIntersects(input_, &numClusters);
        ensure_equals("four clusters by geometry intersection", numClusters, 4u);

        GEOSFree(cluster_ids);
    }

    {
        unsigned numClusters = 123;
        unsigned* cluster_ids = GEOSClusterGeometryDistance(input_, 0.2, &numClusters);
        ensure_equals("three clusters by distance", numClusters, 3u);

        GEOSFree(cluster_ids);
    }
}

template<>
template<>
void object::test<2>()
{
    input_ = fromWKT(
                 "GEOMETRYCOLLECTION ("
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

    {
        unsigned numClusters = 123;
        unsigned* cluster_ids = GEOSClusterDBSCAN(input_, 1.01, 5, &numClusters);
        ensure_equals("two clusters with minPoints = 5", numClusters, 2u);

        GEOSFree(cluster_ids);
    }

    {
        unsigned numClusters = 123;
        unsigned* cluster_ids = GEOSClusterDBSCAN(input_, 1.01, 2, &numClusters);
        ensure_equals("one cluster with minPoints = 2", numClusters, 1u);

        GEOSFree(cluster_ids);
    }

    {
        unsigned numClusters = 123;
        unsigned* cluster_ids = GEOSClusterDBSCAN(input_, 1.01, 20, &numClusters);
        ensure_equals("no clusters with minPoints = 20", numClusters, 0u);

        for (int i = 0; i < GEOSGetNumGeometries(input_); i++) {
            ensure_equals(cluster_ids[i], GEOS_CLUSTER_NONE);
        }
        GEOSFree(cluster_ids);
    }
}


} // namespace tut

