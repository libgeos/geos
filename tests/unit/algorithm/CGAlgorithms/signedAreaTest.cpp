// 
// Test Suite for CGAlgorithms::signedArea() function

// tut
#include <tut.hpp>
// geos
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBReader.h>
// std
#include <string>
#include <memory>
#include <cassert>
#include <sstream>

using namespace geos::algorithm;

namespace tut
{
    //
    // Test Group
    //

    struct test_signedarea_data
    {
	    typedef std::auto_ptr<geos::geom::Geometry> GeometryPtr;

        geos::geom::CoordinateSequence* cs_;
        geos::io::WKTReader reader_;
        geos::io::WKBReader breader_;

        test_signedarea_data()
            : cs_(0)
        {
            assert(0 == cs_);
        }
        
        ~test_signedarea_data()
        {
            delete cs_;
            cs_ = 0;
        }
    };

    typedef test_group<test_signedarea_data> group;
    typedef group::object object;

    group test_signedarea_group("geos::algorithm::CGAlgorithms::signedArea");

    //
    // Test Cases
    //

    // 1 - clockwise oriented
    template<>
    template<>
    void object::test<1>()
    {
        const std::string wkt("POLYGON ((60 180, 140 240, 140 240, 140 240, 200 180, 120 120, 60 180))");
        GeometryPtr geom(reader_.read(wkt));

        cs_ = geom->getCoordinates();
        double area = CGAlgorithms::signedArea(cs_);

        ensure_equals( area, 8400 );
    }

    // 2 - counter-clockwise oriented
    template<>
    template<>
    void object::test<2>()
    {
        const std::string wkt("POLYGON ((60 180, 140 120, 100 180, 140 240, 60 180))");
        GeometryPtr geom(reader_.read(wkt));

        cs_ = geom->getCoordinates();
        double area = CGAlgorithms::signedArea(cs_);

        ensure_equals( area, -2400 );
    }

    // 3 - Test the same polygon as in test No 2 but with duplicated top point
    template<>
    template<>
    void object::test<3>()
    {
        const std::string wkt("POLYGON ((60 180, 140 120, 100 180, 140 240, 140 240, 60 180))");
		GeometryPtr geom(reader_.read(wkt));

        cs_ = geom->getCoordinates();
        double area = CGAlgorithms::signedArea(cs_);

        ensure_equals( area, -2400 );
    }


} // namespace tut

