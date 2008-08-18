// $Id$
// 
// Test Suite for CGAlgorithms::isCCW() function
// Ported from JTS junit/algorithm/IsCCWTest.java

// tut
#include <tut.h>
// geos
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/io/WKTReader.h>
// std
#include <string>
#include <memory>
#include <cassert>

using namespace geos::algorithm;

namespace tut
{
    //
    // Test Group
    //

    struct test_isccw_data
    {
	    typedef std::auto_ptr<geos::geom::Geometry> GeometryPtr;

        geos::geom::CoordinateSequence* cs_;
        geos::io::WKTReader reader_;

        test_isccw_data()
            : cs_(0)
        {
            assert(0 == cs_);
        }
        
        ~test_isccw_data()
        {
            delete cs_;
            cs_ = 0;
        }
    };

    typedef test_group<test_isccw_data> group;
    typedef group::object object;

    group test_isccw_group("geos::algorithm::CGAlgorithms::isCCW");

    //
    // Test Cases
    //

    // 1 - Test if coordinates of polygon are counter-clockwise oriented
    template<>
    template<>
    void object::test<1>()
    {
        const std::string wkt("POLYGON ((60 180, 140 240, 140 240, 140 240, 200 180, 120 120, 60 180))");
		GeometryPtr geom(reader_.read(wkt));

        cs_ = geom->getCoordinates();
        bool isCCW = CGAlgorithms::isCCW(cs_);

        ensure_equals( false, isCCW );
    }

    // 2 - Test if coordinates of polygon are counter-clockwise oriented
    template<>
    template<>
    void object::test<2>()
    {
        const std::string wkt("POLYGON ((60 180, 140 120, 100 180, 140 240, 60 180))");
		GeometryPtr geom(reader_.read(wkt));

        cs_ = geom->getCoordinates();
        bool isCCW = CGAlgorithms::isCCW(cs_);

        ensure_equals( true, isCCW );
    }

    // 3 - Test the same polygon as in test No 2 but with duplicated top point
    template<>
    template<>
    void object::test<3>()
    {
        const std::string wkt("POLYGON ((60 180, 140 120, 100 180, 140 240, 140 240, 60 180))");
		GeometryPtr geom(reader_.read(wkt));

        cs_ = geom->getCoordinates();
        bool isCCW = CGAlgorithms::isCCW(cs_);

        ensure_equals( true, isCCW );
    }

} // namespace tut

