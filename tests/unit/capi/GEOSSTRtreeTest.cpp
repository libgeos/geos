//
// Test Suite for C-API GEOSSTRtree

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
#include <geos/constants.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cmath>

struct INTPOINT {
    INTPOINT(int p_x, int p_y) : x(p_x), y(p_y) {}
    int x;
    int y;
};

static GEOSGeometry*
INTPOINT2GEOS(INTPOINT* p)
{
    GEOSCoordSequence* seq = GEOSCoordSeq_create(1, 2);
    GEOSCoordSeq_setX(seq, 0, p->x);
    GEOSCoordSeq_setY(seq, 0, p->y);
    return GEOSGeom_createPoint(seq);
}

static int
INTPOINT_dist(const void* a, const void* b, double* distance, void*)
{
    INTPOINT* p1 = (INTPOINT*) a;
    INTPOINT* p2 = (INTPOINT*) b;

    int dx = p2->x - p1->x;
    int dy = p2->y - p1->y;

    *distance = sqrt(static_cast<double>(dx * dx) + dy * dy);
    return 1;
}

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capistrtree_data {
    test_capistrtree_data()
    {
        initGEOS(notice, notice);
    }

    static void
    notice(const char* fmt, ...)
    {
        std::fprintf(stdout, "NOTICE: ");

        va_list ap;
        va_start(ap, fmt);
        std::vfprintf(stdout, fmt, ap);
        va_end(ap);

        std::fprintf(stdout, "\n");
    }
};

typedef test_group<test_capistrtree_data> group;
typedef group::object object;

group test_capistrtree_group("capi::GEOSSTRtree");

//
// Test Cases
//

// Test GEOSSTRtree_nearest with a couple of points
template<>
template<>
void object::test<1>
()
{
    GEOSGeometry* g1 = GEOSGeomFromWKT("POINT (3 3)");
    GEOSGeometry* g2 = GEOSGeomFromWKT("POINT (2 7)");
    GEOSGeometry* g3 = GEOSGeomFromWKT("POINT (5 4)");
    GEOSGeometry* g4 = GEOSGeomFromWKT("POINT (3 8)");

    GEOSSTRtree* tree = GEOSSTRtree_create(2);
    GEOSSTRtree_insert(tree, g1, g1);
    GEOSSTRtree_insert(tree, g2, g2);
    GEOSSTRtree_insert(tree, g3, g3);

    const GEOSGeometry* g5 = GEOSSTRtree_nearest(tree, g4);
    ensure(g5 == g2);

    GEOSGeom_destroy(g1);
    GEOSGeom_destroy(g2);
    GEOSGeom_destroy(g3);
    GEOSGeom_destroy(g4);
    GEOSSTRtree_destroy(tree);
}

// Test GEOSSTRtree_nearest with more points.  This is important because we need to make sure the tree
// actually has a couple of layers of depth.
template<>
template<>
void object::test<2>
()
{
    size_t ngeoms = 100;
    std::vector<GEOSGeometry*> geoms;
    std::vector<GEOSGeometry*> queryPoints;
    GEOSSTRtree* tree = GEOSSTRtree_create(8);

    for(size_t i = 0; i < ngeoms; i++) {
        GEOSCoordSequence* seq = GEOSCoordSeq_create(1, 2);
        GEOSCoordSeq_setX(seq, 0, std::rand());
        GEOSCoordSeq_setY(seq, 0, std::rand());
        geoms.push_back(GEOSGeom_createPoint(seq));
        GEOSSTRtree_insert(tree, geoms[i], geoms[i]);
    }

    for(size_t i = 0; i < ngeoms; i++) {
        GEOSCoordSequence* seq = GEOSCoordSeq_create(1, 2);
        GEOSCoordSeq_setX(seq, 0, std::rand());
        GEOSCoordSeq_setY(seq, 0, std::rand());
        queryPoints.push_back(GEOSGeom_createPoint(seq));
    }

    for(size_t i = 0; i < ngeoms; i++) {
        const GEOSGeometry* nearest = GEOSSTRtree_nearest(tree, queryPoints[i]);
        const GEOSGeometry* nearestBruteForce = nullptr;
        double nearestBruteForceDistance = geos::DoubleInfinity;
        for(size_t j = 0; j < ngeoms; j++) {
            double distance;
            GEOSDistance(queryPoints[i], geoms[j], &distance);

            if(nearestBruteForce == nullptr || distance < nearestBruteForceDistance) {
                nearestBruteForce = geoms[j];
                nearestBruteForceDistance = distance;
            }
        }

        ensure(nearest == nearestBruteForce || GEOSEquals(nearest, nearestBruteForce));
    }

    for(size_t i = 0; i < ngeoms; i++) {
        GEOSGeom_destroy(geoms[i]);
        GEOSGeom_destroy(queryPoints[i]);
    }

    GEOSSTRtree_destroy(tree);
}

// GEOSSTRtree_nearest returns NULL on empty tree
template<>
template<>
void object::test<3>
()
{
    GEOSSTRtree* tree = GEOSSTRtree_create(10);
    GEOSGeometry* g1 = GEOSGeomFromWKT("POINT (3 3)");
    const GEOSGeometry* g2 = GEOSSTRtree_nearest(tree, g1);

    ensure(g2 == nullptr);

    GEOSGeom_destroy(g1);
    GEOSSTRtree_destroy(tree);
}

// GEOSSTRtree_nearest with a user-defined type
template<>
template<>
void object::test<4>
()
{
    INTPOINT p1(1, 1);
    INTPOINT p2(4, 4);
    INTPOINT p3(3, 3);

    GEOSGeometry* g1 = INTPOINT2GEOS(&p1);
    GEOSGeometry* g2 = INTPOINT2GEOS(&p2);
    GEOSGeometry* g3 = INTPOINT2GEOS(&p3);

    GEOSSTRtree* tree = GEOSSTRtree_create(4);
    GEOSSTRtree_insert(tree, g1, &p1);
    GEOSSTRtree_insert(tree, g2, &p2);

    const INTPOINT* p4 = (const INTPOINT*) GEOSSTRtree_nearest_generic(tree, &p3, g3, &INTPOINT_dist, nullptr);

    ensure(p4 == &p2);

    GEOSGeom_destroy(g1);
    GEOSGeom_destroy(g2);
    GEOSGeom_destroy(g3);
    GEOSSTRtree_destroy(tree);
}

// GEOSSTRtree_nearest with a tree of empty geometries
template<>
template<>
void object::test<5>
()
{
    GEOSGeometry* g1 = GEOSGeomFromWKT("LINESTRING EMPTY");
    GEOSGeometry* g2 = GEOSGeomFromWKT("POINT (2 7)");

    GEOSSTRtree* tree = GEOSSTRtree_create(4);
    GEOSSTRtree_insert(tree, g1, g1);

    const GEOSGeometry* g3 = GEOSSTRtree_nearest(tree, g2);
    ensure(g3 == nullptr);

    GEOSGeom_destroy(g1);
    GEOSGeom_destroy(g2);
    GEOSSTRtree_destroy(tree);
}

// GEOSSTRtree_nearest with a tree containing some empty geometries
template<>
template<>
void object::test<6>
()
{
    GEOSGeometry* g1 = GEOSGeomFromWKT("LINESTRING EMPTY");
    GEOSGeometry* g2 = GEOSGeomFromWKT("POINT (2 7)");
    GEOSGeometry* g3 = GEOSGeomFromWKT("POINT (12 97)");
    GEOSGeometry* g4 = GEOSGeomFromWKT("LINESTRING (3 8, 4 8)");

    GEOSSTRtree* tree = GEOSSTRtree_create(4);
    GEOSSTRtree_insert(tree, g1, g1);
    GEOSSTRtree_insert(tree, g2, g2);
    GEOSSTRtree_insert(tree, g3, g3);

    const GEOSGeometry* g5 = (const GEOSGeometry*) GEOSSTRtree_nearest(tree, g4);
    ensure(g5 == g2);

    GEOSGeom_destroy(g1);
    GEOSGeom_destroy(g2);
    GEOSGeom_destroy(g3);
    GEOSGeom_destroy(g4);
    GEOSSTRtree_destroy(tree);
}

// querying empty tree should not crash (see #730)
template<>
template<>
void object::test<7>
()
{
    GEOSSTRtree* tree = GEOSSTRtree_create(16);
    GEOSGeometry* q = GEOSGeomFromWKT("POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))");
    GEOSSTRtree_query(tree, q, [](void* item, void* userdata) {
        (void)(item); // make unused parameter warning go away
        (void)(userdata); // make unused parameter warning go away
    }, nullptr);

    GEOSGeom_destroy(q);
    GEOSSTRtree_destroy(tree);
}

// querying tree with box
template<>
template<>
void object::test<8>
()
{
    GEOSSTRtree* tree = GEOSSTRtree_create(10);

    GEOSGeometry* g = GEOSGeomFromWKT("POINT (2 3)");
    int payload = 876;
    GEOSSTRtree_insert(tree, g, &payload);

    GEOSGeometry* q = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");

    typedef std::vector<int*> IList;
    IList items;
    ensure_equals(items.size(), 0);
    GEOSSTRtree_query(
        tree,
        q,
        [](void* item, void* userdata) {
            IList* il = (IList*)userdata;
            il->push_back((int*)item);
        },
        &items);

    ensure_equals(items.size(), 1);

    ensure_equals(*(items[0]), payload);

    GEOSGeom_destroy(q);
    GEOSGeom_destroy(g);
    GEOSSTRtree_destroy(tree);
}


// Index a null pointer
template<>
template<>
void object::test<9>
()
{
    GEOSSTRtree* tree = GEOSSTRtree_create(10);

    GEOSGeometry* g = GEOSGeomFromWKT("POINT (2 3)");
    GEOSSTRtree_insert(tree, g, (void*)0);

    GEOSGeometry* q = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");

    typedef std::vector<int*> IList;
    IList items;
    ensure_equals(items.size(), 0);
    GEOSSTRtree_query(
        tree,
        q,
        [](void* item, void* userdata) {
            IList* il = (IList*)userdata;
            il->push_back((int*)item);
        },
        &items);

    ensure_equals(items.size(), 1);

    ensure_equals(items[0], (void*)0);

    GEOSGeom_destroy(q);
    GEOSGeom_destroy(g);
    GEOSSTRtree_destroy(tree);
}




} // namespace tut



