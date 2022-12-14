#include <geos/geom/util/SineStarFactory.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/profiler.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/io/WKBWriter.h>

using namespace geos::geom;

std::size_t MAX_ITER = 10;
std::size_t NUM_LINES = 10000;
std::size_t NUM_LINES_PTS = 100;

std::unique_ptr<Polygon>
createSineStar(const CoordinateXY& origin, double size, std::size_t npts) {
    util::SineStarFactory gsf(GeometryFactory::getDefaultInstance());
    gsf.setCentre(origin);
    gsf.setSize(size);
    gsf.setNumPoints(static_cast<std::uint32_t>(npts));
    gsf.setArmLengthRatio(0.1);
    gsf.setNumArms(50);

    return gsf.createSineStar();
}

std::unique_ptr<Geometry>
createLine(const CoordinateXY& base, double size, std::size_t npts) {
    util::SineStarFactory gsf(GeometryFactory::getDefaultInstance());
    gsf.setCentre(base);
    gsf.setSize(size);
    gsf.setNumPoints(static_cast<std::uint32_t>(npts));

    auto circle = gsf.createSineStar();

    return circle->getBoundary();
}


std::vector<std::unique_ptr<Geometry>>
createLines(const Envelope& env, std::size_t nItems, double size, std::size_t npts) {
    int nCells = static_cast<int>(std::sqrt(nItems));

    std::vector<std::unique_ptr<Geometry>> geoms;
    double width = env.getWidth();
    double xInc = width / nCells;
    double yInc = width / nCells;

    double x0 = env.getMinX();
    double y0 = env.getMinY();

    for(int i = 0; i < nCells; i++) {
        for(int j = 0; j < nCells; j++) {
            CoordinateXY base(x0 + i*xInc, y0 + j*yInc);
            auto line = createLine(base, size, npts);
            geoms.push_back(std::move(line));
        }
    }

    return geoms;
}

int testRelateOp(std::size_t iter, const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& lines) {
    if (iter == 0) {
        std::cout << "Using RelateOp" << std::endl;
    }
    int count = 0;
    for (const auto& line : lines) {
        auto im = g.relate(line.get());
        count += im->isIntersects();
    }
    return count;
}

int testGeometryIntersects(std::size_t iter, const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& lines) {
    if (iter == 0) {
        std::cout << "Using Geometry::intersects" << std::endl;
    }
    int count = 0;
    for (const auto& line : lines) {
        count += g.intersects(line.get());
    }
    return count;
}

int testPrepGeomCached(std::size_t iter, const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& lines) {
    if (iter == 0) {
        std::cout << "Using cached Prepared Geometry" << std::endl;
    }
    int count = 0;
    auto prep = prep::PreparedGeometryFactory::prepare(&g);
    for (const auto& line : lines) {
        count += prep->intersects(line.get());
    }
    return count;
}

template<typename F>
void test(const Geometry& g, const std::vector<std::unique_ptr<Geometry>>& lines, F&& fun)
{
    std::cout << "AOI # pts: " << g.getNumPoints() << " # lines " << lines.size() << " # pts in line: " << NUM_LINES_PTS << std::endl;

    geos::util::Profile sw("PreparedPolygonIntersects");
    std::cout << std::endl;
    sw.start();

    int count = 0;
    for (std::size_t i = 0; i < MAX_ITER; i++) {
        count += fun(i, g, lines);
    }

    sw.stop();
    std::cout << "Count of intersections = " << count << std::endl;
    std::cout << "Finished in " << sw.getTotFormatted() << std::endl;
}

void test (std::size_t npts) {

    auto target = createSineStar({0, 0}, 100, npts);
    auto lines = createLines(*target->getEnvelopeInternal(), NUM_LINES, 1.0, NUM_LINES_PTS);

    test(*target, lines, testRelateOp);
    test(*target, lines, testGeometryIntersects);
    test(*target, lines, testPrepGeomCached);
}

int main() {
    test(5);
    test(10);
    test(500);
    test(1000);
    test(2000);
    test(4000);
}
