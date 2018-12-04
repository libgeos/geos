//
// Test Suite for geos::algorithm::SIRtreePointInRing

#include <tut/tut.hpp>
// geos
#include <geos/geom/Coordinate.h>
#include <geos/algorithm/SIRtreePointInRing.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LinearRing.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // dummy data, not used
    struct test_sirtreepointinring_data {
        using SIRtreePointInRing = geos::algorithm::SIRtreePointInRing;
        using Geometry = geos::geom::Geometry;
        using Coordinate = geos::geom::Coordinate;

        geos::io::WKTReader reader;
        std::unique_ptr<Geometry> geom;
    };

    typedef test_group<test_sirtreepointinring_data> group;
    typedef group::object object;

    group test_sirtreepointinring_data("geos::algorithm::SIRtreePointInRing");

    //
    // Test Cases
    //

    template<>
    template<>
    void object::test<1>()
    {
        using geos::geom::LinearRing;
        geom.reset(reader.read("LINEARRING (0 0, 1 0, 1 1, 0 1, 0 0)"));

        SIRtreePointInRing pir{dynamic_cast<LinearRing*>(geom.get())};

        Coordinate c{0.5, 0.5};

        ensure(pir.isInside(c));
    }

} // namespace tut

