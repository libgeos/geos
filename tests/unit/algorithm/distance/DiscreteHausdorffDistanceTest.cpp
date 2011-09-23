// 
// Test Suite for geos::algorithm::distance::DiscreteHausdorffDistance
// Ported from JTS junit/algorithm/distance/DiscreteHausdorffDistanceTest.java rev. 1.2


#include <tut.hpp>
// geos
#include <geos/platform.h>
#include <geos/io/WKTReader.h>
#include <geos/algorithm/distance/DiscreteHausdorffDistance.h>
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
	}
}

using namespace geos::geom; 
using namespace geos::algorithm::distance; // for Location

namespace tut
{
	//
	// Test Group
	//

	// Test data, not used
	struct test_DiscreteHausdorffDistance_data {

	typedef std::auto_ptr<Geometry> GeomPtr;

	test_DiscreteHausdorffDistance_data()
		:
		pm(),
		gf(&pm),
		reader(&gf)
	{}

	static const double TOLERANCE;

	void runTest(const std::string& wkt1, const std::string& wkt2,
	             double expectedDistance)
	{
		GeomPtr g1 ( reader.read(wkt1) );
		GeomPtr g2 ( reader.read(wkt2) );

		double distance = DiscreteHausdorffDistance::distance(*g1, *g2);
		double diff = std::fabs(distance-expectedDistance);
		//std::cerr << "expectedDistance:" << expectedDistance << " actual distance:" << distance << std::endl;
		ensure( diff <= TOLERANCE );
	}

	void runTest(const std::string& wkt1, const std::string& wkt2,
	             double densifyFactor, double expectedDistance)
	{
		GeomPtr g1 ( reader.read(wkt1) );
		GeomPtr g2 ( reader.read(wkt2) );

		double distance = DiscreteHausdorffDistance::distance(*g1,
			*g2, densifyFactor);
		double diff = std::fabs(distance-expectedDistance);
		//std::cerr << "expectedDistance:" << expectedDistance << " actual distance:" << distance << std::endl;
		ensure( diff <= TOLERANCE );
	}

	PrecisionModel pm;
	GeometryFactory gf;
        geos::io::WKTReader reader;

	};
	const double test_DiscreteHausdorffDistance_data::TOLERANCE = 0.00001;

	typedef test_group<test_DiscreteHausdorffDistance_data> group;
	typedef group::object object;

	group test_DiscreteHausdorffDistance_group("geos::algorithm::distace::DiscreteHausdorffDistance");



	//
	// Test Cases
	//

	// 1 - testLineSegments
	template<>
	template<>
	void object::test<1>()
	{         
runTest("LINESTRING (0 0, 2 1)", "LINESTRING (0 0, 2 0)", 1.0);
	}

	// 2 - testLineSegments2
	template<>
	template<>
	void object::test<2>()
	{         
runTest("LINESTRING (0 0, 2 0)", "LINESTRING (0 1, 1 2, 2 1)", 2.0);
	}

	// 3 - testLinePoints
	template<>
	template<>
	void object::test<3>()
	{         
runTest("LINESTRING (0 0, 2 0)", "MULTIPOINT (0 1, 1 0, 2 1)", 1.0);
	}

	// 4 - testLinesShowingDiscretenessEffect
	//
	// Shows effects of limiting HD to vertices
	// Answer is not true Hausdorff distance.
	//
	template<>
	template<>
	void object::test<4>()
	{         
runTest("LINESTRING (130 0, 0 0, 0 150)", "LINESTRING (10 10, 10 150, 130 10)", 14.142135623730951);
// densifying provides accurate HD
runTest("LINESTRING (130 0, 0 0, 0 150)", "LINESTRING (10 10, 10 150, 130 10)", 0.5, 70.0);
	}

} // namespace tut

