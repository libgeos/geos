// $Id$
// 
// Test Suite for geos::noding::OrientedCoordinateArray class.

#include <tut.hpp>
// geos
#include <geos/noding/OrientedCoordinateArray.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
// std
#include <memory>

using namespace geos::geom;
using namespace geos::noding;

namespace tut
{
    //
    // Test Group
    //

    // Common data used by all tests
    struct test_orientedcoordinatearray_data
    {
        geos::geom::PrecisionModel pm_;
        geos::geom::GeometryFactory factory_;
        geos::io::WKTReader reader_;

        typedef std::auto_ptr<CoordinateSequence> CoordSeqPtr;
        typedef std::auto_ptr<Geometry> GeomPtr;

        test_orientedcoordinatearray_data()
          : pm_(), factory_(&pm_), reader_(&factory_) {}

        CoordSeqPtr coords_from_wkt(const char *wkt) {
          GeomPtr g ( reader_.read(wkt) );
          CoordSeqPtr cs ( g->getCoordinates() );
          return cs;
        }
    };

    typedef test_group<test_orientedcoordinatearray_data> group;
    typedef group::object object;

    group test_orientedcoordinatearray_group("geos::noding::OrientedCoordinateArray");

    //
    // Test Cases
    //

    // Compare to self, closed
    template<>
    template<>
    void object::test<1>()
    {
      const char *coords = "LINESTRING(361600 6126500, 361620 6126560, 361630 6126550, 361620 6126530, 361600 6126500)";
      CoordSeqPtr cs = coords_from_wkt(coords);
      OrientedCoordinateArray oca1(*cs);
      OrientedCoordinateArray oca2(*cs);
      ensure_equals(oca1.compareTo(oca2), 0);
    }

    // Compare to reverse of self, closed
    template<>
    template<>
    void object::test<2>()
    {
      const char *coords1 = "LINESTRING(361600 6126500, 361620 6126560, 361630 6126550, 361620 6126530, 361600 6126500)";
      const char *coords2 = "LINESTRING(361600 6126500, 361620 6126530, 361630 6126550, 361620 6126560, 361600 6126500)";
      CoordSeqPtr cs1 = coords_from_wkt(coords1);
      OrientedCoordinateArray oca1(*cs1);
      CoordSeqPtr cs2 = coords_from_wkt(coords2);
      OrientedCoordinateArray oca2(*cs2);
      ensure_equals(oca1.compareTo(oca2), 0);
    }

    // Compare to self, not closed
    template<>
    template<>
    void object::test<3>()
    {
      const char *coords = "LINESTRING(361620 6126560, 361630 6126550, 361620 6126530, 361600 6126500)";
      CoordSeqPtr cs = coords_from_wkt(coords);
      OrientedCoordinateArray oca1(*cs);
      OrientedCoordinateArray oca2(*cs);
      ensure_equals(oca1.compareTo(oca2), 0);
    }

    // Compare to reverse of self, not closed
    template<>
    template<>
    void object::test<4>()
    {
      const char *coords1 = "LINESTRING(361620 6126560, 361630 6126550, 361620 6126530, 361600 6126500)";
      const char *coords2 = "LINESTRING(361600 6126500, 361620 6126530, 361630 6126550, 361620 6126560)";
      CoordSeqPtr cs1 = coords_from_wkt(coords1);
      OrientedCoordinateArray oca1(*cs1);
      CoordSeqPtr cs2 = coords_from_wkt(coords2);
      OrientedCoordinateArray oca2(*cs2);
      ensure_equals(oca1.compareTo(oca2), 0);
    }

    // Compare both ways
    template<>
    template<>
    void object::test<5>()
    {
      const char *coords1 = "LINESTRING(0 0, 10 0)";
      const char *coords2 = "LINESTRING(0 0, 10 0, 11 0)";
      CoordSeqPtr cs1 = coords_from_wkt(coords1);
      OrientedCoordinateArray oca1(*cs1);
      CoordSeqPtr cs2 = coords_from_wkt(coords2);
      OrientedCoordinateArray oca2(*cs2);
      ensure_equals(oca1.compareTo(oca2), -1);
      ensure_equals(oca2.compareTo(oca1), 1);
    }

} // namespace tut
