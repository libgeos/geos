// 
// Test Suite for geos::noding::snapround::HotPixel class.

#include <tut.hpp>
// geos
#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/snapround/HotPixel.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/PrecisionModel.h>
// std
#include <memory>

namespace tut
{
  //
  // Test Group
  //

  // Common data used by all tests
  struct test_hotpixel_data
  {

    typedef geos::geom::Coordinate Coordinate;
    typedef geos::geom::Envelope Envelope;
    typedef geos::geom::PrecisionModel PrecisionModel;
    typedef geos::algorithm::LineIntersector LineIntersector;
    typedef geos::noding::snapround::HotPixel HotPixel;

    test_hotpixel_data() {}
  };

  typedef test_group<test_hotpixel_data> group;
  typedef group::object object;

  group test_hotpixel_group("geos::noding::snapround::HotPixel");

  //
  // Test Cases
  //

  // Test with scaleFactor=1
  template<>
  template<>
  void object::test<1>()
  {

    LineIntersector li;
    Coordinate pt(10, 10);
    HotPixel hp(pt, 1, li);

    ensure_equals(hp.getCoordinate(), pt);

    const Envelope& env = hp.getSafeEnvelope();
    ensure_equals(env.toString(), "Env[9.25:10.75,9.25:10.75]");

    Coordinate p0(0, 10);
    Coordinate p1(20, 10);
    ensure( "hp.intersects 0 10, 20 10", hp.intersects(p0, p1) );

    p1.y = 11; // intersection point within 0.75 distance
    ensure( "hp.intersects(0 10, 20 11)", hp.intersects(p0, p1));

    p1.y = 20;
    ensure_not( "!hp.intersects(0 10, 20 20)", hp.intersects(p0, p1));

  }

  // Test with scaleFactor=10
  // See http://trac.osgeo.org/geos/ticket/498
  template<>
  template<>
  void object::test<2>()
  {

    LineIntersector li;
    Coordinate pt(10, 10);
    HotPixel hp(pt, 10, li);

    ensure_equals(hp.getCoordinate(), pt);

    const Envelope& env = hp.getSafeEnvelope();
    ensure_equals(env.toString(), "Env[9.925:10.075,9.925:10.075]");

    Coordinate p0(0, 10);
    Coordinate p1(20, 10);
    ensure( "hp.intersects 0 10, 20 10", hp.intersects(p0, p1) );

    p1.y = 11; // intersection point not within 0.075 distance
    ensure_not( "hp.intersects(0 10, 20 11)", hp.intersects(p0, p1));

  }

  // Test intersects
  // See http://trac.osgeo.org/geos/ticket/635
  template<>
  template<>
  void object::test<3>()
  {

    double scale = 1.0;
    Coordinate p1(0,0);
    Coordinate p2(3,2);
    Coordinate p3(1,1);

    PrecisionModel pm(scale);
    LineIntersector li(&pm);
    HotPixel hp(p3, scale, li);

    ensure(hp.intersects(p1,p2));

  }

  // TODO: test addSnappedNode !


} // namespace tut
