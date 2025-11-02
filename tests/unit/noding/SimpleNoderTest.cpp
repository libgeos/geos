#include <tut/tut.hpp>

#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/noding/ArcIntersectionAdder.h>
#include <geos/noding/NodableArcString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/SimpleNoder.h>

using geos::geom::CoordinateXY;
using geos::geom::CoordinateSequence;
using geos::geom::CircularArc;
using geos::algorithm::Orientation;
using geos::noding::ArcString;
using geos::noding::SegmentString;
using geos::noding::NodableArcString;
using geos::noding::NodedSegmentString;
using geos::noding::PathString;
using geos::noding::SimpleNoder;

namespace tut {

struct test_simplenoder_data {

    template<typename T1, typename T2>
    static void
    check_length_equal(const T1& actual, const T2& expected) {
        double tot_actual = 0.0;
        double tot_expected = 0.0;

        for (const auto& path : actual) {
            tot_actual += path->getLength();
        }
        for (const auto& path : expected) {
            tot_expected += path->getLength();
        }

        ensure_equals("length does not match expected", tot_actual, tot_expected, 1e-8);
    }

    template<typename T>
    static void printPaths(const T& paths) {
        std::cout << "GEOMETRYCOLLECTION (";

        bool collFirst = true;

        for (const auto& path : paths) {
            if (collFirst) {
                collFirst = false;
            } else {
                std::cout << ", ";
            }

            if (auto* segString = dynamic_cast<const SegmentString*>(&*path)) {
                std::cout << "LINESTRING (";

                bool first = true;
                const CoordinateSequence& seq = *segString->getCoordinates();
                seq.forEach([&first](const auto& pt) {
                    if (first) {
                        first = false;
                    } else {
                        std::cout << ", ";
                    }

                    std::cout << pt;
                });

                std::cout << ")";
            } else {
                auto* arcString = static_cast<const ArcString*>(&*path);
                std::cout << "CIRCULARSTRING (";
                bool first = true;
                for (const auto& arc : *arcString) {
                    if (first) {
                        first = false;
                        std::cout << arc.p0 << ", ";
                    } else {
                        std::cout << ", ";
                    }
                    std::cout << arc.p1 << ", " << arc.p2;
                }
                std::cout << ")";
            }

        }

        std::cout << ")";
    }
};

typedef test_group<test_simplenoder_data> group;
typedef group::object object;

group test_simplenoder_group("geos::noding::SimpleNoder");

template<>
template<>
void object::test<1>()
{
    set_test_name("segment-segment intersection");

    auto seq1 = std::make_shared<CoordinateSequence>();
    seq1->add(CoordinateXY{0, 0});
    seq1->add(CoordinateXY{1, 1});
    NodedSegmentString ss1(seq1, false, false, nullptr);

    auto seq2 = std::make_shared<CoordinateSequence>();
    seq2->add(CoordinateXY{1, 0});
    seq2->add(CoordinateXY{0, 1});
    NodedSegmentString ss2(seq2, false, false, nullptr);

    std::vector<PathString*> ss{&ss1, &ss2};

    SimpleNoder noder(std::make_unique<geos::noding::ArcIntersectionAdder>());
    noder.computePathNodes(ss);

    auto paths = noder.getNodedPaths();

    check_length_equal(paths, ss);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("arc-arc intersection");

    CircularArc arc0({-1, 0}, {1, 0}, {0, 0}, 1, Orientation::CLOCKWISE);
    CircularArc arc1({-1, 1}, {1, 1}, {0, 1}, 1, Orientation::COUNTERCLOCKWISE);

    NodableArcString as0({arc0});
    NodableArcString as1({arc1});

    std::vector<PathString*> ss{&as0, &as1};

    SimpleNoder noder(std::make_unique<geos::noding::ArcIntersectionAdder>());
    noder.computePathNodes(ss);

    auto paths = noder.getNodedPaths();

    check_length_equal(paths, ss);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("arc-segment intersection");

    CircularArc arc0({-1, 0}, {1, 0}, {0, 0}, 1, Orientation::CLOCKWISE);
    NodableArcString as0({arc0});

    auto seq1 = std::make_shared<CoordinateSequence>();
    seq1->add(CoordinateXY{-1, 0.5});
    seq1->add(CoordinateXY{1, 0.5});
    NodedSegmentString ss1(seq1, false, false, nullptr);

    std::vector<PathString*> ss{&as0, &ss1};

    SimpleNoder noder(std::make_unique<geos::noding::ArcIntersectionAdder>());
    noder.computePathNodes(ss);

    auto paths = noder.getNodedPaths();

    check_length_equal(paths, ss);
}

}
