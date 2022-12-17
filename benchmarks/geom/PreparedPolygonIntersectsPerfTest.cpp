#include <geos/geom/util/SineStarFactory.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/profiler.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/io/WKBWriter.h>
#include <BenchmarkUtils.h>

using namespace geos::geom;

std::size_t MAX_ITER = 10;
std::size_t NUM_LINES = 10000;
std::size_t NUM_LINES_PTS = 100;


int testRelateOp(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& lines) {
    int count = 0;
    for (const auto& line : lines) {
        auto im = g.relate(line.get());
        count += im->isIntersects();
    }
    return count;
}

int testGeometryIntersects(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& lines) {
    int count = 0;
    for (const auto& line : lines) {
        count += g.intersects(line.get());
    }
    return count;
}

int testGeometryContains(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& lines) {
    int count = 0;
    for (const auto& line : lines) {
        count += g.contains(line.get());
    }
    return count;
}

int testPrepGeomCached(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& lines) {
    int count = 0;
    auto prep = prep::PreparedGeometryFactory::prepare(&g);
    for (const auto& line : lines) {
        count += prep->intersects(line.get());
    }
    return count;
}

template<typename F>
void test(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& lines, const std::string& method, F&& fun)
{
    geos::util::Profile sw("PreparedPolygonIntersects");
    sw.start();

    int count = 0;
    for (std::size_t i = 0; i < MAX_ITER; i++) {
        count += fun(g, lines);
    }

    sw.stop();
    std::cout << g.getNumPoints() << "," << MAX_ITER * lines.size() << "," << count << "," << lines[0]->getGeometryType() << "," << lines[0]->getNumPoints() << "," << method << "," << sw.getTot() << std::endl;
}

void test (std::size_t npts) {

    auto target = geos::benchmark::createSineStar({0, 0}, 100, npts);
    auto lines = geos::benchmark::createLines(*target->getEnvelopeInternal(), NUM_LINES, 1.0, NUM_LINES_PTS);
    auto points = geos::benchmark::createPoints(*target->getEnvelopeInternal(), NUM_LINES);

    test(*target, lines, "RelateOp", testRelateOp);
    test(*target, lines, "Geometry::intersects", testGeometryIntersects);
    test(*target, lines, "PrepGeomCached", testPrepGeomCached);
    test(*target, points, "RelateOp", testRelateOp);
    test(*target, points, "Geometry::intersects", testGeometryIntersects);
    test(*target, points, "PrepGeomCached", testPrepGeomCached);
}

int main() {
    std::cout << "target_points,num_tests,num_hits,test_type,pts_in_test,method,time" << std::endl;
    test(5);
    test(10);
    test(500);
    test(1000);
    test(2000);
    test(4000);
    test(8000);
    test(16000);
}
