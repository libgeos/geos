//
// Test Suite for geos::noding::snapround::SnapRoundingNoder class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/noding/Noder.h>
#include <geos/noding/ValidatingNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/snapround/SnapRoundingNoder.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/util/LinearComponentExtracter.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::noding;
using namespace geos::noding::snapround;
using geos::noding::Noder;
using geos::io::WKTReader;
using geos::io::WKTWriter;
using geos::geom::util::LinearComponentExtracter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_snaproundingnoder_data {

    WKTReader r;

    std::unique_ptr<Geometry>
    toLines(const std::vector<SegmentString*>* nodedList, const GeometryFactory* geomFact)
    {
        std::vector<std::unique_ptr<Geometry>> lines;

        for (auto nss : *nodedList) {
          CoordinateSequence* pts = nss->getCoordinates();
          // pts is owned by nss, so we make a copy to build the line
          // on top of. Lines are 100% self-contained and own all their parts.
          // Input nodedList can be freed.
          lines.emplace_back(geomFact->createLineString(pts->clone()));
        }
        if (lines.size() == 1) return std::move(lines[0]);

        // move the lines to pass ownership to the multiLineString
        return geomFact->createMultiLineString(std::move(lines));
    }


    std::vector<SegmentString*>
    toSegmentStrings(std::vector<const LineString*>& lines)
    {
        std::vector<SegmentString*> nssList;
        for (auto line : lines) {
            // line->getCoordinates() clones CoordinateSequence
            // into a unique_ptr<> which we have to release() to the
            // NodedSegmentString constructor, so
            // nss now owns nss->pts
            NodedSegmentString* nss = new NodedSegmentString(line->getCoordinates().release(), line);
            nssList.push_back(nss);
        }
        return nssList;
    }

    std::unique_ptr<Geometry>
    nodeValidated(const Geometry* geom1, const Geometry* geom2, SnapRoundingNoder& noder)
    {
        std::vector<const LineString*> lines;
        // lines are const* to linear components of geom1, geom1 still
        // owns all coordinates, etc
        LinearComponentExtracter::getLines(*geom1, lines);
        if (geom2 != nullptr) {
            LinearComponentExtracter::getLines(*geom2, lines);
        }

        // ssList needs to be disposed after noder is done working
        std::vector<SegmentString*> ssList = toSegmentStrings(lines);

        ValidatingNoder noderValid(noder);
        // computeNotes might alster ssList, but ssList still
        // holds all memory
        noderValid.computeNodes(&ssList);

        // getNodedSubstrings calls NodedSegmentString::getNodedSubStrings()
        // which creates new NodedSegmentString and new pts member, so complete
        // new copy of data. Can be disposed of after geometries are constructed
        std::vector<SegmentString*>* nodedList = noder.getNodedSubstrings();

        // Dispose of ssList
        for (auto ss: ssList) {
            delete ss;
        }

        std::unique_ptr<Geometry> lineGeom = toLines(nodedList, geom1->getFactory());

        // Dispose of nodedList
        for (auto nss: *nodedList) {
            delete nss;
        }

        return lineGeom;
    }

    void
    checkRounding(std::string& wkt, double scale, std::string& expected_wkt)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        PrecisionModel pm(scale);
        SnapRoundingNoder noder(&pm);
        std::unique_ptr<Geometry> result = nodeValidated(geom.get(), nullptr, noder);

        // only check if expected was provided
        if (expected_wkt.size() == 0) return;

        std::unique_ptr<Geometry> expected = r.read(expected_wkt);
        ensure_equals_geometry(expected.get(), result.get());
    }


    // test_snaproundingnoder_data() {}
};

typedef test_group<test_snaproundingnoder_data> group;
typedef group::object object;

group test_snaproundingnoder_group("geos::noding::snapround::SnapRoundingNoder");

//
// Test Cases
//

// testSimple
template<>
template<>
void object::test<1>
()
{
    std::string wkt = "MULTILINESTRING ((1 1, 9 2), (3 3, 3 0))";
    std::string expected = "MULTILINESTRING ((1 1, 3 1), (3 1, 9 2), (3 3, 3 1), (3 1, 3 0))";
    checkRounding(wkt, 1, expected);
}

/**
* This test checks the HotPixel test for overlapping horizontal line
* testHorizontalLinesWithMiddleNode
*/
template<>
template<>
void object::test<2>
()
{
    std::string wkt = "MULTILINESTRING ((2.5117493 49.0278625, 2.5144958 49.0278625), (2.511749 49.027863, 2.513123 49.027863, 2.514496 49.027863))";
    std::string expected = "MULTILINESTRING ((2.511749 49.027863, 2.513123 49.027863), (2.511749 49.027863, 2.513123 49.027863), (2.513123 49.027863, 2.514496 49.027863), (2.513123 49.027863, 2.514496 49.027863))";
    checkRounding(wkt, 1000000.0, expected);
}

// testNearbyCorner
template<>
template<>
void object::test<3>
()
{
    std::string wkt = "MULTILINESTRING ((0.2 1.1, 1.6 1.4, 1.9 2.9), (0.9 0.9, 2.3 1.7))";
    std::string expected = "MULTILINESTRING ((0 1, 1 1), (1 1, 2 1), (1 1, 2 1), (2 1, 2 2), (2 1, 2 2), (2 2, 2 3))";
    checkRounding(wkt, 1.0, expected);
}



} // namespace tut
