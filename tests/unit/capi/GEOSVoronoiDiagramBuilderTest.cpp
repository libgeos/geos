// 
// Test Suite for C-API GEOSVoronoiDiagramBuilder

#include <tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // Common data used in test cases.
    struct test_capigeosvoronoidiagrambuilder_data
    {
        GEOSGeometry* geom1_;
        GEOSGeometry* geom2_;
        GEOSWKTWriter* w_;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capigeosvoronoidiagrambuilder_data()
            : geom1_(0), geom2_(0)
        {
            initGEOS(notice, notice);
            w_ = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(w_, 1);
        }

        void ensure_equals_wkt(GEOSGeometry* g, const char* exp)
        {
          GEOSNormalize(g);
          char* wkt_c = GEOSWKTWriter_write(w_, g);
          std::string out(wkt_c);
          free(wkt_c);

	  GEOSGeometry* exp_g = GEOSGeomFromWKT(exp);
	  GEOSNormalize(exp_g);
	  char* wkt_c2 = GEOSWKTWriter_write(w_, exp_g); 
	  std::string out_exp(wkt_c2);
	  free(wkt_c2);

//	  cout << "OUTPUT STRING::" << out << endl << endl;
//	  cout << "Expected STRING::" << out_exp << endl << endl;
          ensure_equals(out, out_exp);
        }

        ~test_capigeosvoronoidiagrambuilder_data()
        {
            GEOSGeom_destroy(geom1_);
            GEOSGeom_destroy(geom2_);
            GEOSWKTWriter_destroy(w_);
            geom1_ = 0;
            geom2_ = 0;
            finishGEOS();
        }

    };

    typedef test_group<test_capigeosvoronoidiagrambuilder_data> group;
    typedef group::object object;

    group test_capigeosvoronoidiagrambuilder_group("capi::GEOSVoronoiDiagram");

    //
    // Test Cases
    //

    // Single point
    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeomFromWKT("POINT(10 20)");

        geom2_ = GEOSVoronoiDiagramBuilder(geom1_,0);
        ensure_equals ( GEOSisEmpty(geom2_), 1 );
        ensure_equals ( GEOSGeomTypeId(geom2_), GEOS_GEOMETRYCOLLECTION );
    }

    //More points:
    template<>
    template<>
    void object::test<2>()
    {
	    geom1_ = GEOSGeomFromWKT("MULTIPOINT ((280 300), (420 330), (380 230), (320 160))");

	    geom2_ = GEOSVoronoiDiagramBuilder(geom1_,0);
	    ensure_equals_wkt(geom2_ ,"GEOMETRYCOLLECTION (POLYGON ((110 175.71428571428572, 110 500, 310.35714285714283 500, 353.515625 298.59375, 306.875 231.96428571428572, 110 175.71428571428572)), POLYGON ((590 204, 590 -10, 589.1666666666666 -10, 306.875 231.96428571428572, 353.515625 298.59375, 590 204)), POLYGON ((110 -10, 110 175.71428571428572, 306.875 231.96428571428572, 589.1666666666666 -10, 110 -10)), POLYGON ((310.35714285714283 500, 590 500, 590 204, 353.515625 298.59375, 310.35714285714283 500)))" );
    }
    //Larger number of points:
    template<>
    template<>
    void object::test<3>()
    {
	    geom1_ = GEOSGeomFromWKT("MULTIPOINT ((170 270), (270 270), (230 310), (180 330), (250 340), (315 318), (330 260), (240 170), (220 220), (270 220))");

	    geom2_ = GEOSVoronoiDiagramBuilder(geom1_,0);
	    ensure_equals_wkt(geom2_,"GEOMETRYCOLLECTION (POLYGON ((0 329.1666666666667, 0 510, 190 510, 213.94736842105263 342.36842105263156, 195.625 296.5625, 0 329.1666666666667)), POLYGON ((0 76.50000000000001, 0 329.1666666666667, 195.625 296.5625, 216 266, 88.33333333333333 138.33333333333334, 0 76.50000000000001)), POLYGON ((216 266, 195.625 296.5625, 213.94736842105263 342.36842105263156, 267 307, 225 265, 216 266)), POLYGON ((245 245, 225 265, 267 307, 275.9160583941606 309.54744525547443, 303.1666666666667 284, 296.6666666666667 245, 245 245)), POLYGON ((225 265, 245 245, 245 201, 88.33333333333333 138.33333333333334, 216 266, 225 265)), POLYGON ((0 0, 0 76.50000000000001, 88.33333333333333 138.33333333333334, 245 201, 380 120, 500 0, 0 0)), POLYGON ((190 510, 343.76153846153846 510, 275.9160583941606 309.54744525547443, 267 307, 213.94736842105263 342.36842105263156, 190 510)), POLYGON ((245 201, 245 245, 296.6666666666667 245, 380 120, 245 201)), POLYGON ((343.76153846153846 510, 500 510, 500 334.9051724137931, 303.1666666666667 284, 275.9160583941606 309.54744525547443, 343.76153846153846 510)), POLYGON ((500 334.9051724137931, 500 0, 380 120, 296.6666666666667 245, 303.1666666666667 284, 500 334.9051724137931)))");
    }

} // namespace tut

