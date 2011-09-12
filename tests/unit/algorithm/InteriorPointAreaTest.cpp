// 
// Test Suite for geos::algorithm::InteriorPointArea 

#include <tut.hpp>
// geos
#include <geos/geom/Coordinate.h>
#include <geos/algorithm/InteriorPointArea.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/Geometry.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // dummy data, not used
    struct test_interiorpointarea_data {
      typedef geos::geom::Geometry Geometry;
      typedef geos::geom::Coordinate Coordinate;
      typedef geos::algorithm::InteriorPointArea InteriorPointArea;

      geos::io::WKTReader reader;
      std::auto_ptr<Geometry> geom;

      test_interiorpointarea_data()
      {}

    };

    typedef test_group<test_interiorpointarea_data> group;
    typedef group::object object;

    group test_interiorpointarea_group("geos::algorithm::InteriorPointArea");

    //
    // Test Cases
    //

    // http://trac.osgeo.org/geos/ticket/475
    // This is a test for a memory leak more than anything else
    template<>
    template<>
    void object::test<1>()
    {
      Coordinate centroid;

      // this polygon is a typical hourglass-shape with a self intersection
      // without a node
      geom.reset( reader.read("POLYGON((6 54, 15 54, 6 47, 15 47, 6 54))") );

      bool threw = false;

      try {
        InteriorPointArea interior_builder(geom.get());
        interior_builder.getInteriorPoint(centroid);
      }
      catch(...) {
        threw = true;
      }

      ensure(threw);
    }

} // namespace tut

