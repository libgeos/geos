#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>
#include <utility.h>

#include <geos/operation/grid/Grid.h>
#include <geos/constants.h>

using geos::operation::grid::Grid;
using geos::operation::grid::bounded_extent;
using geos::operation::grid::infinite_extent;
using geos::geom::Envelope;

namespace tut {
struct test_grid_data : GEOSTestBase {
    const Envelope global{ -180, 180, -90, 90 };
    const Envelope infinite{geos::DoubleNegInfinity, geos::DoubleInfinity, geos::DoubleNegInfinity, geos::DoubleInfinity};
};

typedef test_group<test_grid_data, 255> group;
typedef group::object object;

group test_gridtest_group("geos::operation::grid::Grid");

template<>
template<>
void object::test<1>()
{
    set_test_name("Infinite grid dimensions calculated correctly");

    Grid<infinite_extent> grid{ global, 0.5, 0.5 , infinite };

    ensure_equals(grid.getNumRows(), 2u + 360u);
    ensure_equals(grid.getNumCols(), 2u + 720u);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("Infinite grid dimension robustness");

    Envelope extent {8.5, 16.2, 1.6, 13.1};

    Grid<infinite_extent> grid{ extent, 0.1, 0.1, infinite };

    ensure_equals(grid.getNumRows(), 2u + 115u);
    ensure_equals(grid.getNumCols(), 2u + 77u);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("Bounded grid dimensions calculated correctly");

    Grid<bounded_extent> grid{ global, 0.5, 0.5 };

    ensure_equals(grid.getNumRows(), 360u);
    ensure_equals(grid.getNumCols(), 720u);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("Bounded grid dimension robustness");

    Envelope extent {8.5, 16.2, 1.6, 13.1};
    Grid<bounded_extent> grid{extent, 0.1, 0.1 };

    ensure_equals(grid.getNumCols(), 77u);
    ensure_equals(grid.getNumRows(), 115u);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("Infinite grid index lookups are correct");

    Grid<infinite_extent> grid{ global, 1.0, 0.5, infinite };

    ensure_equals(grid.getRow(90), 1u);
    ensure_equals(grid.getRow(-89.50000001), 360u);
    ensure_equals(grid.getRow(-89.5), 360u);
    ensure_equals(grid.getRow(-90), 360u);

    ensure_equals(grid.getRow(-90.00000001), 361u);
    ensure_equals(grid.getRow(90.00000001), 0u);

    ensure_equals(grid.getColumn(-180), 1u);
    ensure_equals(grid.getColumn(-179.000001), 1u);
    ensure_equals(grid.getColumn(-179), 2u);
    ensure_equals(grid.getColumn(179), 360u);
    ensure_equals(grid.getColumn(180), 360u);

    ensure_equals(grid.getColumn(-180.0000001), 0u);
    ensure_equals(grid.getColumn(180.0000001), 361u);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("Bounded grid index lookups are correct");

    Grid<bounded_extent> grid{ global, 1.0, 0.5 };

    ensure_equals(grid.getRow(90), 0u);
    ensure_equals(grid.getRow(-89.50000001), 359u);
    ensure_equals(grid.getRow(-89.5), 359u);
    ensure_equals(grid.getRow(-90), 359u);

    ensure_THROW(grid.getRow(-90.00000001), std::exception);
    ensure_THROW(grid.getRow(90.00000001), std::exception);

    ensure_equals(grid.getColumn(-180), 0u);
    ensure_equals(grid.getColumn(-179.000001), 0u);
    ensure_equals(grid.getColumn(-179), 1u);
    ensure_equals(grid.getColumn(179), 359u);
    ensure_equals(grid.getColumn(180), 359u);

    ensure_THROW(grid.getColumn(-180.0000001), std::exception);
    ensure_THROW(grid.getColumn(180.0000001), std::exception);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("Infinite grid shrink works correctly");

    Grid<infinite_extent> grid1{ global, 1, 0.5, infinite};

    Envelope reduced{-44.3, 18.3, -21.4, 88.2};
    Grid<infinite_extent> grid2 = grid1.shrinkToFit(reduced);

    ensure_equals(grid2.xmin(), -45);
    ensure_equals(grid2.xmax(), 19);
    ensure_equals(grid2.ymin(), -21.5);
    ensure_equals(grid2.ymax(), 88.5);
    ensure_equals(grid2.dx(), grid1.dx());
    ensure_equals(grid2.dy(), grid1.dy());
}

template<>
template<>
void object::test<8>()
{
    set_test_name("Bounded grid shrink works correctly");

    Grid<bounded_extent> grid1{ global, 1, 0.5 };

    Envelope reduced{-44.3, 18.3, -21.4, 88.2};
    Grid<bounded_extent> grid2 = grid1.shrinkToFit(reduced);

    ensure_equals(grid2.xmin(), -45);
    ensure_equals(grid2.xmax(), 19);
    ensure_equals(grid2.ymin(), -21.5);
    ensure_equals(grid2.ymax(), 88.5);
    ensure_equals(grid2.dx(), grid1.dx());
    ensure_equals(grid2.dy(), grid1.dy());
}

template<>
template<>
void object::test<9>()
{
    set_test_name("Repeated shrink has no effect");

    Envelope original{-180.5, 180, -90, 90};
    Grid<bounded_extent> grid{original, 0.1, 0.1 };

    Envelope reduced{ 8.532812500000006, 16.183398437500017, 1.6762207031249972, 13.078515624999994 };

    Grid<bounded_extent> grid2 = grid.shrinkToFit(reduced);
    Grid<bounded_extent> grid3 = grid2.shrinkToFit(reduced);
    ensure_equals(grid2.getNumRows(), grid3.getNumRows());
    ensure_equals(grid2.getNumCols(), grid3.getNumCols());
}

template<>
template<>
void object::test<10>()
{
    set_test_name("Shrink robustness");

    Envelope original{-180.5, 180, -90, 90};
    Grid<bounded_extent> grid{original, 0.5, 0.5 };

    Envelope reduced{ -1.0000000000000142, 0.08749999999993818, 8.141666666665664, 9.904166666665645 };

    Grid<bounded_extent> grid2 = grid.shrinkToFit(reduced);

    ensure(reduced.getMinX() >= grid2.xmin());
    ensure(reduced.getMaxX() <= grid2.xmax());
    ensure(reduced.getMinY() >= grid2.ymin());
    ensure(reduced.getMaxY() <= grid2.ymax());
}

template<>
template<>
void object::test<11>()
{
    set_test_name("Shrink robustness (2)");

    Envelope original{-180.5, 180.5, -90.5, 90.5};
    Grid<bounded_extent> grid{original, 0.25, 0.25 };

    Envelope reduced{ 129.75833333333242, 129.7624999999993, -1.2541666666666238, -1.2499999999999964 };

    Grid<bounded_extent> grid2 = grid.shrinkToFit(reduced);

    ensure(reduced.getMinX() >= grid2.xmin());
    ensure(reduced.getMaxX() <= grid2.xmax());
    ensure(reduced.getMinY() >= grid2.ymin());
    ensure(reduced.getMaxY() <= grid2.ymax());
}

template<>
template<>
void object::test<12>()
{
    set_test_name("Shrink to empty box");

    Grid<bounded_extent> grid{ { 10, 10, 20, 20 }, 1, 1 };

    Envelope e;
    e.setToNull();

    auto reduced = grid.shrinkToFit(e);

    ensure(reduced.isEmpty());
}

template<>
template<>
void object::test<13>()
{
    set_test_name("Cell center calculations");

    Grid<bounded_extent> g1{ global, 0.5, 0.25 };
    Grid<infinite_extent> g2{ global, 0.5, 0.25, infinite};

    ensure_equals(g1.getColX(0), -179.75);
    ensure_equals(g2.getColX(1), -179.75);

    ensure_equals(g1.getRowY(0), 89.875);
    ensure_equals(g2.getRowY(1), 89.875);
}

template<>
template<>
void object::test<14>()
{
    set_test_name("Offset calculations");

    Grid<bounded_extent> g1{ global, 0.5, 0.25 };
    Grid<bounded_extent> g2{ { -170, 180, -90, 88.5 }, 0.5, 0.25 };

    // Symmetrical; we're expected to already know which grid is positively offset from the other
    ensure_equals(g1.getRowOffset(g2), 6u);
    ensure_equals(g2.getRowOffset(g1), 6u);

    ensure_equals(g1.getColOffset(g2), 20u);
    ensure_equals(g2.getColOffset(g1), 20u);
}

template<>
template<>
void object::test<15>()
{
    set_test_name("Infinite grid offset calculations");

    Grid<infinite_extent> g1{ global, 0.5, 0.25, infinite};
    Grid<infinite_extent> g2{ { -170, 180, -90, 88.5 }, 0.5, 0.25, infinite };

    // Symmetrical; we're expected to already know which grid is positively offset from the other
    ensure_equals(g1.getRowOffset(g2), 6u);
    ensure_equals(g2.getRowOffset(g1), 6u);

    ensure_equals(g1.getColOffset(g2), 20u);
    ensure_equals(g2.getColOffset(g1), 20u);
}

template<>
template<>
void object::test<16>()
{
    set_test_name("Crop bounded grid while calculating cell boundaries relative to parent");

    double res = 1.0/3601;

    Grid<bounded_extent> grid{global, res, res};

    const auto env0 = grid.getCellEnvelope(30, 30);

    Envelope cropEnv(global.getMinX() + 17*res + 1e-6, global.getMaxX(), global.getMinY(), global.getMaxY() - 17*res - 1e-6);

    auto cropped = grid.shrinkToFit(cropEnv, false);

    const auto env1 = cropped.getCellEnvelope(13, 13);

    ensure_equals("minX", env0.getMinX(), env1.getMinX());
    ensure_equals("maxX", env0.getMaxX(), env1.getMaxX());
    ensure_equals("minY", env0.getMinY(), env1.getMinY());
    ensure_equals("maxY", env0.getMaxY(), env1.getMaxY());
}

template<>
template<>
void object::test<17>()
{
    set_test_name("Crop infinite grid while calculating cell boundaries relative to parent");

    double res = 1.0/3601;

    Grid<infinite_extent> grid{global, res, res, global};

    const auto env0 = grid.getCellEnvelope(30, 30);

    Envelope cropEnv(global.getMinX() + 17*res + 1e-6, global.getMaxX(), global.getMinY(), global.getMaxY() - 17*res - 1e-6);

    auto cropped = grid.shrinkToFit(cropEnv, false);
    auto rowOffset = cropped.getRowOffset(grid);
    auto colOffset = cropped.getColOffset(grid);

    const auto env1 = cropped.getCellEnvelope(30 - rowOffset, 30 - colOffset);

    ensure_equals("minX", env0.getMinX(), env1.getMinX());
    ensure_equals("maxX", env0.getMaxX(), env1.getMaxX());
    ensure_equals("minY", env0.getMinY(), env1.getMinY());
    ensure_equals("maxY", env0.getMaxY(), env1.getMaxY());
}

}
