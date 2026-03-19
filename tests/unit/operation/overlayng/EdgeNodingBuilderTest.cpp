//
// Test Suite for geos::operation::overlayng::EdgeNodingBuilder class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/geom/CircularString.h>
#include <geos/algorithm/CircularArcIntersector.h>
#include <geos/noding/ArcIntersectionAdder.h>
#include <geos/noding/SimpleNoder.h>
#include <geos/operation/overlayng/EdgeNodingBuilder.h>

// std
#include <memory>



using namespace geos::geom;
using namespace geos::noding;
using namespace geos::operation::overlayng;
using geos::algorithm::CircularArcIntersector;
using geos::io::WKTReader;
using geos::io::WKTWriter;

namespace tut {
struct test_edgenodingbuilder_data {
    WKTReader reader_;

    PrecisionModel pm_floating{PrecisionModel::Type::FLOATING};
    GeometryFactory::Ptr factory_{GeometryFactory::create(&pm_floating)};

    std::unique_ptr<Geometry> toGeometry(const Edge& edge) const
    {
        if (edge.isCurved()) {
            return factory_->createCircularString(edge.getCoordinates());
        }
        return factory_->createLineString(edge.getCoordinates());
    }

    std::unique_ptr<Geometry> toGeometry(const std::vector<Edge*> edges) const
    {
        std::vector<std::unique_ptr<Geometry>> geoms;
        for (const auto* edge : edges) {
            geoms.push_back(toGeometry(*edge));
        }
        return factory_->createGeometryCollection(std::move(geoms));
    }

    void checkEdges(const std::string& wkt1, const std::string& wkt2, const std::string& wktExpected) const
    {
        auto geom1 = reader_.read(wkt1);
        auto geom2 = reader_.read(wkt2);

        SimpleNoder noder;
        CircularArcIntersector cai;
        ArcIntersectionAdder aia(cai);
        noder.setArcIntersector(aia);

        EdgeNodingBuilder builder(geom1->getPrecisionModel(), &noder);

        auto edges = builder.build(geom1.get(), geom2.get());
        std::unique_ptr<Geometry> expected = reader_.read(wktExpected);
        ensure_equals_geometry_xyzm(toGeometry(edges).get(), expected.get(), 1e-5);
    }
};

typedef test_group<test_edgenodingbuilder_data> group;
typedef group::object object;

group test_edgenodingbuilder_group("geos::operation::overlayng::EdgeNodingBuilder");

template<>
template<>
void object::test<1>()
{
    set_test_name("two CircularStrings");

    checkEdges(
        "CIRCULARSTRING (0 0, 1 1, 2 0)",
        "CIRCULARSTRING (0 1, 1 0, 2 1)",
        "GEOMETRYCOLLECTION ("
            "CIRCULARSTRING (0 0, 0.0340742 0.258819, 0.133975 0.5),"
            "CIRCULARSTRING (0.133975 0.5, 1 1, 1.86603 0.5),"
            "CIRCULARSTRING (1.86603 0.5, 1.96593 0.258819, 2 0),"
            "CIRCULARSTRING (0 1, 0.0340742 0.741181, 0.133975 0.5),"
            "CIRCULARSTRING (0.133975 0.5, 1 0, 1.86603 0.5),"
            "CIRCULARSTRING (1.86603 0.5, 1.96593 0.741181, 2 1)"
        ")");
}

template<>
template<>
void object::test<2>()
{
    set_test_name("CurvePolygon and LineString");

    checkEdges("CURVEPOLYGON (COMPOUNDCURVE(CIRCULARSTRING (0 0, 1 1, 2 0), (2 0, 0 0)))",
               "LINESTRING (1 0, 2 1)",
         "GEOMETRYCOLLECTION ("
                         "CIRCULARSTRING (0 0, 0.6173165676349103 0.9238795325112867, 1.7071067811865475 0.7071067811865475),"
                         "CIRCULARSTRING (1.7071067811865475 0.7071067811865475, 1.9238795325112867 0.3826834323650898, 2 0),"
                         "LINESTRING (1 0, 1.7071067811865475 0.7071067811865475),"
                         "LINESTRING (1.7071067811865475 0.7071067811865475, 2 1),"
                         "LINESTRING (0 0, 1 0),"
                         "LINESTRING (1 0, 2 0)"
                     ")");
}

}
