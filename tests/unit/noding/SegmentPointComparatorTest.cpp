// $Id: SegmentPointComparatorTest.cpp 3235 2011-02-23 10:25:28Z strk $
// 
// Test Suite for geos::noding::SegmentPointComparator class.
//
// Ports tests found in jts/junit/noding/SegmentPointComparatorTest.java
// and jts/junit/noding/SegmentPointComparatorFullTest.java

#include <tut.hpp>
// geos
#include <geos/noding/SegmentNode.h>
#include <geos/noding/SegmentPointComparator.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/Octant.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
// std
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_segmentpointcomparator_data
    {

        typedef std::auto_ptr<geos::geom::CoordinateSequence>
            CoordinateSequenceAutoPtr;

        typedef std::auto_ptr<geos::noding::SegmentString>
            SegmentStringAutoPtr;

        typedef geos::geom::Coordinate Coordinate;
        typedef geos::geom::LineSegment LineSegment;
        typedef geos::geom::PrecisionModel PrecisionModel;
	
        PrecisionModel pm;

        test_segmentpointcomparator_data()
            : pm(1.0)
        {}

        void checkNodePosition(int octant, double x0, double y0,
                               double x1, double y1, int
                               expectedPositionValue)
        {
            using geos::noding::SegmentPointComparator;

            int posValue = SegmentPointComparator::compare(octant,
                Coordinate(x0, y0),
                Coordinate(x1, y1)
              );
            ensure( posValue == expectedPositionValue );
        }

        void checkNodePosition(const LineSegment& seg, const Coordinate& p0,
                               const Coordinate& p1, int expectedPositionValue)
        {
            using geos::noding::Octant;
            using geos::noding::SegmentPointComparator;

            int octant = Octant::octant(seg.p0, seg.p1);
            int posValue = SegmentPointComparator::compare(octant, p0, p1);

            ensure( posValue == expectedPositionValue );
        }

        Coordinate computePoint(const LineSegment& seg, double dist)
        {
          double dx = seg.p1.x - seg.p0.x;
          double dy = seg.p1.y - seg.p0.y;
          double len = seg.getLength();
          Coordinate pt(dist * dx / len, dist * dy / len);
          pm.makePrecise(pt);
          return pt;
        }

        void checkPointsAtDistance(const LineSegment& seg,
                                   double dist0, double dist1)
        {
          using geos::geom::Coordinate;
          Coordinate p0 = computePoint(seg, dist0);
          Coordinate p1 = computePoint(seg, dist1);
          if (p0.equals(p1)) {
            checkNodePosition(seg, p0, p1, 0);
          }
          else {
            checkNodePosition(seg, p0, p1, -1);
            checkNodePosition(seg, p1, p0, 1);
          }
        }

        void checkSegment(double x, double y)
        {
          Coordinate seg0(0, 0);
          Coordinate seg1(x, y);
          LineSegment seg(seg0, seg1);

          for (int i = 0; i < 4; i++) {
            double dist = i;

            double gridSize = 1 / pm.getScale();

            checkPointsAtDistance(seg, dist, dist + 1.0 * gridSize);
            checkPointsAtDistance(seg, dist, dist + 2.0 * gridSize);
            checkPointsAtDistance(seg, dist, dist + 3.0 * gridSize);
            checkPointsAtDistance(seg, dist, dist + 4.0 * gridSize);
          }
        }


    };

    typedef test_group<test_segmentpointcomparator_data> group;
    typedef group::object object;

    group test_segmentpointcomparator_group("geos::noding::SegmentPointComparator");

    //
    // Test Cases
    //

    // testOctant0 (from simple unit test)
    template<>
    template<>
    void object::test<1>()
    {
        checkNodePosition(0, 1, 1, 2, 2, -1);
        checkNodePosition(0, 1, 0, 1, 1, -1);
    }

    // testQuadrant0 
    template<>
    template<>
    void object::test<2>()
    {
        checkSegment(100, 0);
        checkSegment(100, 50);
        checkSegment(100, 100);
        checkSegment(100, 150);
        checkSegment(0, 100);
    }

    // testQuadrant4
    template<>
    template<>
    void object::test<3>()
    {
      checkSegment(100, -50);
      checkSegment(100, -100);
      checkSegment(100, -150);
      checkSegment(0, -100);
    }

    // testQuadrant1
    template<>
    template<>
    void object::test<4>()
    {
      checkSegment(-100, 0);
      checkSegment(-100, 50);
      checkSegment(-100, 100);
      checkSegment(-100, 150);
    }

    // testQuadrant2
    template<>
    template<>
    void object::test<5>()
    {
      checkSegment(-100, 0);
      checkSegment(-100, -50);
      checkSegment(-100, -100);
      checkSegment(-100, -150);
    }

} // namespace tut
