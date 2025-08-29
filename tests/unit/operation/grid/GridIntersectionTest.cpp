#include <tut/tut.hpp>

#include <geos/coverage/CoverageValidator.h>
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
    check_area(const Matrix<float>& actual, const Grid<bounded_extent> ext, const Geometry& input)
    {
        double tot_area = 0;
        for (float f : actual) {
            tot_area += static_cast<double>(f) * ext.dx()*ext.dy();
        }

        ensure_equals("Area does not equal input", tot_area, input.getArea(), 1e-6*tot_area);
    }

    static void
    check_subdivided_polygon(const Geometry& input, const Geometry& subdivided)
    {
        double tot_area = 0;

        std::vector<const Geometry*> components;

        for (size_t i = 0; i < subdivided.getNumGeometries(); i++) {
            const Geometry* subg = subdivided.getGeometryN(i);
            components.push_back(subg);

            ensure("subdivided component " + subg->toString() + " is invalid",subg->isValid());
            tot_area += subg->getArea();
        }

        ensure("subdivided polygons do not form a valid coverage", geos::coverage::CoverageValidator::isValid(components));

        std::string error = "subdivided polygon area does not match input: " + subdivided.toString();
        ensure_equals(error, tot_area, input.getArea(), input.getArea() * 1e-14);
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

    check_subdivided_polygon(*g, *subdivided);
}

template<>
template<>
void object::test<38>() 
{
    set_test_name("subdividing a polygon that contains the grid");

    Envelope e(0, 10, 0, 10);
    Grid<bounded_extent> ext(e, 1, 1);

    auto g = wkt_reader_.read("POINT (5 5)")->buffer(20);

    auto subd = GridIntersection::subdividePolygon(ext, *g, true);
    check_subdivided_polygon(*g, *subd);
}

template<>
template<>
void object::test<39>()
{
    set_test_name("subdividing a polygon that is identical to the grid extent");

    Envelope e(0, 10, 0, 10);
    Grid<bounded_extent> ext(e, 1, 1);

    auto g = GeometryFactory::getDefaultInstance()->toGeometry(&e);

    auto subd = GridIntersection::subdividePolygon(ext, *g, false);
    check_subdivided_polygon(*g, *subd);
}

template<>
template<>
void object::test<40>()
{
    set_test_name("multiple shells and a hole in single grid cell");

    Envelope e(0, 30, 0, 30);
    Grid<bounded_extent> ext(e, 10, 10);

    auto g = wkt_reader_.read("MULTIPOLYGON (((1 1, 15 1, 15 25, 1 25, 1 1), (12 12, 12 14, 14 14, 14 12, 12 12)), ((16 1, 25 1, 25 25, 16 25, 16 1)))");

    auto subd = GridIntersection::subdividePolygon(ext, *g, false);
    check_subdivided_polygon(*g, *subd);
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

    auto g = wkt_reader_.read("POLYGON ((4 0, 6 0, 6 2, 8 2, 6 4, 8 4, 5 7, 2 4, 4 4, 2 2, 4 2, 4 0))");
    auto rci = GridIntersection::getIntersectionFractions(ext, *g);
    check_area(*rci, ext, *g);

    auto subd = GridIntersection::subdividePolygon(ext, *g, false);
    check_subdivided_polygon(*g, *subd);
}

template<>
template<>
void object::test<46>()
{
    set_test_name("subdivide polygon whose edges follow cell boundaries (2)");

    Envelope e(0, 10, 0, 10);
    Grid<bounded_extent> ext(e, 1, 1);

    auto g = wkt_reader_.read("POLYGON ((4.5 0, 6.5 0, 6.5 2, 8.5 2, 6.5 4, 8.5 4, 5.5 7, 2.5 4, 4.5 4, 2.5 2, 4.5 2, 4.5 0))");

    auto rci = GridIntersection::getIntersectionFractions(ext, *g);
    check_area(*rci, ext, *g);

    auto subd = GridIntersection::subdividePolygon(ext, *g, false);

    check_subdivided_polygon(*g, *subd);
}

template<>
template<>
void object::test<47>()
{
    set_test_name("valid polygon coverage obtained when a traversed cell covered area ~= cell area");

    Envelope e(-180, 180, -90, 90);
    Grid<bounded_extent> ext(e, 0.5, 0.5);

    auto g = wkt_reader_.read("Polygon ((-179.99999999999991473 70.99201035500004764, -179.99999999999991473 71.05263157900003534, -179.99999999999991473 71.5366879880000397, -179.86286373599992316 71.53864166900007149, -179.91222083199991744 71.55585358300004373, -179.90074622299991347 71.55849844000005078, -179.79881751199991413 71.56907786700003271, -179.75743567599991479 71.58319733300004373, -179.73595130099991479 71.58641185100003668, -179.7154434889999095 71.58323802300003535, -179.69749915299991017 71.57733795800004373, -179.67870032499990884 71.57367584800005034, -179.61082923099991149 71.58519114800003535, -179.37205969999990884 71.56907786700003271, -179.3267716139999095 71.55548737200007281, -179.30683346299991854 71.55756256700004769, -179.28718014199992581 71.56293366100004505, -179.24286861899992118 71.56907786700003271, -179.20466061099992316 71.58319733300004373, -179.07457434799991347 71.60004303600004505, -178.73471025299991766 71.57037995000004571, -178.39484615799992184 71.54071686400004637, -178.32319088399989937 71.51837799700007281, -178.25963294199991083 71.51068756700004769, -178.30488033799991854 71.51312897300005034, -178.32347571499991545 71.51512278900003139, -178.3415421209999181 71.51752350500004241, -178.32245846299991854 71.50543854400007149, -178.21532141799991678 71.47801341400003139, -178.19347083199991744 71.47662995000007413, -178.14777584499989871 71.48517487200007281, -178.12446041599991986 71.48187897300005034, -178.00572669199991083 71.44863515800005871, -178.01720130099991479 71.44139232000003403, -178.05418860599991149 71.42877838700007942, -178.04706783799991854 71.42572663000004241, -178.03343665299991017 71.4177920590000781, -178.02623450399991611 71.41510651200007942, -178.03010006399992449 71.41347890800005871, -178.03990637899991611 71.40766022300005034, -177.97089596299991854 71.39642975500004241, -177.77985592399991788 71.33319733300004373, -177.71837317599991479 71.30524323100007678, -177.70641028599990818 71.30390045800004373, -177.68211829299991678 71.30487702000004901, -177.67027747299991347 71.30182526200007942, -177.65538489499991215 71.29315827000004901, -177.58759518099992647 71.28595612200007281, -177.5485733709999181 71.29486725500004241, -177.53111731699991083 71.29633209800005034, -177.51410885299992515 71.29340241100004505, -177.4986466139999095 71.28473541900007149, -177.50621497299991347 71.26862213700007942, -177.48700924399992118 71.25873444200004769, -177.45970618399991281 71.24990469000005078, -177.44343014199992581 71.23700592700004108, -177.4459122389999095 71.22264232000003403, -177.45775305899991281 71.20937734600005342, -177.50780188699991413 71.17377350500004241, -177.58116614499991215 71.1476097680000521, -177.63764400899989937 71.1170108090000781, -177.68415279899991788 71.11098867400005474, -177.7519018219999225 71.09296295800004373, -177.81928463399989937 71.08466217700004108, -177.87767493399991281 71.0525576840000781, -177.93049068899992449 71.04144928600004505, -178.20661373599992316 71.03839752800007545, -178.31012936099992316 71.01361725500004241, -178.59302730999991127 70.99732086800005959, -178.87592525899989937 70.98102448100007678, -178.9802953769999192 70.95066966400003139, -179.34211178299992184 70.9080264340000781, -179.33625240799992184 70.91107819200004769, -179.32225501199991413 70.9216983090000781, -179.36449133999991545 70.93024323100007678, -179.45750891799991678 70.91551341400003139, -179.50121008999991545 70.919663804000038, -179.66600501199991413 70.96548086100006003, -179.85338294199991083 70.97943756700004769, -179.88878333199991744 70.99359772300005034, -179.90754146999989871 70.99677155200004108, -179.99999999999991473 70.99201035500004764))");

    auto subd = GridIntersection::subdividePolygon(ext, *g, false);

    check_subdivided_polygon(*g, *subd);
}

template<>
template<>
void object::test<48>()
{
    set_test_name("self-touching rings force geometry to be corrected");

    Envelope e(3000000, 10000000, 525000, 6595000);
    Grid<bounded_extent> ext(e, 10000, 10000);

    auto g = wkt_reader_.read("MultiPolygon (((5196000 2052000, 5184185 2054473, 5182537 2054890, 5182796 2055916, 5182006 2056057, 5182183 2056774, 5181023 2058767, 5180374 2058127, 5180034 2058226, 5179989 2057895, 5179854 2057364, 5179674 2056658, 5179236 2056764, 5179289 2055146, 5180169 2052000, 5175958 2052000, 5175900 2068000, 5196000 2068000, 5196000 2052000),(5183832 2056356, 5183529 2055571, 5184300 2055372, 5184506 2056186, 5183832 2056356),(5179491 2062463, 5179636 2059879, 5180441 2059638, 5180438 2059666, 5180855 2061320, 5180076 2061534, 5180260 2062270, 5179491 2062463),(5181043 2062069, 5181035 2065286, 5180449.50561340618878603 2064531.85610372573137283, 5179685 2063225, 5180476 2063057, 5180260 2062270, 5181043 2062069)),((5180501 2056431, 5180341 2055640, 5179465 2055835, 5179674 2056658, 5180501 2056431)),((5180501 2056431, 5180641 2057164, 5181406 2056969, 5181239 2056302, 5180501 2056431)))	");
    auto subd = GridIntersection::subdividePolygon(ext, *g, false);

    check_subdivided_polygon(*g, *subd);
}

template<>
template<>
void object::test<49>()
{
    set_test_name("island in lake");

    Envelope e(0, 30, 0, 30 );
    Grid<bounded_extent> ext(e, 10, 10);

    auto g = wkt_reader_.read("MULTIPOLYGON (((5 5, 25 5, 25 25, 5 25, 5 5), (11 11, 11 19, 19 19, 19 11, 11 11)), ((12 12, 14 12, 14 14, 12 14, 12 12)))");

    auto rci = GridIntersection::getIntersectionFractions(ext, *g);
    check_area(*rci, ext, *g);

    auto subd = GridIntersection::subdividePolygon(ext, *g, false);

    check_subdivided_polygon(*g, *subd);
}


}
