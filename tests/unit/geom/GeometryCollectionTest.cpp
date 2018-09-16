//
// Test Suite for geos::geom::GeometryCollection class.

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryCollection.h>
// std
#include <vector>

namespace tut
{
	//
	// Test Group
	//

	// Common data used by tests
	struct test_geometrycollection_data {};

	typedef test_group<test_geometrycollection_data> group;
	typedef group::object object;

	group test_geometrycollection_group("geos::geom::GeometryCollection");

	//
	// Test Cases
	//

	// Test of default constructor
	template<>
	template<>
	void object::test<1>()
	{
		using geos::geom::GeometryFactory;
		geos::geom::PrecisionModel pm;
		auto gf = GeometryFactory::create(&pm, 1);
		auto g = gf->createEmptyGeometry();

		g->setSRID(0);
		std::vector<decltype(g)> v = {g};
		auto geom_col = gf->createGeometryCollection(v);
		ensure_equals(geom_col->getGeometryN(0)->getSRID(), 1);

		geom_col->setSRID(2);
		ensure_equals(geom_col->getGeometryN(0)->getSRID(), 2);

		auto clone = geom_col->clone();
		ensure_equals(clone->getGeometryN(0)->getSRID(), 2);

		// FREE MEMORY
		gf->destroyGeometry(geom_col);
		gf->destroyGeometry(clone);
	}
} // namespace tut
