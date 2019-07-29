//
// Test Suite for geos::operation::valid::RepeatedPointRemover class.
//

// tut
#include <tut/tut.hpp>
// geos
#include <geos/operation/valid/RepeatedPointRemover.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>
#include <string>

using geos::geom::Geometry;
using geos::geom::LineString;
using geos::geom::CoordinateSequence;
using geos::operation::valid::RepeatedPointRemover;


namespace tut {
//
// Test Group
//

// Common data used by tests
    struct test_repeated_point_remover_test_data {
        geos::io::WKTReader wktreader;

        test_repeated_point_remover_test_data()
                : wktreader()
        {}

        std::unique_ptr<Geometry> readWKT(const std::string & wkt) {
            return std::unique_ptr<Geometry>(wktreader.read(wkt));
        }
    };

    typedef test_group<test_repeated_point_remover_test_data> group;
    typedef group::object object;

    group test_repeated_point_remover_group("geos::operation::valid::RepeatedPointRemover");

    template<>
    template<>
    void object::test<1>()
    {
        auto input = readWKT("LINESTRING (3 7, 8 8, 8 8, 8 8, 10 9)");
        auto expected = readWKT("LINESTRING (3 7, 8 8, 10 9)");

        std::unique_ptr<CoordinateSequence> coords(input->getCoordinates());
        auto resultCoords = RepeatedPointRemover::removeRepeatedPoints(coords.get());

        std::unique_ptr<LineString> result(input->getFactory()->createLineString(resultCoords.release()));

        ensure(expected->equalsExact(result.get()));
    }
} // namespace tut


