//
// Test Suite for geos::algorithm::distance::DiscreteDTWDistance


#include <tut.hpp>
// geos
#include <geos/platform.h>
#include <geos/io/WKTReader.h>
#include <geos/algorithm/distance/DiscreteDTWDistance.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h> // required for use in auto_ptr
#include <geos/geom/Coordinate.h>
// std
#include <cmath>
#include <sstream>
#include <string>
#include <memory>

namespace geos {
	namespace geom {
		class Geometry;
    class LineString;
	}
}

using namespace geos::geom;
using namespace geos::algorithm::distance; // for Location

namespace tut
{
	// Test Group

	// Test data, not used
	struct test_DiscreteDTWDistance_data {


    test_DiscreteDTWDistance_data()
      :
      pm(),
      gf(&pm),
      reader(&gf)
    {}

    static const double TOLERANCE;

    void runTest(const std::string& wkt1, const std::string& wkt2,
                 double expectedDistance)
    {
      const Geometry* g1 = reader.read(wkt1);
      const Geometry* g2 = reader.read(wkt2);
      const LineString* ls1 = dynamic_cast<const LineString*>(g1);
      const LineString* ls2 = dynamic_cast<const LineString*>(g2);

      double distance = DiscreteDTWDistance::distance(*ls1, *ls2);
      double diff = std::fabs(distance-expectedDistance);
      //std::cerr << "expectedDistance:" << expectedDistance << " actual distance:" << distance << std::endl;
      ensure( diff <= TOLERANCE );
    }

    PrecisionModel pm;
    GeometryFactory gf;
    geos::io::WKTReader reader;

	};
	const double test_DiscreteDTWDistance_data::TOLERANCE = 0.00001;

	typedef test_group<test_DiscreteDTWDistance_data> group;
	typedef group::object object;

	group test_DiscreteDTWDistance_group("geos::algorithm::distance::DiscreteDTWDistance");
	//
	// Test Cases
	//

	// Empty first string should throw an illegal argument exception.
	template<>
	template<>
	void object::test<1>()
	{
    try
    {
      runTest("LINESTRING EMPTY", "LINESTRING (0 0, 2 0)", 1.0);
    }
    catch (const geos::util::IllegalArgumentException& e)
    {
    }
	}

	// Empty second string should throw an illegal argument exception.
	template<>
	template<>
	void object::test<2>()
	{
    try
    {
      runTest("LINESTRING (0 0, 2 0)", "LINESTRING EMPTY", 1.0);
    }
    catch (const geos::util::IllegalArgumentException& e)
    {
    }
	}

	// Identical linestrings should have a DTW distance of zero.
	template<>
	template<>
	void object::test<3>()
	{
    runTest("LINESTRING (0 0, 2 0, 2 1, 3 2)", "LINESTRING (0 0, 2 0, 2 1, 3 2)", 0.0);
	}

  // DTW should not be zero, even for equivalent segments, when discretization is different.
  template<>
  template<>
  void object::test<4>()
  {
    runTest("LINESTRING (0 0, 3 0, 6 0)", "LINESTRING (0 0, 2 0, 4 0, 6 0)", 2.0);
  }

}

