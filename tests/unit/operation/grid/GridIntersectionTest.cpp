#include <tut/tut.hpp>

#include <geos/operation/grid/GridIntersection.h>
#include <geos/io/WKTReader.h>
#include <tut/tut_macros.hpp>

#include <utility.h>

using namespace geos::operation::grid;
using geos::geom::CoordinateXY;
using geos::geom::Envelope;
using geos::geom::Geometry;

namespace tut {
struct test_gridintersectiontest_data : GEOSTestBase {
    geos::io::WKTReader wkt_reader_;

    static void
    check_cell_intersections(const Matrix<float>& actual, const std::vector<std::vector<float>>& v)
    {
        Matrix<float> expected{ v };

        ensure_equals(actual, expected);
    }

    static void
    check_subdivided_polygon(const Geometry& input, const Geometry& subdivided)
    {
        double tot_area = 0;

        for (size_t i = 0; i < subdivided.getNumGeometries(); i++) {
            const Geometry* subg = subdivided.getGeometryN(i);
            ensure("subdivided component " + subg->toString() + " is invalid",subg->isValid());
            tot_area += subg->getArea();
        }

        std::string error = "subdivided polygon area does not match input: " + subdivided.toString();
        ensure_equals(error, tot_area, input.getArea());
    }
};

typedef test_group<test_gridintersectiontest_data, 255> group;
typedef group::object object;

group test_gridintersectiontest_group("geos::operation::grid::GridIntersection");

template<>
template<>
void object::test<1> ()
{
    set_test_name("basic rectangle");

    Grid<bounded_extent> ex{ { 0, 3, 0, 3 }, 1, 1 }; // 3x3 grid

    auto g = wkt_reader_.read("POLYGON ((0.5 0.5, 2.5 0.5, 2.5 2.5, 0.5 2.5, 0.5 0.5))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, { { 0.25, 0.5, 0.25 }, { 0.50, 1.0, 0.50 }, { 0.25, 0.5, 0.25 } });

    // check alternate interface for rectangles
    auto rci2 = GridIntersection::getIntersectionFractions(ex, *g->getEnvelopeInternal());

    ensure_equals(*rci, *rci2);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("basic rectangular line");

    Grid<bounded_extent> ex{ { 0, 3, 0, 3 }, 1, 1 }; // 3x3 grid

    auto g = wkt_reader_.read("LINESTRING (0.5 0.5, 2.5 0.5, 2.5 2.5, 0.5 2.5, 0.5 0.5)");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, { { 1.00, 1.00, 1.00 }, { 1.00, 0, 1.00 }, { 1.00, 1.00, 1.00 } });
}

template<>
template<>
void object::test<3>()
{
    set_test_name("basic rectangle with GeometryCollection");

    Grid<bounded_extent> ex{ { 0, 3, 0, 3 }, 1, 1 }; // 3x3 grid

    auto g = wkt_reader_.read(
                 "GEOMETRYCOLLECTION ("
                 "MULTIPOLYGON (((0.5 0.5, 1.5 0.5, 1.5 1.5, 0.5 1.5, 0.5 0.5)),"
                 "              ((1.5 1.5, 2.5 1.5, 2.5 2.5, 1.5 2.5, 1.5 1.5))),"
                 "POLYGON ((0.5 1.5, 1.5 1.5, 1.5 2.5, 0.5 2.5, 0.5 1.5)),"
                 "POLYGON ((1.5 0.5, 2.5 0.5, 2.5 1.5, 1.5 1.5, 1.5 0.5)))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, { { 0.25, 0.5, 0.25 }, { 0.50, 1.0, 0.50 }, { 0.25, 0.5, 0.25 } });
}

template<>
template<>
void object::test<4>()
{
    set_test_name("unsupported geometry");

    Grid<bounded_extent> extent{ { 0, 3, 0, 3 }, 1, 1 }; // 3x3 grid

    auto g = wkt_reader_.read("POINT (3 7)");

    ensure_THROW( GridIntersection::getIntersectionFractions(extent, *g), std::exception);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("basic non-rectangle");

    Grid<bounded_extent> ex{ { 0, 3, 0, 3 }, 1, 1 }; // 3x3 grid

    auto g = wkt_reader_.read("POLYGON ((0.5 0.5, 2.5 0.5, 2.5 2.0, 2.0 2.0, 2.0 2.5, 0.5 2.5, 0.5 0.5))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, { { 0.25, 0.5, 0.00 }, { 0.50, 1.0, 0.50 }, { 0.25, 0.5, 0.25 } });
}

template<>
template<>
void object::test<6>()
{
    set_test_name("small polygon optimization");

    Grid<bounded_extent> ex{ { 0, 3, 0, 3 }, 1, 1 }; // 3x3 grid

    // small polygon entirely contained in a single cell
    auto g = wkt_reader_.read("POLYGON ((0.5 0.5, 0.6 0.5, 0.6 0.6, 0.5 0.5))");
    double g_area = g->getArea();

    auto rci =  GridIntersection::getIntersectionFractions(ex, *g);

    ensure_equals(rci->getNumRows(), 3ul);
    ensure_equals(rci->getNumCols(), 3ul);
    ensure_equals((*rci)(2, 0),  static_cast<float>(g_area));
}

template<>
template<>
void object::test<7>()
{
    set_test_name("zero-area polygon");

    Grid<bounded_extent> ex{ { 0, 3, 0, 3 }, 1, 1 }; // 3x3 grid
    auto g = wkt_reader_.read("POLYGON ((0 0, 3 3, 0 0))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } });
}

template<>
template<>
void object::test<8>()
{
    set_test_name("Small line optimization");

    Grid<bounded_extent> ex{ { 0, 3, 0, 3 }, 1, 1 }; // 3x3 grid

    // small line entirely contained in a single cell
    auto g = wkt_reader_.read("LINESTRING (0.5 0.5, 0.6 0.5, 0.6 0.6, 0.5 0.5)");
    double g_length = g->getLength();

    auto rci =  GridIntersection::getIntersectionFractions(ex, *g);

    ensure_equals(rci->getNumRows(), 3u);
    ensure_equals(rci->getNumCols(), 3u);
    ensure_equals((*rci)(2, 0), static_cast<float>(g_length));
}

template<>
template<>
void object::test<9>()
{
    set_test_name("basic line");

    Grid<bounded_extent> ex{ { 0, 3, 0, 3 }, 1, 1 }; // 3x3 grid

    auto g = wkt_reader_.read("LINESTRING (0.5 0.5, 2.5 0.5, 2.5 2.5, 0.5 2.5)");
    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, { { 0.50, 1.00, 1.00 }, { 0, 0, 1.00 }, { 0.50, 1.00, 1.00 } });
}

template<>
template<>
void object::test<10>()
{
    set_test_name("geometry extent larger than raster");
    // Process a geometry using four 3x3 tiles

    // +-----+-----+
    // |  1  |  2  |
    // +-----+-----+
    // |  3  |  4  |
    // +-----+-----+

    Envelope b3{ 0, 3, 0, 3 };
    Envelope b2 = b3;
    b2.translate(3, 3);
    Envelope b1 = b3;
    b1.translate(0, 3);
    Envelope b4 = b3;
    b4.translate(3, 0);

    Grid<bounded_extent> g1{ b1, 1, 1 };
    Grid<bounded_extent> g2{ b2, 1, 1 };
    Grid<bounded_extent> g3{ b3, 1, 1 };
    Grid<bounded_extent> g4{ b4, 1, 1 };

    auto g = wkt_reader_.read( "POLYGON ((0.5 0.5, 4.5 0.5, 4.5 5.5, 0.5 5.5, 0.5 0.5))");

    auto ll = GridIntersection::getIntersectionFractions(g3, *g);

    check_cell_intersections(*ll, { { 0.50, 1.0, 1.0 }, { 0.50, 1.0, 1.0 }, { 0.25, 0.5, 0.5 } });

    auto lr = GridIntersection::getIntersectionFractions(g4, *g);

    check_cell_intersections(*lr, { { 1.00, 0.50, 0.0 }, { 1.00, 0.50, 0.0 }, { 0.50, 0.25, 0.0 } });

    auto ur = GridIntersection::getIntersectionFractions(g2, *g);

    check_cell_intersections(*ur, { { 0.50, 0.25, 0.0 }, { 1.00, 0.50, 0.0 }, { 1.00, 0.50, 0.0 } });

    auto ul = GridIntersection::getIntersectionFractions(g1, *g);

    check_cell_intersections(*ul, { { 0.25, 0.5, 0.5 }, { 0.50, 1.0, 1.0 }, { 0.50, 1.0, 1.0 } });
}

template<>
template<>
void object::test<11>()
{
    set_test_name("geometry entirely outside raster");

    Grid<bounded_extent> ex{ { -3, 0, -3, 0 }, 1, 1 }; // 3x3 grid

    auto g = wkt_reader_.read("POLYGON ((1.5 0.5, 2.5 1.5, 1.5 2.5, 0.5 1.5, 1.5 0.5))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } });

    // check alternate pathway for rectangles
    auto rci_rect =  GridIntersection::getIntersectionFractions(ex, *g->getEnvelopeInternal());

    check_cell_intersections(*rci_rect, { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } });
}

template<>
template<>
void object::test<12>()
{
    set_test_name("Invalid geometry with detached inner ring outside raster");

    Grid<bounded_extent> ex{ {0, 3, 0, 3}, 1, 1 }; // 3x3 grid

    auto g = wkt_reader_.read("POLYGON ((1.5 0.5, 2.5 1.5, 1.5 2.5, 0.5 1.5, 1.5 0.5), (100 100, 100 101, 101 101, 100 100))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    ensure_equals(rci->getNumRows(), 3u);
    ensure_equals(rci->getNumCols(), 3u);
}

template<>
template<>
void object::test<13>()
{
    set_test_name("diagonals");

    Grid<bounded_extent> ex{ {0, 3, 0, 3}, 1, 1 }; // 3x3 grid

    auto g = wkt_reader_.read("POLYGON ((1.5 0.5, 2.5 1.5, 1.5 2.5, 0.5 1.5, 1.5 0.5))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, {
        { 0.00, 0.25, 0.00 },
        { 0.25, 1.00, 0.25 },
        { 0.00, 0.25, 0.00 },
    });
}

template<>
template<>
void object::test<14>()
{
    set_test_name("starting on cell boundary");

    // Situation found in Canada using 0.5-degree global grid
    Grid<bounded_extent> ex{ { 0, 2, 0, 2 }, 1, 1 }; // 2x2 grid

    auto g = wkt_reader_.read("POLYGON ((1 1.5, 1.5 1.5, 1.5 0.5, 0.5 0.5, 0.5 1.5, 1 1.5))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, {
        { 0.25, 0.25 },
        { 0.25, 0.25 },
    });
}

template<>
template<>
void object::test<15>()
{
    set_test_name("bouncing off boundary");

    // Situation found in Trinidad and Tobago using 0.5-degree global grid
    Grid<bounded_extent> ex{ { 0, 2, -1, 2 }, 1, 1 }; // 3x2 grid

    auto g = wkt_reader_.read("POLYGON ((0.5 1.5, 0.5 0.5, 0.5 0, 1.5 0.5, 1.5 1.5, 0.5 1.5))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, { { 0.25, 0.25 }, { 0.4375, 0.3125 }, { 0.0, 0.0 } });
}

template<>
template<>
void object::test<16>()
{
    set_test_name("bouncing off boundary (2)");

    Grid<bounded_extent> extent{ { 0, 2, 0, 2 }, 1, 1 };

    auto g = wkt_reader_.read("POLYGON ((0.5 0.5, 1.5 0.5, 1.5 1.5, 0.5 1.5, 1 1.2, 0.5 0.5))");

    ensure_NO_THROW(GridIntersection(extent, *g));
}

template<>
template<>
void object::test<17>()
{
    set_test_name("follows grid boundary");

    // Occurs on the Libya-Egypt border, for example

    Grid<bounded_extent> ex{ { 0, 3, 0, 3 }, 1, 1 };

    auto g = wkt_reader_.read("POLYGON ((0.5 0.5, 2 0.5, 2 1.5, 2 2.5, 0.5 2.5, 0.5 0.5))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, {
        { 0.25, 0.5, 0.0 },
        { 0.50, 1.0, 0.0 },
        { 0.25, 0.5, 0.0 },
    });
}

template<>
template<>
void object::test<18>()
{
    set_test_name("starts on vertical boundary, moving up");
    Grid<bounded_extent> ex{ { 0, 4, 0, 3 }, 1, 1 }; // 4x3 grid

    auto g = wkt_reader_.read("POLYGON ((3 0.5, 3 2.5, 0.5 2.5, 0.5 0.5, 3 0.5))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, {
        { 0.25, 0.5, 0.5, 0.0 },
        { 0.50, 1.0, 1.0, 0.0 },
        { 0.25, 0.5, 0.5, 0.0 },
    });
}

template<>
template<>
void object::test<19>()
{
    set_test_name("starts on vertical boundary, moving down");
    Grid<bounded_extent> ex{ { 0, 4, 0, 3 }, 1, 1 }; // 4x3 grid

    auto g = wkt_reader_.read("POLYGON ((0.5 2.5, 0.5 0.5, 3 0.5, 3 2.5, 0.5 2.5))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, {
        { 0.25, 0.5, 0.5, 0.0 },
        { 0.50, 1.0, 1.0, 0.0 },
        { 0.25, 0.5, 0.5, 0.0 },
    });
}

template<>
template<>
void object::test<20>()
{
    set_test_name("starts on vertical boundary, moving down at rightmost extent of grid");
    Grid<bounded_extent> ex{ { 0, 3, 0, 3 }, 1, 1 }; // 3x3 grid

    auto g = wkt_reader_.read("POLYGON ((3 2.5, 3 0.5, 0.5 0.5, 0.5 2.5, 3 2.5))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, {
        { 0.25, 0.5, 0.5 },
        { 0.50, 1.0, 1.0 },
        { 0.25, 0.5, 0.5 },
    });
}

template<>
template<>
void object::test<21>()
{
    set_test_name("starts on horizontal boundary, moving right");
    Grid<bounded_extent> ex{ { 0, 3, 0, 4 }, 1, 1 }; // 3x4 grid

    auto g = wkt_reader_.read("POLYGON ((0.5 1, 2.5 1, 2.5 3.5, 0.5 3.5, 0.5 1))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, { { 0.25, 0.5, 0.25 }, { 0.50, 1.0, 0.50 }, { 0.50, 1.0, 0.50 }, { 0.0, 0.0, 0.0 } });
}

template<>
template<>
void object::test<22>()
{
    set_test_name("starts on horizontal boundary, moving left");
    Grid<bounded_extent> ex{ { 0, 3, 0, 4 }, 1, 1 }; // 3x4 grid

    auto g = wkt_reader_.read("POLYGON ((2.5 3, 0.5 3, 0.5 3.5, 0.25 3.5, 0.25 0.5, 2.5 0.5, 2.5 3))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, {
        { 0.125, 0.00, 0.00 },
        { 0.750, 1.00, 0.50 },
        { 0.750, 1.00, 0.50 },
        { 0.375, 0.50, 0.25 },
    });
}

template<>
template<>
void object::test<23>()
{
    set_test_name("Regression test - Fiji");

    // Just make sure this polygon doesn't throw an exception. It caused some problems where the
    // rightmost edge was interpreted to be exactly on a cell wall.
    Grid<bounded_extent> extent{ { -180.5, 180.5, -90.5, 90.5 }, 0.5, 0.5 };

    auto g = wkt_reader_.read("MULTIPOLYGON (((178.3736000000001 -17.33992000000002, 178.71806000000007 -17.62845999999996, 178.5527099999999 -18.150590000000008, 177.93266000000008 -18.287990000000036, 177.38145999999992 -18.164319999999975, 177.28504000000007 -17.72464999999997, 177.67087 -17.381139999999974, 178.12557000000007 -17.50480999999995, 178.3736000000001 -17.33992000000002)), ((179.36414266196417 -16.801354076946836, 178.7250593629972 -17.012041674368007, 178.5968385951172 -16.63915000000003, 179.0966093629972 -16.43398427754741, 179.4135093629972 -16.379054277547382, 180.00000000000003 -16.06713266364241, 180.00000000000003 -16.555216566639146, 179.36414266196417 -16.801354076946836)), ((-179.91736938476527 -16.501783135649347, -179.99999999999997 -16.555216566639146, -179.99999999999997 -16.06713266364241, -179.79332010904858 -16.020882256741217, -179.91736938476527 -16.501783135649347)))");

    ensure_NO_THROW(GridIntersection(extent, *g));
}

template<>
template<>
void object::test<24>()
{
    set_test_name("small polygon");

    Grid<bounded_extent> ex{ { 0, 10, 0, 10 }, 10, 10 }; // Single cell

    auto g = wkt_reader_.read("POLYGON ((3 3, 4 3, 4 4, 3 4, 3 3))");

    auto rci = GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, { { 0.01f } });
}

template<>
template<>
void object::test<25>()
{
    set_test_name("Fill handled correctly");

    Grid<bounded_extent> ex{ { 0, 3, 0, 5 }, 1, 1 }; // 3x5 grid

    auto g = wkt_reader_.read("POLYGON ((0.5 0.2, 2.2 0.2, 2.2 0.4, 0.7 0.4, 0.7 2.2, 2.2 2.2, 2.2 0.6, 2.4 0.6, 2.4 4.8, 0.5 4.8, 0.5 0.2))");

    auto rci =  GridIntersection::getIntersectionFractions(ex, *g);

    check_cell_intersections(*rci, {
        { 0.40f, 0.80f, 0.32f },
        { 0.50f, 1.00f, 0.40f },
        { 0.44f, 0.80f, 0.36f },
        { 0.20f, 0.00f, 0.20f },
        { 0.22f, 0.20f, 0.12f },
    });
}

template<>
template<>
void object::test<26>()
{
    set_test_name("Robustness regression test #1");

    // This test exercises some challenging behavior where a polygon follows
    // ymin, but the grid resolution is such that ymin < (ymax - ny*dy)

    Grid<bounded_extent> extent{ { -180, 180, -90, 90 }, 1.0 / 6, 1.0 / 6 };

    auto g = wkt_reader_.read(load_resource("antarctica.wkt"));
    ensure_NO_THROW(GridIntersection(extent, *g));
}

template<>
template<>
void object::test<27>()
{
    set_test_name("Robustness regression test #2");

    // This test exercises some challenging behavior where a polygon follows
    // xmax, but the grid resolution is such that xmax < (xmin + nx*m_dx)

    Grid<bounded_extent> extent{ { -180, 180, -90, 90 }, 1.0 / 6, 1.0 / 6 };

    auto g = wkt_reader_.read(load_resource("russia.wkt"));

    ensure_NO_THROW(GridIntersection(extent, *g));
}

template<>
template<>
void object::test<29>()
{
    set_test_name("robustness regression test #3");
    // The situation in this case was causing some kind of infinite loop, ultimately exhausting memory

    Grid<bounded_extent> extent{ { 179.96666666664618, 179.99999999997954, -16.541666666669137, -16.475000000002474 }, 0.0083333333333328596, 0.0083333333333328596 };

    auto g = wkt_reader_.read("POLYGON ((179.9715827094184135 -16.5409617106119526,  180.0000000000000000 -16.5326999999999984, 179.9872884114583655 -16.5342697143554425,  179.9715827094184135 -16.5409617106119526))");
    Envelope env = g->getEnvelopeInternal()->intersection(extent.getExtent());
    extent = extent.shrinkToFit(env);

    ensure_NO_THROW( GridIntersection::getIntersectionFractions(extent, *g));
}

template<>
template<>
void object::test<30>()
{
    set_test_name("Robustness regression test #4");

    Grid<bounded_extent> extent{ { -166.84166666666667, -152.625, 66.991666666666674, 71.358333333333334 }, 0.0083333333333333332, 0.0083333333333333332 };

    auto g = wkt_reader_.read(load_resource("regression4.wkt"));

    ensure_NO_THROW(GridIntersection(extent, *g));
}

template<>
template<>
void object::test<31>()
{
    set_test_name("robustness regression test #5");

    Grid<bounded_extent> extent{ { 0, 10, 0, 10 }, 1, 1 };

    auto g = wkt_reader_.read("POINT (2 2)")->buffer(1, 30);

    ensure_NO_THROW( GridIntersection::getIntersectionFractions(extent, *g));
}

template<>
template<>
void object::test<32>()
{
    set_test_name("Robustness regression test #6");

    Grid<bounded_extent> ex{ { 145.925, 147.375, -35.525, -33.475 }, 0.05, 0.05 };

    auto g = wkt_reader_.read(load_resource("regression6.wkt"));
    GridIntersection gi(ex, *g);
    const auto& result = *gi.getResults();

    float tot = 0;

    for (size_t i = 0; i < result.getNumRows(); i++) {
        for (size_t j = 0; j < result.getNumCols(); j++) {
            tot += result(i, j);
            if (result(i, j) < 0 || result(i, j) > 1) {
                fail();
            }
        }
    }

    ensure_equals(tot, 823.0f);
}

template<>
template<>
void object::test<33>() 
{
    set_test_name("robustness regression test #7");

    Grid<bounded_extent> ex{ { 487800, 492800, 5813800, 5818800 }, 100, 100 };

    auto g = wkt_reader_.read("POLYGON ((492094.9283999996 5816959.8553, 492374.9335527361 5816811.352641133, 492374.9335527363 5816811.352641133, 492094.9283999996 5816959.8553))");
    ex = ex.shrinkToFit(*g->getEnvelopeInternal());

    double total_area = g->getArea();
    double cell_area = ex.dx() * ex.dy();
    double max_possible_frac = total_area / cell_area;

    auto result =  GridIntersection::getIntersectionFractions(ex, *g);

    for (std::size_t i = 0; i < result->getNumRows(); i++) {
        for (std::size_t j = 0; j < result->getNumCols(); j++) {
            auto frac = (*result)(i, j);
            ensure((frac >= 0 && frac <= static_cast<float>(max_possible_frac)));
        }
    }
}

template<>
template<>
void object::test<34>() {set_test_name("Processing region is empty when there are no polygons");

    Envelope raster_extent{ 0, 10, 0, 10 };

    auto g = wkt_reader_.read("POLYGON EMPTY");

    ensure_equals(GridIntersection::processingRegion(raster_extent, *g).getArea(), 0);
}

template<>
template<>
void object::test<35>() 
{
    set_test_name("Processing region is empty when all polygons are outside of it");

    Envelope raster_extent{ 40, 50, 40, 50 };

    auto g = wkt_reader_.read("MULTIPOLYGON (((60 60, 70 60, 70 70, 60 70, 60 60)), ((20 20, 30 20, 30 30, 20 30, 20 20)))");

    ensure_equals(GridIntersection::processingRegion(raster_extent, *g).getArea(), 0);
}

template<>
template<>
void object::test<36>() 
{
    set_test_name("subdividing a simple polygon");

    Envelope e(0, 10, 0, 10);
    Grid<bounded_extent> ext(e, 1, 2);

    auto g = wkt_reader_.read("POLYGON ((1 3, 9 5, 8 9, 1 3))") ;

    auto subdivided = GridIntersection::subdividePolygon(ext, *g, false);

    check_subdivided_polygon(*g, *subdivided);
}

template<>
template<>
void object::test<37>() 
{
    set_test_name("subdividing a polygon that partially intersects grid");

    Envelope e(0, 10, 0, 10);
    Grid<bounded_extent> ext(e, 1, 1);

    auto g = wkt_reader_.read("POLYGON ((8.5 8.7, 12 8, 12 12, 8 12, 8.5 8.7))");

    auto subdivided = GridIntersection::subdividePolygon(ext, *g, true);

    ensure_equals("", subdivided->getArea(), g->getArea(), 1e-8);
}

template<>
template<>
void object::test<38>() 
{
    set_test_name("subdividing a polygon that contains the grid");

    Envelope e(0, 10, 0, 10);
    Grid<bounded_extent> ext(e, 1, 1);

    auto g = wkt_reader_.read("POINT (5 5)")->buffer(20);

    auto subdivided = GridIntersection::subdividePolygon(ext, *g, true);

    ensure_equals("", subdivided->getArea(), g->getArea(), 1e-8);
}

template<>
template<>
void object::test<39>()
{
    set_test_name("subdividing a polygon that is identical to the grid extent");

    Envelope e(0, 10, 0, 10);
    Grid<bounded_extent> ext(e, 1, 1);

    auto g = GeometryFactory::getDefaultInstance()->toGeometry(&e);

    auto subdivided = GridIntersection::subdividePolygon(ext, *g, false);

    ensure_equals("", subdivided->getArea(), e.getArea(), 1e-8);
}

template<>
template<>
void object::test<40>()
{
    set_test_name("multiple shells and a hole in single grid cell");

    Envelope e(0, 30, 0, 30);
    Grid<bounded_extent> ext(e, 10, 10);

    auto g = wkt_reader_.read("MULTIPOLYGON (((1 1, 15 1, 15 25, 1 25, 1 1), (12 12, 12 14, 14 14, 14 12, 12 12)), ((16 1, 25 1, 25 25, 16 25, 16 1)))");

    auto subdivided = GridIntersection::subdividePolygon(ext, *g, false);

    ensure_equals("", subdivided->getArea(), g->getArea(), 1e-8);
}

template<>
template<>
void object::test<41>()
{
    set_test_name("small non-square hole in a single cell");

    Envelope e(0, 30, 0, 30);
    Grid<bounded_extent> ext(e, 10, 10);

    auto g = wkt_reader_.read("POLYGON ((5 5, 25 5, 25 25, 5 25, 5 5), (12 12, 12 14, 14 14, 12 12))");

    auto fracs = GridIntersection::getIntersectionFractions(ext, *g);

    float sum = 0;
    for (const auto& frac : *fracs) {
        sum += frac * 100;
    }

    ensure_equals("", static_cast<double>(sum), g->getArea(), 1e-5);
}

template<>
template<>
void object::test<42>()
{
    set_test_name("curved geometries not supported");

    Envelope e(0, 30, 0, 30);
    Grid<bounded_extent> ext(e, 10, 10);

    auto g = wkt_reader_.read("CURVEPOLYGON ("
            "COMPOUNDCURVE ((0 0, 1 4, 4 5, 4 3), CIRCULARSTRING (4 3, 2 3, 2 1, 2 0, 0 0)), "
            "CIRCULARSTRING (1.7 1, 1.6 0.5, 1.6 0.4, 1.4 0.4, 1.7 1))");

    ensure_THROW(GridIntersection::subdividePolygon(ext, *g, false), std::exception);
    ensure_THROW(GridIntersection::getIntersectionFractions(ext, *g), std::exception);
}

template<>
template<>
void object::test<43>()
{
    set_test_name("linear geometries not supported by subdividePolygon");

    Envelope e(0, 30, 0, 30);
    Grid<bounded_extent> ext(e, 10, 10);

    auto g = wkt_reader_.read("LINESTRING (0 0, 30 30)");

    ensure_THROW(GridIntersection::subdividePolygon(ext, *g, false), std::exception);
}

template<>
template<>
void object::test<44>()
{
    set_test_name("mixed-type geometries not supported by getIntersectionFractions");

    Envelope e(0, 30, 0, 30);
    Grid<bounded_extent> ext(e, 10, 10);

    auto g = wkt_reader_.read("GEOMETRYCOLLECTION( LINESTRING (0 0, 30 30), POLYGON ((5 5, 10 5, 10 10, 5 5)))");

    ensure_THROW(GridIntersection::getIntersectionFractions(ext, *g), std::exception);
}

template<>
template<>
void object::test<45>()
{
    set_test_name("subdivide polygon whose edges follow cell boundaries");

    Envelope e(0, 10, 0, 10);
    Grid<bounded_extent> ext(e, 1, 1);

    auto tree = wkt_reader_.read("POLYGON ((4 0, 6 0, 6 2, 8 2, 6 4, 8 4, 5 7, 2 4, 4 4, 2 2, 4 2, 4 0))");

    auto subd = GridIntersection::subdividePolygon(ext, *tree, false);

    check_subdivided_polygon(*tree, *subd);
}

}
