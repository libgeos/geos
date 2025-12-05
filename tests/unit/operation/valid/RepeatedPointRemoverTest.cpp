//
// Test Suite for geos::operation::valid::RepeatedPointRemover class.
//

// tut
#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/valid/RepeatedPointRemover.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>

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
struct test_repeated_point_remover_test_data
{
    geos::io::WKTReader reader;
    geos::io::WKTWriter writer;

    test_repeated_point_remover_test_data()
        : reader() {};

    std::string
    wkt(const Geometry& geom)
    {
        return writer.write(&geom);
    }

    void
    checkSimpleSequence(const std::string& input, const std::string& expected, double tolerance = 0.0)
    {
        std::unique_ptr<CoordinateSequence> inCoords = reader.readCoordinates(input);
        std::unique_ptr<CoordinateSequence> exCoords = reader.readCoordinates(expected);
        auto outCoords = RepeatedPointRemover::removeRepeatedPoints(inCoords.get(), tolerance);
        // std::cout << "output" << std::endl;
        // std::cout << outCoords->toString() << std::endl;
        // std::cout << "expected" << std::endl;
        // std::cout <<  exCoords->toString() << std::endl;
        ensure_equals(*outCoords, *exCoords);
    }

    void
    checkSequence(const std::string& input, const std::string& expected, double tolerance = 0.0)
    {
        std::unique_ptr<Geometry> inGeom = reader.read(input);
        std::unique_ptr<Geometry> exGeom = reader.read(expected);

        const CoordinateSequence* inCoords = static_cast<LineString*>(inGeom.get())->getCoordinatesRO();
        const CoordinateSequence* exCoords = static_cast<LineString*>(exGeom.get())->getCoordinatesRO();

        auto outCoords = RepeatedPointRemover::removeRepeatedPoints(inCoords, tolerance);

        ensure_equals("hasZ", exCoords->hasZ(), outCoords->hasZ());
        ensure_equals("hasM", exCoords->hasM(), outCoords->hasM());

        // std::cout << "output" << std::endl;
        // std::cout << outCoords->toString() << std::endl;
        // std::cout << "expected" << std::endl;
        // std::cout <<  exCoords->toString() << std::endl;
        ensure_equals(*outCoords, *exCoords);
    }

    void
    checkGeometry(const std::string& input, const std::string& expected, double tolerance = 0.0)
    {
        std::unique_ptr<Geometry> inGeom = reader.read(input);
        std::unique_ptr<Geometry> outGeom = RepeatedPointRemover::removeRepeatedPoints(inGeom.get(), tolerance);
        ensure(outGeom.get());

        std::unique_ptr<Geometry> exGeom = reader.read(expected);

        // std::cout << std::endl << wkt(*outGeom) << std::endl;
        // std::cout << wkt(*exGeom) << std::endl;
        ensure_equals_geometry(outGeom.get(), exGeom.get());
    }

};

typedef test_group<test_repeated_point_remover_test_data> group;
typedef group::object object;

group test_repeated_point_remover_group("geos::operation::valid::RepeatedPointRemover");

template<>
template<>
void object::test<1>()
{
    checkSimpleSequence(
        "(3 7, 8 8, 8 8, 8 8, 10 9)",
        "(3 7, 8 8, 10 9)",
        0.0
        );
}

template<>
template<>
void object::test<2>()
{
    checkSimpleSequence(
        "(3 7, 8 8, 8 8, 8 8)",
        "(3 7, 8 8)",
        0.0
        );
}

// CoordinateSequences just retain each coordinate within
// the filter tolerance
template<>
template<>
void object::test<3>()
{
    checkSimpleSequence(
        "(0 0, 1 0, 4 0, 5 0)",
        "(0 0, 4 0)",
        3.0
        );
}

// Linestrings note the last point and somehow
// retain it in preference over the internal point
template<>
template<>
void object::test<4>()
{
    checkGeometry(
        "LINESTRING (0 0, 1 0, 4 0, 5 0)",
        "LINESTRING (0 0, 5 0)",
        3
        );
}


template<>
template<>
void object::test<5>()
{
    checkGeometry(
        "MULTIPOLYGON (((0 0, 9 0, 10 0, 10 10, 0 10, 0 1, 0 0)))",
        "MULTIPOLYGON (((0 0, 9 0, 10 10, 0 10, 0 0)))",
        3.0
        );
}


// Dimension is preserved during reduction
template<>
template<>
void object::test<6>()
{
    checkSequence("LINESTRING M EMPTY", "LINESTRING M EMPTY", 0.0);
    checkSequence("LINESTRING M (1 1 1, 2 2 2, 2 2 3, 3 3 3)", "LINESTRING M (1 1 1, 2 2 2, 3 3 3)", 0.0);
    checkSequence("LINESTRING ZM (1 2 3 4, 5 6 7 8, 5 6 9 9, 10 11 12 13)", "LINESTRING ZM (1 2 3 4, 5 6 7 8, 10 11 12 13)", 0.5);
}


// Removing from a sequence with enough tolerance
// results in single-entry sequence
template<>
template<>
void object::test<7>()
{
    checkSimpleSequence(
        "(3 7, 3 7, 3 7, 3 7)",
        "(3 7)",
        0.0
        );
}

// Removing from a sequence with enough tolerance
// results in single-entry sequence
template<>
template<>
void object::test<8>()
{
    checkSimpleSequence(
        "(3 7, 3.1 7.1, 3.2 7.2, 3.3 7.3)",
        "(3 7)",
        1.0
        );
}


template<>
template<>
void object::test<9>()
{
    checkGeometry(
        "LINESTRING (0 0, 0 1, 0 2, 0 3)",
        "LINESTRING EMPTY",
        14.0
        );
}


// small hole should collapse away
template<>
template<>
void object::test<10>()
{
    checkGeometry(
        "POLYGON ((0 0, 9 0, 10 0, 10 10, 0 10, 0 1, 0 0), (5 5, 5 6, 6 6, 6 5, 5 5))",
        "POLYGON ((0 0, 9 0, 10 10, 0 10, 0 0))",
        3.0
        );
}

// small exterior ring should disappear whole polygon
template<>
template<>
void object::test<11>()
{
    checkGeometry(
        "POLYGON ((0 0, 9 0, 10 0, 10 10, 0 10, 0 1, 0 0))",
        "POLYGON ((0 0, 10 10, 0 0))",
        12.0
        );
}

template<>
template<>
void object::test<12>()
{
    checkGeometry(
        "POLYGON ((0 0, 9 0, 10 0, 10 10, 0 10, 0 1, 0 0))",
        "POLYGON EMPTY",
        22.0
        );
}

// Careful not to replace invalid coordinates
template<>
template<>
void object::test<13>()
{
    checkGeometry(
        "LINESTRING (0 0, 0 Inf, 1 1, Inf 0)",
        "LINESTRING (0 0, 1 1)",
        1.0
        );
}

// If it filters down to just one point, it should be empty
template<>
template<>
void object::test<14>()
{
    checkGeometry(
        "LINESTRING (0 0, 0 Inf, 1 1)",
        "LINESTRING EMPTY",
        2.0
        );
}

// Filter out invalid coordinate, even at start/ends
template<>
template<>
void object::test<15>()
{
    checkGeometry(
        "POLYGON ((Inf Inf, 0 0, 10 0, 10 10, 0 10, 0 0, Inf Inf))",
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
        2.0
        );
}

// If it filters down to just one point, it should be empty
template<>
template<>
void object::test<16>()
{
    checkGeometry(
        "POLYGON ((Inf Inf, 0 0, 10 0, 10 10, 0 10, 0 0, Inf Inf))",
        "POLYGON EMPTY",
        22.0
        );
}

// https://github.com/libgeos/geos/issues/1293
// Hole collapses, should not error out.
template<>
template<>
void object::test<17>()
{
    checkGeometry(
        "POLYGON ((139770.26822331376024522 188334.00010800323798321, 139769.5 188338.01162790699163452, 139769.5 188338.3723930635896977, 139769.5 188338.5, 139769.81343283582828008 188338.5, 139770.375 188339.375, 139772.39924806414637715 188340.26989983080420643, 139770.26822331376024522 188334.00010800323798321),(139769.75256541155977175 188338.40516005983226933, 139769.75256541153066792 188338.40516005983226933, 139769.75256541153066792 188338.4051600598031655, 139769.75256541155977175 188338.40516005983226933))",
        "POLYGON ((139769.5 188338.011627907, 139769.5 188338.3723930636, 139769.5 188338.5, 139769.81343283583 188338.5, 139770.375 188339.375, 139772.39924806415 188340.2698998308, 139770.26822331376 188334.00010800324, 139769.5 188338.011627907))",
        1e-8
        );
}


} // namespace tut


