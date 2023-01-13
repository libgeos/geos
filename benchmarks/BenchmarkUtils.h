#include <geos/geom/Coordinate.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/util/SineStarFactory.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/util.h>

#include <random>
#include <vector>

namespace geos {
namespace benchmark {

std::unique_ptr<geom::Polygon>
createSineStar(const geom::CoordinateXY& origin, double size, std::size_t npts) {
    geom::util::SineStarFactory gsf(geom::GeometryFactory::getDefaultInstance());
    gsf.setCentre(origin);
    gsf.setSize(size);
    gsf.setNumPoints(static_cast<std::uint32_t>(npts));
    gsf.setArmLengthRatio(0.1);
    gsf.setNumArms(50);

    return gsf.createSineStar();
}

template<typename Generator>
std::vector<std::unique_ptr<geom::Geometry>>
createGeometriesOnGrid(const geom::Envelope& env, std::size_t nItems, Generator&& g) {
    int nCells = static_cast<int>(std::sqrt(nItems));

    std::vector<std::unique_ptr<geom::Geometry>> geoms;
    double width = env.getWidth();
    double xInc = width / nCells;
    double yInc = width / nCells;

    double x0 = env.getMinX();
    double y0 = env.getMinY();

    for(int i = 0; i < nCells; i++) {
        for(int j = 0; j < nCells; j++) {
            geom::CoordinateXY base(x0 + i*xInc, y0 + j*yInc);
            geoms.push_back(g(base));
        }
    }

    return geoms;
}

std::unique_ptr<geom::Geometry>
createLine(const geom::CoordinateXY& base, double size, std::size_t npts) {
    geom::util::SineStarFactory gsf(geom::GeometryFactory::getDefaultInstance());
    gsf.setCentre(base);
    gsf.setSize(size);
    gsf.setNumPoints(static_cast<std::uint32_t>(npts));

    auto circle = gsf.createSineStar();

    return circle->getBoundary();
}


std::vector<std::unique_ptr<geom::Geometry>>
createLines(const geom::Envelope& env, std::size_t nItems, double size, std::size_t npts) {
    return createGeometriesOnGrid(env, nItems, [size, npts](const geom::CoordinateXY& base) {
        return createLine(base, size, npts);
    });
}

std::vector<std::unique_ptr<geom::Geometry>>
createPoints(const geom::Envelope& env, std::size_t nItems) {
    return createGeometriesOnGrid(env, nItems, [](const geom::CoordinateXY& base) {
        return geom::GeometryFactory::getDefaultInstance()->createPoint(base);
    });
}

std::unique_ptr<geom::CoordinateSequence>
createRandomCoords(const geom::Envelope& env, std::size_t npts, std::default_random_engine& e) {
    auto ret = detail::make_unique<geom::CoordinateSequence>(npts, true, true, false);

    std::uniform_real_distribution<> xdist(env.getMinX(), env.getMaxX());
    std::uniform_real_distribution<> ydist(env.getMinY(), env.getMaxY());
    std::uniform_real_distribution<> zdist(0, 1);
    std::uniform_real_distribution<> mdist(0, 1);

    for (std::size_t i = 0;i < npts; i++) {
        ret->setAt(geom::CoordinateXYZM(xdist(e), ydist(e), zdist(e), mdist(e)), i);
    }

    return ret;
}

}
}

