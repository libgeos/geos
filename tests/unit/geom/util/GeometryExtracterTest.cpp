// $Id$
// 
// Test Suite for geos::geom::util::GeometryExtracter class.

// tut
#include <tut.hpp>
// geos
#include <geos/platform.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/util/GeometryExtracter.h>
// std
#include <vector>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_geometryextracter_data
    {
      geos::geom::PrecisionModel pm;
      geos::geom::GeometryFactory gf;
      geos::io::WKTReader wktreader;
      geos::io::WKTWriter wktwriter;

		  typedef std::auto_ptr<geos::geom::Geometry> GeomPtr;

      typedef geos::io::WKTReader WKTReader;
      typedef geos::io::WKTWriter WKTWriter;

      test_geometryextracter_data()
        :
        pm(1.0),
        gf(&pm),
        wktreader(&gf)
      {
      }
    };

    typedef test_group<test_geometryextracter_data> group;
    typedef group::object object;

    group test_geometryextracter_group("geos::geom::util::GeometryExtracter");

    //
    // Test Cases
    //

    // Test extraction of single point
    template<>
    template<>
    void object::test<1>()
    {
      using namespace geos::geom::util;
      using namespace geos::geom;

      GeomPtr geom(wktreader.read("POINT(-117 33)"));
      std::vector<const Point*> lst_points;
      std::vector<const LineString*> lst_lines;

      GeometryExtracter::extract<Point>(*geom, lst_points);
      GeometryExtracter::extract<LineString>(*geom, lst_lines);

      ensure_equals( lst_points.size(), 1u );
      ensure_equals( lst_lines.size(), 0u );
    }

    // Test extraction of multiple types 
    template<>
    template<>
    void object::test<2>()
    {
      using namespace geos::geom::util;
      using namespace geos::geom;

      GeomPtr geom(wktreader.read("GEOMETRYCOLLECTION(POINT(-117 33),LINESTRING(0 0, 10 0),POINT(0 0),POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)),LINESTRING(10 0, 23 30),POINT(20 20))"));

      std::vector<const Point*> lst_points;
      std::vector<const LineString*> lst_lines;
      std::vector<const Polygon*> lst_polys;

      GeometryExtracter::extract<Point>(*geom, lst_points);
      GeometryExtracter::extract<LineString>(*geom, lst_lines);
      GeometryExtracter::extract<Polygon>(*geom, lst_polys);

      ensure_equals( lst_points.size(), 3u );
      ensure_equals( lst_lines.size(), 2u );
      ensure_equals( lst_polys.size(), 1u );
    }

} // namespace tut
