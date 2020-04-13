// Test Suite for C-API GEOSMaximumInscribedCircle

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace tut{
//
// Test Group
//

// Common data used in test cases.
struct test_capimaximuminscribedcircle_data {
    GEOSGeometry* input_;
    GEOSGeometry* expcircle_;
    GEOSGeometry* actcircle_;
    GEOSGeometry* expcentre_;
    GEOSGeometry* actcentre_;

    static void
    notice(const char* fmt, ...)
    {
        std::fprintf(stdout, "NOTICE: ");

        va_list ap;
        va_start(ap, fmt);
        std::vfprintf(stdout, fmt, ap);
        va_end(ap);

        std::fprintf(stdout, "\n");
    }

    test_capimaximuminscribedcircle_data()
        : input_(nullptr), expcircle_(nullptr), actcircle_(nullptr), expcentre_(nullptr), actcentre_(nullptr)
    {
        initGEOS(notice, notice);
    }

    ~test_capimaximuminscribedcircle_data()
    {
        GEOSGeom_destroy(input_);
        GEOSGeom_destroy(expcircle_);
        GEOSGeom_destroy(actcircle_);
        GEOSGeom_destroy(expcentre_);
        GEOSGeom_destroy(actcentre_);
        input_ = nullptr;
        expcircle_ = nullptr;
        actcircle_ = nullptr;
        expcentre_ = nullptr;
        actcentre_ = nullptr;
        finishGEOS();
    }
};

typedef test_group<test_capimaximuminscribedcircle_data> group;
typedef group::object object;

group test_capimaximuminscribedcircle_group("capi::GEOSMaximumInscribedCircle");

//
// Test Cases
//

// Single point
template<>
template<>
void object::test<1>()
{
    double radius;
    input_ = GEOSGeomFromWKT("POINT(10 10)");
    ensure(nullptr != input_);
    expcentre_ = GEOSGeomFromWKT("POINT(10 10)");
    ensure(nullptr != expcentre_);
    expcircle_ = GEOSGeomFromWKT("POINT(10 10)");
    ensure(nullptr != expcircle_);
    actcircle_ = GEOSMaximumInscribedCircle(input_, 2, &radius, &actcentre_);
    ensure(nullptr != actcircle_);

    bool centreEqual = GEOSEqualsExact(actcentre_, expcentre_, 0.0001) != 0;
    bool circleEqual = GEOSEqualsExact(actcircle_, expcircle_, 0.0001) != 0;
    ensure(std::fabs(radius - 0) < 0.0001);
    ensure(centreEqual);
    ensure(circleEqual);
}

// Line string
template<>
template<>
void object::test<2>()
{
    double radius;
    input_ = GEOSGeomFromWKT("LINESTRING(0 10, 0 20)");
    ensure(nullptr != input_);
    expcentre_ = GEOSGeomFromWKT("POINT EMPTY");
    ensure(nullptr != expcentre_);
    expcircle_ = GEOSGeomFromWKT("POINT EMPTY");
    ensure(nullptr != expcircle_);
    actcircle_ = GEOSMaximumInscribedCircle(input_, 2, &radius, &actcentre_);
    ensure(nullptr != actcircle_);

    bool centreEqual = GEOSEqualsExact(actcentre_, expcentre_, 0.0001) != 0;
    bool circleEqual = GEOSEqualsExact(actcircle_, expcircle_, 0.0001) != 0;
    ensure(std::fabs(radius - 0) < 0.0001);
    ensure(centreEqual);
    ensure(circleEqual);
}

// Degenerate polygon
template<>
template<>
void object::test<3>()
{
    double radius;
    input_ = GEOSGeomFromWKT("POLYGON EMPTY");
    ensure(nullptr != input_);
    expcentre_ = GEOSGeomFromWKT("POINT EMPTY");
    ensure(nullptr != expcentre_);
    expcircle_ = GEOSGeomFromWKT("POINT EMPTY");
    ensure(nullptr != expcircle_);
    actcircle_ = GEOSMaximumInscribedCircle(input_, 2, &radius, &actcentre_);
    ensure(nullptr != actcircle_);

    bool centreEqual = GEOSEqualsExact(actcentre_, expcentre_, 0.0001) != 0;
    bool circleEqual = GEOSEqualsExact(actcircle_, expcircle_, 0.0001) != 0;
    ensure(std::fabs(radius - 0) < 0.0001);
    ensure(centreEqual);
    ensure(circleEqual);
}

// Trivial square polygon
template<>
template<>
void object::test<4>()
{
    double radius;
    input_ = GEOSGeomFromWKT("POLYGON ((0.0 0.0, 2.0 0.0, 2.0 2.0, 0.0 2.0, 0.0 0.0))");
    ensure(nullptr != input_);
    expcentre_ = GEOSGeomFromWKT("POINT (1 1)");
    ensure(nullptr != expcentre_);
    expcircle_ = GEOSGeomFromWKT("POLYGON ((2 1, 1.9807853 0.80490968, 1.9238795 0.61731657, 1.8314696 0.44442977, 1.7071068 0.29289322, 1.5555702 0.16853039, 1.3826834 0.076120467, 1.1950903 0.01921472, 1 0, 0.80490968 0.01921472, 0.61731657 0.076120467, 0.44442977 0.16853039, 0.29289322 0.29289322, 0.16853039 0.44442977, 0.076120467 0.61731657, 0.01921472 0.80490968, 0 1, 0.01921472 1.1950903, 0.076120467 1.3826834, 0.16853039 1.5555702, 0.29289322 1.7071068, 0.44442977 1.8314696, 0.61731657 1.9238795, 0.80490968 1.9807853, 1 2, 1.1950903 1.9807853, 1.3826834 1.9238795, 1.5555702 1.8314696, 1.7071068 1.7071068, 1.8314696 1.5555702, 1.9238795 1.3826834, 1.9807853 1.1950903, 2 1))");
    ensure(nullptr != expcircle_);
    actcircle_ = GEOSMaximumInscribedCircle(input_, 2, &radius, &actcentre_);
    ensure(nullptr != actcircle_);

    bool centreEqual = GEOSEqualsExact(actcentre_, expcentre_, 0.0001) != 0;
    bool circleEqual = GEOSEqualsExact(actcircle_, expcircle_, 0.0001) != 0;
    ensure(std::fabs(radius - 1) < 0.0001);
    ensure(centreEqual);
    ensure(circleEqual);
}

// Square with the centre cut out. True centroid lies inside hole.
template<>
template<>
void object::test<5>()
{
    double radius;
    input_ = GEOSGeomFromWKT("POLYGON ((0.0 0.0, 0.0 30.0, 30.0 30.0, 30.0 0.0, 0.0 0.0), \
                  (10.0 10.0, 10.0 20.0, 20.0 20.0, 20.0 10.0, 10.0 10.0))");
    ensure(nullptr != input_);
    expcentre_ = GEOSGeomFromWKT("POINT (5 12.5)");
    ensure(nullptr != expcentre_);
    expcircle_ = GEOSGeomFromWKT("POLYGON ((10 12.5, 9.9039264 11.524548, 9.6193977 10.586583, 9.1573481 9.7221488, 8.5355339 8.9644661, 7.7778512 8.3426519, 6.9134172 7.8806023, 5.9754516 7.5960736, 5 7.5, 4.0245484 7.5960736, 3.0865828 7.8806023, 2.2221488 8.3426519, 1.4644661 8.9644661, 0.84265194 9.7221488, 0.38060234 10.586583, 0.096073598 11.524548, 0 12.5, 0.096073598 13.475452, 0.38060234 14.413417, 0.84265194 15.277851, 1.4644661 16.035534, 2.2221488 16.657348, 3.0865828 17.119398, 4.0245484 17.403926, 5 17.5, 5.9754516 17.403926, 6.9134172 17.119398, 7.7778512 16.657348, 8.5355339 16.035534, 9.1573481 15.277851, 9.6193977 14.413417, 9.9039264 13.475452, 10 12.5))");
    ensure(nullptr != expcircle_);
    actcircle_ = GEOSMaximumInscribedCircle(input_, 2, &radius, &actcentre_);
    ensure(nullptr != actcircle_);

    bool centreEqual = GEOSEqualsExact(actcentre_, expcentre_, 0.0001) != 0;
    bool circleEqual = GEOSEqualsExact(actcircle_, expcircle_, 0.0001) != 0;
    ensure(std::fabs(radius - 5) < 0.0001);
    ensure(centreEqual);
    ensure(circleEqual);
}

// Polygon with no holes, but centroid does not intersect polygon.
template<>
template<>
void object::test<6>()
{
    double radius;
    input_ = GEOSGeomFromWKT("POLYGON ((0.0 0.0, 1.0 0.0, 1.0 3.0, 2.0 3.0, 2.0 0.0, 3.0 0.0, 3.0 4.0, 0.0 4.0, 0.0 0.0))");
    ensure(nullptr != input_);
    expcentre_ = GEOSGeomFromWKT("POINT (1.25 3.5)");
    ensure(nullptr != expcentre_);
    expcircle_ = GEOSGeomFromWKT("POLYGON ((1.75 3.5, 1.7403926 3.4024548, 1.7119398 3.3086583, 1.6657348 3.2222149, 1.6035534 3.1464466, 1.5277851 3.0842652, 1.4413417 3.0380602, 1.3475452 3.0096074, 1.25 3, 1.1524548 3.0096074, 1.0586583 3.0380602, 0.97221488 3.0842652, 0.89644661 3.1464466, 0.83426519 3.2222149, 0.78806023 3.3086583, 0.75960736 3.4024548, 0.75 3.5, 0.75960736 3.5975452, 0.78806023 3.6913417, 0.83426519 3.7777851, 0.89644661 3.8535534, 0.97221488 3.9157348, 1.0586583 3.9619398, 1.1524548 3.9903926, 1.25 4, 1.3475452 3.9903926, 1.4413417 3.9619398, 1.5277851 3.9157348, 1.6035534 3.8535534, 1.6657348 3.7777851, 1.7119398 3.6913417, 1.7403926 3.5975452, 1.75 3.5))");
    ensure(nullptr != expcircle_);
    actcircle_ = GEOSMaximumInscribedCircle(input_, 2, &radius, &actcentre_);
    ensure(nullptr != actcircle_);

    bool centreEqual = GEOSEqualsExact(actcentre_, expcentre_, 0.0001) != 0;
    bool circleEqual = GEOSEqualsExact(actcircle_, expcircle_, 0.0001) != 0;
    ensure(std::fabs(radius - 0.5) < 0.0001);
    ensure(centreEqual);
    ensure(circleEqual);
}

// Multipolygon where polygon of smallest area actually contains largest inscribed circle.
// However, algorithm uses polygon of largest area in calculation. This polygon is same as in test 4.
template<>
template<>
void object::test<7>()
{
    double radius;
    input_ = GEOSGeomFromWKT("MULTIPOLYGON (((0.0 0.0, 1.0 0.0, 1.0 3.0, 2.0 3.0, 2.0 0.0, 3.0 0.0, 3.0 4.0, 0.0 4.0, 0.0 0.0)), \
                       ((-3.0 -3.0, -1.0 -3.0, -1.0 -1.0, -3.0 -1.0, -3.0 -3.0)))");
    ensure(nullptr != input_);
    expcentre_ = GEOSGeomFromWKT("POINT (1.25 3.5)");
    ensure(nullptr != expcentre_);
    expcircle_ = GEOSGeomFromWKT("POLYGON ((1.75 3.5, 1.7403926 3.4024548, 1.7119398 3.3086583, 1.6657348 3.2222149, 1.6035534 3.1464466, 1.5277851 3.0842652, 1.4413417 3.0380602, 1.3475452 3.0096074, 1.25 3, 1.1524548 3.0096074, 1.0586583 3.0380602, 0.97221488 3.0842652, 0.89644661 3.1464466, 0.83426519 3.2222149, 0.78806023 3.3086583, 0.75960736 3.4024548, 0.75 3.5, 0.75960736 3.5975452, 0.78806023 3.6913417, 0.83426519 3.7777851, 0.89644661 3.8535534, 0.97221488 3.9157348, 1.0586583 3.9619398, 1.1524548 3.9903926, 1.25 4, 1.3475452 3.9903926, 1.4413417 3.9619398, 1.5277851 3.9157348, 1.6035534 3.8535534, 1.6657348 3.7777851, 1.7119398 3.6913417, 1.7403926 3.5975452, 1.75 3.5))");
    ensure(nullptr != expcircle_);
    actcircle_ = GEOSMaximumInscribedCircle(input_, 2, &radius, &actcentre_);
    ensure(nullptr != actcircle_);

    bool centreEqual = GEOSEqualsExact(actcentre_, expcentre_, 0.0001) != 0;
    bool circleEqual = GEOSEqualsExact(actcircle_, expcircle_, 0.0001) != 0;
    ensure(std::fabs(radius - 0.5) < 0.0001);
    ensure(centreEqual);
    ensure(circleEqual);
}

// Triangle that demonstrates more accurate results with increased segments.
// Bisecting
template<>
template<>
void object::test<8>()
{
    double radius;
    input_ = GEOSGeomFromWKT("POLYGON ((0.0 0.0, 5.0 0.0, 5.0 1.0, 0.0 0.0))");
    ensure(nullptr != input_);
    expcentre_ = GEOSGeomFromWKT("POINT (3.75 0.25)");
    ensure(nullptr != expcentre_);
    expcircle_ = GEOSGeomFromWKT("POLYGON ((4 0.25, 3.9951963 0.20122742, 3.9809699 0.15432914, 3.9578674 0.11110744, 3.9267767 0.073223305, 3.8888926 0.042132597, 3.8456709 0.019030117, 3.7987726 0.0048036799, 3.75 0, 3.7012274 0.0048036799, 3.6543291 0.019030117, 3.6111074 0.042132597, 3.5732233 0.073223305, 3.5421326 0.11110744, 3.5190301 0.15432914, 3.5048037 0.20122742, 3.5 0.25, 3.5048037 0.29877258, 3.5190301 0.34567086, 3.5421326 0.38889256, 3.5732233 0.4267767, 3.6111074 0.4578674, 3.6543291 0.48096988, 3.7012274 0.49519632, 3.75 0.5, 3.7987726 0.49519632, 3.8456709 0.48096988, 3.8888926 0.4578674, 3.9267767 0.4267767, 3.9578674 0.38889256, 3.9809699 0.34567086, 3.9951963 0.29877258, 4 0.25))");
    ensure(nullptr != expcircle_);
    actcircle_ = GEOSMaximumInscribedCircle(input_, 2, &radius, &actcentre_);
    ensure(nullptr != actcircle_);

    bool centreEqual = GEOSEqualsExact(actcentre_, expcentre_, 0.0001) != 0;
    bool circleEqual = GEOSEqualsExact(actcircle_, expcircle_, 0.0001) != 0;
    ensure(std::fabs(radius - 0.25) < 0.0001);
    ensure(centreEqual);
    ensure(circleEqual);
}

// Triangle that demonstrates more accurate results with increased segments.
// 10-secting
template<>
template<>
void object::test<9>()
{
    double radius;
    input_ = GEOSGeomFromWKT("POLYGON ((0.0 0.0, 5.0 0.0, 5.0 1.0, 0.0 0.0))");
    ensure(nullptr != input_);
    expcentre_ = GEOSGeomFromWKT("POINT (4.55 0.45)");
    ensure(nullptr != expcentre_);
    expcircle_ = GEOSGeomFromWKT("POLYGON ((5 0.45, 4.9913534 0.36220936, 4.9657458 0.27779246, 4.9241613 0.1999934, 4.8681981 0.13180195, 4.8000066 0.075838674, 4.7222075 0.03425421, 4.6377906 0.0086466238, 4.55 5.5511151e-17, 4.4622094 0.0086466238, 4.3777925 0.03425421, 4.2999934 0.075838674, 4.2318019 0.13180195, 4.1758387 0.1999934, 4.1342542 0.27779246, 4.1086466 0.36220936, 4.1 0.45, 4.1086466 0.53779064, 4.1342542 0.62220754, 4.1758387 0.7000066, 4.2318019 0.76819805, 4.2999934 0.82416133, 4.3777925 0.86574579, 4.4622094 0.89135338, 4.55 0.9, 4.6377906 0.89135338, 4.7222075 0.86574579, 4.8000066 0.82416133, 4.8681981 0.76819805, 4.9241613 0.7000066, 4.9657458 0.62220754, 4.9913534 0.53779064, 5 0.45))");
    ensure(nullptr != expcircle_);
    actcircle_ = GEOSMaximumInscribedCircle(input_, 10, &radius, &actcentre_);
    ensure(nullptr != actcircle_);

    bool centreEqual = GEOSEqualsExact(actcentre_, expcentre_, 0.0001) != 0;
    bool circleEqual = GEOSEqualsExact(actcircle_, expcircle_, 0.0001) != 0;
    ensure(std::fabs(radius - 0.45) < 0.0001);
    ensure(centreEqual);
    ensure(circleEqual);
}

// Triangle that demonstrates more accurate results with increased segments.
// 100-secting
template<>
template<>
void object::test<10>()
{
    double radius;
    input_ = GEOSGeomFromWKT("POLYGON ((0.0 0.0, 5.0 0.0, 5.0 1.0, 0.0 0.0))");
    ensure(nullptr != input_);
    expcentre_ = GEOSGeomFromWKT("POINT (4.54944 0.450556)");
    ensure(nullptr != expcentre_);
    expcircle_ = GEOSGeomFromWKT("POLYGON ((4.9998578 0.45055556, 4.9912033 0.36268426, 4.9655722 0.27818981, 4.9239495 0.20031928, 4.8679348 0.13206519, 4.7996807 0.076050508, 4.7218102 0.034427843, 4.6373157 0.0087967322, 4.5494444 0.00014216519, 4.4615732 0.0087967322, 4.3770787 0.034427843, 4.2992082 0.076050508, 4.2309541 0.13206519, 4.1749394 0.20031928, 4.1333167 0.27818981, 4.1076856 0.36268426, 4.0990311 0.45055556, 4.1076856 0.53842685, 4.1333167 0.6229213, 4.1749394 0.70079183, 4.2309541 0.76904592, 4.2992082 0.8250606, 4.3770787 0.86668327, 4.4615732 0.89231438, 4.5494444 0.90096895, 4.6373157 0.89231438, 4.7218102 0.86668327, 4.7996807 0.8250606, 4.8679348 0.76904592, 4.9239495 0.70079183, 4.9655722 0.6229213, 4.9912033 0.53842685, 4.9998578 0.45055556))");
    ensure(nullptr != expcircle_);
    actcircle_ = GEOSMaximumInscribedCircle(input_, 100, &radius, &actcentre_);
    ensure(nullptr != actcircle_);

    bool centreEqual = GEOSEqualsExact(actcentre_, expcentre_, 0.0001) != 0;
    bool circleEqual = GEOSEqualsExact(actcircle_, expcircle_, 0.0001) != 0;
    ensure(std::fabs(radius - 0.450413) < 0.0001);
    ensure(centreEqual);
    ensure(circleEqual);
}

} // namespace tut