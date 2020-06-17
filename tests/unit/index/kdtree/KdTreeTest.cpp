#include <tut/tut.hpp>
// geos
#include <geos/index/kdtree/KdTree.h>
#include <geos/geom/Envelope.h>
#include <geos/io/WKTReader.h>

using namespace geos::index::kdtree;
using namespace geos::geom;

namespace tut {

// dummy data, not used
struct test_kdtree_data {

    geos::io::WKTReader reader_;

    void testQuery(std::string& wktInput, double tolerance, const Envelope& queryEnv, std::string& wktExpected, bool includeRepeated) {
        KdTree index(tolerance);
        // Read input and fill tree with it
        auto geo = reader_.read(wktInput);
        std::unique_ptr<CoordinateSequence> coords = geo->getCoordinates();
        for (std::size_t i = 0; i < coords->size(); i++) {
            index.insert(coords->getAt(i));
        }
        // Read expected output into vector of coordinates
        auto geoExpected = reader_.read(wktExpected);
        auto csExpected = geoExpected->getCoordinates();
        std::vector<Coordinate> expectedCoord;
        csExpected->toVector(expectedCoord);
        // Read tree into vector of coordinates
        std::unique_ptr<std::vector<Coordinate>> result = KdTree::toCoordinates(*(index.query(queryEnv)), includeRepeated);

        std::sort(result->begin(), result->end());
        std::sort(expectedCoord.begin(), expectedCoord.end());

        ensure("Result count not equal to expected count", result->size() == expectedCoord.size());
        ensure("Expected result coordinates not found", *result == expectedCoord);
    }

    void testQuery(std::string& wktInput, double tolerance, const Envelope& queryEnv, std::string& wktExpected) {
        testQuery(wktInput, tolerance, queryEnv, wktExpected, false);
    }

    void testQueryRepeated(std::string& wktInput, double tolerance, const Envelope& queryEnv, std::string& wktExpected) {
        testQuery(wktInput, tolerance, queryEnv, wktExpected, true);
    }

};

using group = test_group<test_kdtree_data>;
using object = group::object;

group test_kdtree_group("geos::index::kdtree::KdTree");

//
// testSinglePoint
//
template<>
template<>
void object::test<1> ()
{
    KdTree index(.001);

    KdNode* node1 = index.insert(Coordinate(1, 1));
    KdNode* node2 = index.insert(Coordinate(1, 1));

    ensure("Inserting 2 identical points should create one node", node1 == node2);

    Envelope queryEnv(0, 10, 0, 10);
    std::unique_ptr<std::vector<KdNode*>> result = index.query(queryEnv);

    ensure("query should return 1 result", result->size() == 1);

    KdNode* node = result->at(0);
    ensure("node should have two entries", node->getCount() == 2);
    ensure("node should be repeated", node->isRepeated());
}

//
// testMultiplePoint
//
template<>
template<>
void object::test<2> ()
{
    std::string wkt_in = "MULTIPOINT ((1 1), (2 2))";
    double tolerance = 0.0;
    Envelope env(0, 10, 0, 10);
    std::string wkt_out = "MULTIPOINT ((1 1), (2 2))";
    testQuery(wkt_in, tolerance, env, wkt_out);
}

//
// testSubset
//
template<>
template<>
void object::test<3> ()
{
    std::string wkt_in = "MULTIPOINT ( (1 1), (2 2), (3 3), (4 4) )";
    double tolerance = 0.0;
    Envelope env(1.5, 3.4, 1.5, 3.5);
    std::string wkt_out = "MULTIPOINT ( (2 2), (3 3) )";
    testQuery(wkt_in, tolerance, env, wkt_out);

}

//
// testToleranceFailure
//
template<>
template<>
void object::test<4> ()
{
    std::string wkt_in = "MULTIPOINT ( (0 0), (-.1 1), (.1 1) )";
    double tolerance = 1.0;
    Envelope env(-9, 9, -9, 9);
    std::string wkt_out = "MULTIPOINT ( (0 0), (-.1 1) )";
    testQuery(wkt_in, tolerance, env, wkt_out);

}

//
// testTolerance2
//
template<>
template<>
void object::test<5> ()
{
    std::string wkt_in = "MULTIPOINT ((10 60), (20 60), (30 60), (30 63))";
    double tolerance = 9.0;
    Envelope env(0,99, 0, 99);
    std::string wkt_out = "MULTIPOINT ((10 60), (20 60), (30 60))";
    testQuery(wkt_in, tolerance, env, wkt_out);

}

//
// testTolerance2_perturbedY
//
template<>
template<>
void object::test<6> ()
{
    std::string wkt_in = "MULTIPOINT ((10 60), (20 61), (30 60), (30 63))";
    double tolerance = 9.0;
    Envelope env(0,99, 0, 99);
    std::string wkt_out = "MULTIPOINT ((10 60), (20 61), (30 60))";
    testQuery(wkt_in, tolerance, env, wkt_out);

}

//
// testSnapToNearest
//
template<>
template<>
void object::test<7> ()
{
    std::string wkt_in = "MULTIPOINT ( (10 60), (20 60), (16 60))";
    double tolerance = 5.0;
    Envelope env(0,99, 0, 99);
    std::string wkt_out = "MULTIPOINT ( (10 60), (20 60), (20 60))";
    testQueryRepeated(wkt_in, tolerance, env, wkt_out);
}



//
// testSinglePoint
//
template<>
template<>
void object::test<8> ()
{
    KdTree index(.001);
    KdNode* node1 = index.insert(Coordinate(1, 1));
    KdNode* node2 = index.insert(Coordinate(1, 1));

    ensure("Inserting 2 identical points should create one node", node1 == node2);

    Envelope queryEnv(0, 10, 0, 10);
    std::unique_ptr<std::vector<KdNode*>> result = index.query(queryEnv);

    ensure(result->size() == 1);

    KdNode* node = (KdNode*)(*result)[0];
    ensure(node->getCount() == 2);
    ensure(node->isRepeated());
}




} // namespace tut

