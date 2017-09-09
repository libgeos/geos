//
// Test Suite for geos::algorithm::ConvexHull
// Ported from JTS junit/algorithm/ConvexHullTest.java

#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/algorithm/ConvexHull.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
// std
#include <sstream>
#include <memory>
#include <cassert>

namespace geos {
	namespace geom {
		class Geometry;
	}
}

using namespace geos::geom; // for Location

namespace tut
{
	//
	// Test Group
	//

	// dummy data, not used
	struct test_convexhull_data
    {
        GeometryPtr geom_;
        geos::geom::PrecisionModel pm_;
        geos::geom::GeometryFactory::Ptr factory_;
        geos::io::WKTReader reader_;

        test_convexhull_data()
			: geom_(nullptr), pm_(1), factory_(GeometryFactory::create(&pm_, 0)), reader_(factory_.get())
        {
            assert(nullptr == geom_);
        }

        ~test_convexhull_data()
        {
            factory_->destroyGeometry(geom_);
            geom_ = nullptr;
        }
    };

	typedef test_group<test_convexhull_data> group;
	typedef group::object object;

    group test_convexhull_group("geos::algorithm::ConvexHull");

	//
	// Test Cases
	//

	// 1 - Test convex hull of linestring
	template<>
    template<>
    void object::test<1>()
    {
        using geos::geom::LineString;

        Geometry::Ptr lineGeom(reader_.read("LINESTRING (30 220, 240 220, 240 220)"));
        LineString::Ptr line(dynamic_cast<LineString*>(lineGeom.release()));
        ensure(nullptr != line.get());

        Geometry::Ptr hullGeom(reader_.read("LINESTRING (30 220, 240 220)"));
        LineString::Ptr convexHull(dynamic_cast<LineString*>(hullGeom.release()));
        ensure(nullptr != convexHull.get());

        geom_ = line->convexHull();
        ensure( convexHull->equalsExact(geom_) );
    }

	// 2 - Test convex hull of multipoint
	template<>
    template<>
    void object::test<2>()
    {
        using geos::geom::LineString;

        Geometry::Ptr geom(reader_.read("MULTIPOINT (130 240, 130 240, 130 240, 570 240, 570 240, 570 240, 650 240)"));
        ensure(nullptr != geom.get());

        Geometry::Ptr hullGeom(reader_.read("LINESTRING (130 240, 650 240)"));
        LineString::Ptr convexHull(dynamic_cast<LineString*>(hullGeom.release()));
        ensure(nullptr != convexHull.get());

        geom_ = geom->convexHull();
        ensure( convexHull->equalsExact(geom_) );
    }

	// 3 - Test convex hull of multipoint
	template<>
    template<>
    void object::test<3>()
    {
        using geos::geom::LineString;

        Geometry::Ptr geom(reader_.read("MULTIPOINT (0 0, 0 0, 10 0)"));
        ensure(nullptr != geom.get());

        Geometry::Ptr hullGeom(reader_.read("LINESTRING (0 0, 10 0)"));
        LineString::Ptr convexHull(dynamic_cast<LineString*>(hullGeom.release()));
        ensure(nullptr != convexHull.get());

        geom_ = geom->convexHull();
        ensure( convexHull->equalsExact(geom_) );
    }

	// 4 - Test convex hull of multipoint
	template<>
    template<>
    void object::test<4>()
    {
        using geos::geom::LineString;

        Geometry::Ptr geom(reader_.read("MULTIPOINT (0 0, 10 0, 10 0)"));
        ensure(nullptr != geom.get());

        Geometry::Ptr hullGeom(reader_.read("LINESTRING (0 0, 10 0)"));
        LineString::Ptr convexHull(dynamic_cast<LineString*>(hullGeom.release()));
        ensure(nullptr != convexHull.get());

        geom_ = geom->convexHull();
        ensure( convexHull->equalsExact(geom_) );
    }

	// 5 - Test convex hull of multipoint
	template<>
    template<>
    void object::test<5>()
    {
        using geos::geom::LineString;

        Geometry::Ptr geom(reader_.read("MULTIPOINT (0 0, 5 0, 10 0)"));
        ensure(nullptr != geom.get());

        Geometry::Ptr hullGeom(reader_.read("LINESTRING (0 0, 10 0)"));
        LineString::Ptr convexHull(dynamic_cast<LineString*>(hullGeom.release()));
        ensure(nullptr != convexHull.get());

        geom_ = geom->convexHull();
        ensure( convexHull->equalsExact(geom_) );
    }

	// 6 - Test convex hull of multipoint exported to string form
	template<>
    template<>
    void object::test<6>()
    {
        using geos::geom::LineString;

        Geometry::Ptr geom(reader_.read("MULTIPOINT (0 0, 5 1, 10 0)"));
        ensure(nullptr != geom.get());

        Geometry::Ptr hullGeom(geom->convexHull());
        ensure(nullptr != hullGeom.get());

        Geometry::Ptr expectedHull(reader_.read("POLYGON ((0 0, 5 1, 10 0, 0 0))"));
        ensure(nullptr != expectedHull.get());

        ensure_equals( hullGeom->toString(), expectedHull->toString() );
    }

	// 7 - Test convex hull of multipoint
	template<>
    template<>
    void object::test<7>()
    {
        using geos::geom::LineString;

        Geometry::Ptr geom(reader_.read("MULTIPOINT (0 0, 0 0, 5 0, 5 0, 10 0, 10 0)"));
        ensure(nullptr != geom.get());

        Geometry::Ptr hullGeom(reader_.read("LINESTRING (0 0, 10 0)"));
        LineString::Ptr convexHull(dynamic_cast<LineString*>(hullGeom.release()));
        ensure(nullptr != convexHull.get());

        geom_ = geom->convexHull();
        ensure( convexHull->equalsExact(geom_) );
    }

} // namespace tut

