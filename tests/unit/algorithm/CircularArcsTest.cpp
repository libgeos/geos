#include <tut/tut.hpp>

#include <geos/geom/Coordinate.h>
#include <geos/algorithm/CircularArcs.h>

using geos::geom::CoordinateXY;
using geos::algorithm::CircularArcs;
using geos::geom::Envelope;

namespace tut {

struct test_circulararcs_data {
    const double eps = 1e-8;

    void checkEnvelope(const CoordinateXY& p0, const CoordinateXY& p1, const CoordinateXY& p2,
                       double xmin, double ymin, double xmax, double ymax)
    {
        {
            Envelope e;
            CircularArcs::expandEnvelope(e, p0, p1, p2);

            ensure_equals("p0-p1-p2 xmin", e.getMinX(), xmin, eps);
            ensure_equals("p0-p1-p2 xmax", e.getMaxX(), xmax, eps);
            ensure_equals("p0-p1-p2 ymin", e.getMinY(), ymin, eps);
            ensure_equals("p0-p1-p2 ymax", e.getMaxY(), ymax, eps);
        }

        {
            Envelope e;
            CircularArcs::expandEnvelope(e, p2, p1, p0);

            ensure_equals("p2-p1-p0 xmin", e.getMinX(), xmin, eps);
            ensure_equals("p2-p1-p0 xmax", e.getMaxX(), xmax, eps);
            ensure_equals("p2-p1-p0 ymin", e.getMinY(), ymin, eps);
            ensure_equals("p2-p1-p0 ymax", e.getMaxY(), ymax, eps);
        }
    }
};

using group = test_group<test_circulararcs_data>;
using object = group::object;

group test_circulararcs_group("geos::algorithm::CircularArcs");

template<>
template<>
void object::test<1>()
{
    CoordinateXY p0{0, 10};
    CoordinateXY p1{100, 110};
    CoordinateXY p2{200, 10};

    auto center = CircularArcs::getCenter(p0, p1, p2);

    ensure_equals(center, CoordinateXY{100, 10});
}

template<>
template<>
void object::test<2>()
{
    CoordinateXY p0{0, 0};
    CoordinateXY p1{1, 1};
    CoordinateXY p2{0, 2};

    auto center = CircularArcs::getCenter(p0, p1, p2);

    ensure_equals(center, CoordinateXY{0, 1});
}

template<>
template<>
void object::test<3>()
{
    CoordinateXY p0{54.22, 31.8};
    CoordinateXY p1{16.07, 11.9};
    CoordinateXY p2{12.22, 3.99};

    auto center = CircularArcs::getCenter(p0, p1, p2);

    ensure(center.distance(CoordinateXY{52.0123, -10.486}) < 1e-4);
}

// complete circle
template<>
template<>
void object::test<4>()
{
    CoordinateXY p0{3, 4};
    CoordinateXY p1{7, 8};
    CoordinateXY p2{3, 4};

    auto center = CircularArcs::getCenter(p0, p1, p2);

    ensure_equals(center, CoordinateXY{5, 6});
}

// collinear
template<>
template<>
void object::test<5>()
{
    CoordinateXY p0{1, 2};
    CoordinateXY p1{2, 3};
    CoordinateXY p2{3, 4};

    auto center = CircularArcs::getCenter(p0, p1, p2);

    ensure(std::isnan(center.x));
    ensure(std::isnan(center.y));
}

// CCW quadrant 2 to quadrant 1
template<>
template<>
void object::test<6>()
{
    CoordinateXY p0{-std::sqrt(2), -std::sqrt(2)};
    CoordinateXY p1{2, 0};
    CoordinateXY p2{-std::sqrt(2), std::sqrt(2)};

    checkEnvelope(p0, p1, p2,
                  -std::sqrt(2), -2, 2, 2);
}

// quadrant 0 to quadrant 0, crossing all axes
template<>
template<>
void object::test<7>()
{
    CoordinateXY p0{std::sqrt(2), std::sqrt(2)};
    CoordinateXY p1{2, 0};
    CoordinateXY p2{std::sqrt(3), 1};

    checkEnvelope(p0, p1, p2, -2, -2, 2, 2);
}


// quadrant 0 to quadrant 0, crossing no axes
template<>
template<>
void object::test<8>()
{
    CoordinateXY p0{1, std::sqrt(3)};
    CoordinateXY p1{std::sqrt(2), std::sqrt(2)};
    CoordinateXY p2{std::sqrt(3), 1};

    checkEnvelope(p0, p1, p2,
                  1, 1, std::sqrt(3), std::sqrt(3));
}

// half circle with start points on -/+ x axis
template<>
template<>
void object::test<9>()
{
    CoordinateXY p0{-1, 0};
    CoordinateXY p1{0, 1};
    CoordinateXY p2{1, 0};

    checkEnvelope(p0, p1, p2,
                  -1, 0, 1, 1);
}

// CCW quadrant 0 to quadrant 3
template<>
template<>
void object::test<10>()
{
    CoordinateXY p0{std::sqrt(2), std::sqrt(2)};
    CoordinateXY p1{-2, 0};
    CoordinateXY p2{std::sqrt(2), -std::sqrt(2)};

    checkEnvelope(p0, p1, p2,
                  -2, -2, std::sqrt(2), 2);
}

// collinear
template<>
template<>
void object::test<11>()
{
    CoordinateXY p0{-1, -1};
    CoordinateXY p1{1, 1};
    CoordinateXY p2{2, 2};

    checkEnvelope(p0, p1, p2,
                  -1, -1, 2, 2);
}

// collinear
template<>
template<>
void object::test<12>()
{

    CoordinateXY p0{1, 2};
    CoordinateXY p1{2, 3};
    CoordinateXY p2{3, 4};

    checkEnvelope(p0, p1, p2,
                  1, 2, 3, 4);
}

// repeated
template<>
template<>
void object::test<13>()
{
    CoordinateXY p0{3, 4};
    CoordinateXY p1{3, 4};
    CoordinateXY p2{3, 4};

    checkEnvelope(p0, p1, p2,
                  3, 4, 3, 4);
}

template<>
template<>
void object::test<14>()
{
    set_test_name("envelope: GH #1313");

    CoordinateXY p0{2, 0};
    CoordinateXY p1{4, 2};
    CoordinateXY p2{2, 1};

    checkEnvelope(p0, p1, p2,
    2, -1.0811388300841898, 5.08113883008419,2.08113883008419);
}

}

