/******************************************************
 *   Performance tests for topological predicates
 * 
 * Tests a target geometry against grids of points, lines and polygons covering the target. 
 * Target is either a geometry from a WKT file or a set of generated sine stars increasing in size.
 * 
 * Usage: perf_topo_predicate [WKT file] [ intersects | contains | covers | touches ]
******************************************************/

#include <geos/geom/util/SineStarFactory.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/profiler.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/operation/relate/RelateOp.h>
#include <geos/operation/relateng/RelateNG.h>
#include <geos/operation/relateng/RelatePredicate.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBWriter.h>
#include <BenchmarkUtils.h>

#include <iomanip>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace geos::geom;
using geos::operation::relate::RelateOp;

std::size_t MAX_ITER = 1;
std::size_t NUM_LINES = 10000;
std::size_t NUM_LINES_PTS = 100;

#define INTERSECTS 0
#define CONTAINS 1
#define COVERS 2
#define TOUCHES 3

std::string inputFilename{""};
std::string predicateName{"intersects"};
int predicateOp = INTERSECTS;

int testRelateOpIntersects(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    for (const auto& geom : geoms) {
        auto im = RelateOp::relate(&g, geom.get());
        count += im->isIntersects();
    }
    return count;
}

int testRelateOpContains(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    for (const auto& geom : geoms) {
        auto im = RelateOp::relate(&g, geom.get());
        count += im->isContains();
    }
    return count;
}

int testRelateOpCovers(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    for (const auto& geom : geoms) {
        auto im = RelateOp::relate(&g, geom.get());
        count += im->isCovers();
    }
    return count;
}

int testRelateOpTouches(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    for (const auto& geom : geoms) {
        auto im = RelateOp::relate(&g, geom.get());
        count += im->isTouches(g.getDimension(), geom.get()->getDimension());
    }
    return count;
}

int testGeometryIntersects(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    for (const auto& geom : geoms) {
        count += g.intersects(geom.get());
    }
    return count;
}

int testGeometryContains(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    for (const auto& geom : geoms) {
        count += g.contains(geom.get());
    }
    return count;
}

int testGeometryCovers(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    for (const auto& geom : geoms) {
        count += g.covers(geom.get());
    }
    return count;
}

int testGeometryTouches(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    for (const auto& geom : geoms) {
        count += g.touches(geom.get());
    }
    return count;
}

int testPrepGeomIntersects(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    auto prep = prep::PreparedGeometryFactory::prepare(&g);
    for (const auto& geom : geoms) {
        count += prep->intersects(geom.get());
    }
    return count;
}

int testPrepGeomContains(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    auto prep = prep::PreparedGeometryFactory::prepare(&g);
    for (const auto& geom : geoms) {
        count += prep->contains(geom.get());
    }
    return count;
}

int testPrepGeomCovers(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    auto prep = prep::PreparedGeometryFactory::prepare(&g);
    for (const auto& geom : geoms) {
        count += prep->covers(geom.get());
    }
    return count;
}

int testRelateNGPreparedIntersects(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    auto prep = geos::operation::relateng::RelateNG::prepare(&g);
    for (const auto& line : geoms) {
        count += prep->evaluate(line.get(), *geos::operation::relateng::RelatePredicate::intersects());
    }
    return count;
}

int testRelateNGPreparedContains(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    auto prep = geos::operation::relateng::RelateNG::prepare(&g);
    for (const auto& line : geoms) {
        count += prep->evaluate(line.get(), *geos::operation::relateng::RelatePredicate::contains());
    }
    return count;
}

int testRelateNGPreparedCovers(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    auto prep = geos::operation::relateng::RelateNG::prepare(&g);
    for (const auto& line : geoms) {
        count += prep->evaluate(line.get(), *geos::operation::relateng::RelatePredicate::covers());
    }
    return count;
}

int testRelateNGPreparedTouches(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms) {
    int count = 0;
    auto prep = geos::operation::relateng::RelateNG::prepare(&g);
    for (const auto& line : geoms) {
        count += prep->evaluate(line.get(), *geos::operation::relateng::RelatePredicate::touches());
    }
    return count;
}

template<typename F>
double test(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& geoms, const std::string& method, F&& fun, double baseTime)
{
    geos::util::Profile sw("TopologyPredicatePerf");
    sw.start();

    int count = 0;
    for (std::size_t i = 0; i < MAX_ITER; i++) {
        count += fun(g, geoms);
    }

    sw.stop();
    double tot = sw.getTot();
    double timesFaster = baseTime == 0 ? 1 : baseTime / tot;
    std::cout << std::fixed << std::setprecision(0);
    std::cout << g.getNumPoints() << "," 
        << MAX_ITER * geoms.size() << "," 
        << count << "," << geoms[0]->getGeometryType() << "," 
        << geoms[0]->getNumPoints() << "," 
        << method << " - " << predicateName << "," 
        << tot << ",";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << timesFaster 
        << std::endl;
    return tot;
}


std::unique_ptr<Geometry> 
loadWKT(std::string& fname) {
    //std::string fname = "/Users/mdavis/proj/jts/testing/relateng/europe_main.wkt";
    std::ifstream f(fname);
    std::string line;
    std::unique_ptr<Geometry> geom;
    geos::io::WKTReader r;
    if (std::getline(f, line)) {
        if (line != "") {
            geom = r.read(line.c_str());
        }
    }
    f.close();
    return geom;
}

void testTarget(int dim, Geometry& target) {
    std::vector<std::unique_ptr<Geometry>> geoms;
    switch (dim) {
    case 0:
        geoms = geos::benchmark::createPoints(*target.getEnvelopeInternal(), NUM_LINES);
        break;
    case 1:
        geoms = geos::benchmark::createLines(*target.getEnvelopeInternal(), NUM_LINES, 1.0, NUM_LINES_PTS);
        break;
    case 2:
        geoms = geos::benchmark::createPolygons(*target.getEnvelopeInternal(), NUM_LINES, 1.0, NUM_LINES_PTS);
        break;
    }
    double baseTime;
    switch (predicateOp) {
    case INTERSECTS:
        baseTime = test(target, geoms, "RelateOp", testRelateOpIntersects, 0);
        test(target, geoms, "Geometry", testGeometryIntersects, baseTime);
        test(target, geoms, "PreparedGeom", testPrepGeomIntersects, baseTime);
        test(target, geoms, "RelateNGPrepared", testRelateNGPreparedIntersects, baseTime);
        break;
    case CONTAINS:
        baseTime = test(target, geoms, "RelateOp", testRelateOpIntersects, 0);
        test(target, geoms, "Geometry", testGeometryIntersects, baseTime);
        test(target, geoms, "PreparedGeom", testPrepGeomContains, baseTime);
        test(target, geoms, "RelateNGPrepared", testRelateNGPreparedContains, baseTime);
        break;
    case COVERS:
        baseTime = test(target, geoms, "RelateOp", testRelateOpCovers, 0);
        test(target, geoms, "Geometry", testGeometryCovers, baseTime);
        test(target, geoms, "PreparedGeom", testPrepGeomCovers, baseTime);
        test(target, geoms, "RelateNGPrepared", testRelateNGPreparedCovers, baseTime);
        break;
    case TOUCHES:
        baseTime = test(target, geoms, "RelateOp", testRelateOpTouches, 0);
        test(target, geoms, "Geometry", testGeometryTouches, baseTime);
        test(target, geoms, "RelateNGPrepared", testRelateNGPreparedTouches, baseTime);
        break;
    }
}

void testStar(int dim, std::size_t npts) {
    std::unique_ptr<Geometry> star = geos::benchmark::createSineStar({0, 0}, 100, npts);
    testTarget(dim, *star);
}

void testStarAll(int dim)
{
    testStar(dim, 5);
    testStar(dim, 10);
    testStar(dim, 500);
    testStar(dim, 1000);
    testStar(dim, 2000);
    testStar(dim, 4000);
    testStar(dim, 8000);
    testStar(dim, 16000);
}

void parseArgs(int argc, char** argv) {
    int iPred = -1;
    int iFile = -1;
    if (argc == 2) {
        iPred = 1; 
    }
    else if (argc >= 3) {
        iFile = 1;
        iPred = 2;
    }
    //predicateName = "intersects";
    if (iPred > 0) {
        predicateName = argv[iPred];
    }
    if (iFile > 0) {
        inputFilename = argv[iFile]; 
    }

    //------------------
    //-- interpret args
    
    predicateOp = INTERSECTS;
    if (predicateName == "contains") {
        predicateOp = CONTAINS;
    }
    else if (predicateName == "covers") {
        predicateOp = COVERS;
    }
    else if (predicateName == "touches") {
        predicateOp = TOUCHES;
    }
}

int main(int argc, char** argv) {
    parseArgs(argc, argv);

    std::cout << "target_points,num_tests,num_hits,test_type,pts_in_test,method,time,factor" << std::endl;

    if (inputFilename.length() == 0) {
        testStarAll(0);
        testStarAll(1);
        testStarAll(2);
    }
    else {
        std::unique_ptr<Geometry> inputGeom = loadWKT(inputFilename);
        testTarget(0, *inputGeom);
        testTarget(1, *inputGeom);
        testTarget(2, *inputGeom);
    }
}
