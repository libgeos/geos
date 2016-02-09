//
// Test Suite for C-API GEOSSTRtree

#include <tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace tut
{
	//
	// Test Group
	//

	// Common data used in test cases.
	struct test_capistrtree_data
	{
		test_capistrtree_data() {
			initGEOS(notice, notice);
		}

		static void notice(const char *fmt, ...)
		{
			std::fprintf( stdout, "NOTICE: ");

			va_list ap;
			va_start(ap, fmt);
			std::vfprintf(stdout, fmt, ap);
			va_end(ap);

			std::fprintf(stdout, "\n");
		}

	};

	typedef test_group<test_capistrtree_data> group;
	typedef group::object object;

	group test_capistrtree_group("capi::GEOSSTRtree");

	//
	// Test Cases
	//

	// Test GEOSSTRtree_nearest with a couple of points
	template<>
	template<>
	void object::test<1>()
	{
		GEOSGeometry* g1 = GEOSGeomFromWKT("POINT (3 3)");
		GEOSGeometry* g2 = GEOSGeomFromWKT("POINT (2 7)");
		GEOSGeometry* g3 = GEOSGeomFromWKT("POINT (5 4)");
		GEOSGeometry* g4 = GEOSGeomFromWKT("POINT (3 8)");

		GEOSSTRtree* tree = GEOSSTRtree_create(2);
		GEOSSTRtree_insert(tree, g1, g1);
		GEOSSTRtree_insert(tree, g2, g2);
		GEOSSTRtree_insert(tree, g3, g3);

		const GEOSGeometry* g5 = (GEOSGeometry*) GEOSSTRtree_nearest(tree, g4,
		(int (*)(const void* item1, const void* item2, double* distance))(GEOSDistance));
		ensure(g5 == g2);

		GEOSGeom_destroy(g1);
		GEOSGeom_destroy(g2);
		GEOSGeom_destroy(g3);
		GEOSGeom_destroy(g4);
		GEOSSTRtree_destroy(tree);
	}

	// Test GEOSSTRtree_nearest with more points.  This is important because we need to make sure the tree
	// actually has a couple of layers of depth.
	template<>
	template<>
	void object::test<2>()
	{
		int ngeoms = 100;
		std::vector<GEOSGeometry*> geoms;
		std::vector<GEOSGeometry*> queryPoints;
		GEOSSTRtree* tree = GEOSSTRtree_create(ngeoms);

		for (int i = 0; i < ngeoms; i++) {
			GEOSCoordSequence* seq = GEOSCoordSeq_create(1, 2);
			GEOSCoordSeq_setX(seq, 0, std::rand());
			GEOSCoordSeq_setY(seq, 0, std::rand());
			geoms.push_back(GEOSGeom_createPoint(seq));
			GEOSSTRtree_insert(tree, geoms[i], geoms[i]);
		}

		for (int i = 0; i < ngeoms; i++) {
			GEOSCoordSequence* seq = GEOSCoordSeq_create(1, 2);
			GEOSCoordSeq_setX(seq, 0, std::rand());
			GEOSCoordSeq_setY(seq, 0, std::rand());
			queryPoints.push_back(GEOSGeom_createPoint(seq));
		}

		for (int i = 0; i < ngeoms; i++) {
			const GEOSGeometry* nearest = (GEOSGeometry*) GEOSSTRtree_nearest(tree, queryPoints[i], NULL);
			const GEOSGeometry* nearestBruteForce = NULL;
			double nearestBruteForceDistance;
			for (int j = 0; j < ngeoms; j++) {
				double distance;
				GEOSDistance(queryPoints[i], geoms[j], &distance);

				if (nearestBruteForce == NULL || distance < nearestBruteForceDistance) {
					nearestBruteForce = geoms[j];
					nearestBruteForceDistance = distance;
				}
			}

			ensure(nearest == nearestBruteForce || GEOSEquals(nearest, nearestBruteForce));
		}

		for (int i = 0; i < ngeoms; i++) {
			GEOSGeom_destroy(geoms[i]);
			GEOSGeom_destroy(queryPoints[i]);
		}

		GEOSSTRtree_destroy(tree);
	}


} // namespace tut

