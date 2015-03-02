// 
// Test Suite for C-API GEOSintersection

#include <tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace tut
{
	//
	// Test Group
	//

	// Common data used in test cases.
	struct test_capigeosintersection_data
	{
		GEOSWKTWriter* wktw_;
		GEOSGeometry* geom1_;
		GEOSGeometry* geom2_;
		GEOSGeometry* geom3_;

		static void notice(const char *fmt, ...)
		{
			std::fprintf(stdout, "NOTICE: ");

			va_list ap;
			va_start(ap, fmt);
			std::vfprintf(stdout, fmt, ap);
			va_end(ap);

			std::fprintf(stdout, "\n");
		}

		test_capigeosintersection_data()
			: geom1_(0), geom2_(0), geom3_(0)
		{
			initGEOS(notice, notice);
			wktw_ = GEOSWKTWriter_create();
			GEOSWKTWriter_setTrim(wktw_, 1);
			GEOSWKTWriter_setOutputDimension(wktw_, 3);
		}

		std::string toWKT(GEOSGeometry* g)
		{
			char* wkt = GEOSWKTWriter_write(wktw_, g);
			std::string ret(wkt);
			GEOSFree(wkt);
			return ret;
		}

		~test_capigeosintersection_data()
		{
			GEOSWKTWriter_destroy(wktw_);
			GEOSGeom_destroy(geom1_);
			GEOSGeom_destroy(geom2_);
			GEOSGeom_destroy(geom3_);
			geom1_ = 0;
			geom2_ = 0;
			geom3_ = 0;
			finishGEOS();
		}

	};

	typedef test_group<test_capigeosintersection_data> group;
	typedef group::object object;

	group test_capigeosintersection_group("capi::GEOSIntersection");

	//
	// Test Cases
	//

	template<>
	template<>
	void object::test<1>()
	{
		geom1_ = GEOSGeomFromWKT("POLYGON EMPTY");
		geom2_ = GEOSGeomFromWKT("POLYGON EMPTY");

		ensure(0 != geom1_);
		ensure(0 != geom2_);

		geom3_ = GEOSIntersection(geom1_, geom2_);
		ensure(0 != geom3_);
		ensure_equals(toWKT(geom3_), std::string("GEOMETRYCOLLECTION EMPTY"));
	}

	template<>
	template<>
	void object::test<2>()
	{
		geom1_ = GEOSGeomFromWKT("POLYGON((1 1,1 5,5 5,5 1,1 1))");
		geom2_ = GEOSGeomFromWKT("POINT(2 2)");

		ensure(0 != geom1_);
		ensure(0 != geom2_);

		geom3_ = GEOSIntersection(geom1_, geom2_);
		ensure(0 != geom3_);
		ensure_equals(toWKT(geom3_), std::string("POINT (2 2)"));
	}

	template<>
	template<>
	void object::test<3>()
	{
		geom1_ = GEOSGeomFromWKT("MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)))");
		geom2_ = GEOSGeomFromWKT("POLYGON((-1 1,-1 2,2 2,2 1,-1 1))");

		ensure(0 != geom1_);
		ensure(0 != geom2_);

		geom3_ = GEOSIntersection(geom1_, geom2_);

		ensure(0 != geom3_);
		ensure_equals(toWKT(geom3_), std::string("POLYGON ((0 1, 0 2, 2 2, 2 1, 0 1))"));
	}

  /* See http://trac.osgeo.org/geos/ticket/719 */
	template<>
	template<>
	void object::test<4>()
	{
		geom1_ = GEOSGeomFromWKT("MULTIPOLYGON(((0 0,5 10,10 0,0 0),(1 1,1 2,2 2,2 1,1 1),(100 100,100 102,102 102,102 100,100 100)))");
		geom2_ = GEOSGeomFromWKT("POLYGON((0 1,0 2,10 2,10 1,0 1))");

		ensure(0 != geom1_);
		ensure(0 != geom2_);

		geom3_ = GEOSIntersection(geom1_, geom2_);

		ensure(0 != geom3_);
		ensure_equals(toWKT(geom3_), std::string("GEOMETRYCOLLECTION (LINESTRING (1 2, 2 2), LINESTRING (2 1, 1 1), POLYGON ((0.5 1, 1 2, 1 1, 0.5 1)), POLYGON ((9 2, 9.5 1, 2 1, 2 2, 9 2)))"));
	}

} // namespace tut

