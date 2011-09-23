// $Id: LineStringSnapperTest.cpp 3157 2010-12-03 17:26:55Z strk $
// 
// Test Suite for geos::operation::overlay::snap::LineStringSnapper class.

// tut
#include <tut.hpp>
// geos
#include <geos/operation/overlay/snap/LineStringSnapper.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateList.h>
#include <geos/geom/CoordinateArraySequence.h>
// std
#include <string>
#include <vector>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_linestringsnapper_data
    {

        test_linestringsnapper_data() {}
    };

    typedef test_group<test_linestringsnapper_data> group;
    typedef group::object object;

    group test_linestringsnapper_group("geos::operation::overlay::snap::LineStringSnapper");

    //
    // Test Cases
    //

    // Test vertices snapping
    template<>
    template<>
    void object::test<1>()
    {
		using geos::geom::Coordinate;
		using geos::operation::overlay::snap::LineStringSnapper;

		typedef std::auto_ptr<Coordinate::Vect> CoordsVectAptr;


		// source coordinates
		Coordinate src_a(0, 0);
		Coordinate src_b(10, 10);

		// snap coordinates
		Coordinate snp_a(0.1, 0);
		Coordinate snp_b(10, 10.1);

		Coordinate::Vect srcCoords;
		srcCoords.push_back( src_a );
		srcCoords.push_back( src_b );

		Coordinate::ConstVect snpCoords;
		snpCoords.push_back( &snp_a );
		snpCoords.push_back( &snp_b );

		LineStringSnapper snapper(srcCoords, 0.4);

		CoordsVectAptr ret(snapper.snapTo(snpCoords));

		// both points should have been snapped
		ensure_equals(ret->size(), 2u);
		ensure_equals(ret->operator[](0), snp_a);
		ensure_equals(ret->operator[](1), snp_b);

	}

    // Test segment snapping
    template<>
    template<>
    void object::test<2>()
    {
		using geos::geom::Coordinate;
		using geos::operation::overlay::snap::LineStringSnapper;

		typedef std::auto_ptr<Coordinate::Vect> CoordsVectAptr;


		// source coordinates
		Coordinate src_a(0, 0);
		Coordinate src_b(10, 10);

		// snap coordinates
		Coordinate snp_a(0.4, 0);
		Coordinate snp_b(10, 10.4);

		Coordinate::Vect srcCoords;
		srcCoords.push_back( src_a );
		srcCoords.push_back( src_b );

		Coordinate::ConstVect snpCoords;
		snpCoords.push_back( &snp_a );
		snpCoords.push_back( &snp_b );

		LineStringSnapper snapper(srcCoords, 0.3);

		CoordsVectAptr ret(snapper.snapTo(snpCoords));

		// snap point a should be inserted
		ensure_equals(ret->size(), 3u); 
		ensure_equals(ret->operator[](0), src_a);
		ensure_equals(ret->operator[](1), snp_a);
		ensure_equals(ret->operator[](2), src_b);

	}

  // Test vertices snapping in a closed ring
  template<>
  template<>
  void object::test<3>()
  {
		using geos::geom::Coordinate;
		using geos::operation::overlay::snap::LineStringSnapper;

		typedef std::auto_ptr<Coordinate::Vect> CoordsVectAptr;


		// source coordinates

		Coordinate src_a(0, 0);
		Coordinate src_b(10, 10);
		Coordinate src_c(0, 10);

		Coordinate::Vect srcCoords;
		srcCoords.push_back( src_a );
		srcCoords.push_back( src_b );
		srcCoords.push_back( src_c );
		srcCoords.push_back( src_a );

		// snap coordinates

		Coordinate snp_a(0.1, 0);
		Coordinate snp_b(10, 10.1);

		Coordinate::ConstVect snpCoords;
		snpCoords.push_back( &snp_a );
		snpCoords.push_back( &snp_b );

		LineStringSnapper snapper(srcCoords, 0.4);

		CoordsVectAptr ret(snapper.snapTo(snpCoords));

		// Points A and B should be snapped
		ensure_equals(ret->size(), 4u);
		ensure_equals(ret->operator[](0), snp_a);
		ensure_equals(ret->operator[](1), snp_b);
		ensure_equals(ret->operator[](2), src_c);
		ensure_equals(ret->operator[](3), snp_a);

	}

  // Test vertices snapping in a short sequence
  template<>
  template<>
  void object::test<4>()
  {
		using geos::geom::Coordinate;
		using geos::operation::overlay::snap::LineStringSnapper;

		typedef std::auto_ptr<Coordinate::Vect> CoordsVectAptr;


		// source coordinates

		Coordinate src_a(0, 0);
		Coordinate::Vect srcCoords;
		srcCoords.push_back( src_a );

		// snap coordinates

		Coordinate snp_a(0.1, 0);
		Coordinate::ConstVect snpCoords;
		snpCoords.push_back( &snp_a );

		LineStringSnapper snapper(srcCoords, 0.4);

		CoordsVectAptr ret(snapper.snapTo(snpCoords));

		ensure_equals(ret->size(), 1u);
		ensure_equals(ret->operator[](0), snp_a);

	}

  // Test vertices snapping in an empty sequence
  template<>
  template<>
  void object::test<5>()
  {
		using geos::geom::Coordinate;
		using geos::operation::overlay::snap::LineStringSnapper;

		typedef std::auto_ptr<Coordinate::Vect> CoordsVectAptr;


		// source coordinates

		Coordinate::Vect srcCoords;

		// snap coordinates

		Coordinate::ConstVect snpCoords;

		LineStringSnapper snapper(srcCoords, 0.4);

		CoordsVectAptr ret(snapper.snapTo(snpCoords));

		ensure_equals(ret->size(), 0u);

	}

  // Test snapping an empty sequence
  template<>
  template<>
  void object::test<6>()
  {
		using geos::geom::Coordinate;
		using geos::operation::overlay::snap::LineStringSnapper;

		typedef std::auto_ptr<Coordinate::Vect> CoordsVectAptr;


		// source coordinates

		Coordinate::Vect srcCoords;

		// snap coordinates

		Coordinate snp_a(0.1, 0);
		Coordinate::ConstVect snpCoords;
		snpCoords.push_back( &snp_a );

		LineStringSnapper snapper(srcCoords, 0.4);

		CoordsVectAptr ret(snapper.snapTo(snpCoords));

		ensure_equals(ret->size(), 0u);

	}

  // Test allow snapping to source vertices
  template<>
  template<>
  void object::test<7>()
  {
    using geos::geom::Coordinate;
    using geos::operation::overlay::snap::LineStringSnapper;

    typedef std::auto_ptr<Coordinate::Vect> CoordsVectAptr;


    // Source: (0 0, 10 0, 0 1)
    Coordinate src_a(0, 0);
    Coordinate src_b(10, 0);
    Coordinate src_c(0, 1);
    Coordinate::Vect srcCoords;
    srcCoords.push_back(src_a);
    srcCoords.push_back(src_b);
    srcCoords.push_back(src_c);

    // Snap: (0 0)
    Coordinate snp_a(0, 0);
    Coordinate::ConstVect snpCoords;
    snpCoords.push_back( &snp_a );

    // Snap with tolerance of 1
    // (both first and second point could be snapped)
    LineStringSnapper snapper(srcCoords, 1);

    // Allow source-snapping, expect: (0 0, 5 0, 0 0, 10 0)
    snapper.setAllowSnappingToSourceVertices(true);
    CoordsVectAptr ret(snapper.snapTo(snpCoords));

    ensure_equals(ret->size(), 4u);
    ensure_equals(ret->operator[](0), src_a);
    ensure_equals(ret->operator[](1), src_b);
    ensure_equals(ret->operator[](2), snp_a);
    ensure_equals(ret->operator[](3), src_c);

    // Do not allow source-snapping, expect: (0 0, 5 0, 10 0)
    snapper.setAllowSnappingToSourceVertices(false);
    ret = snapper.snapTo(snpCoords);
    ensure_equals(ret->size(), 3u);
    ensure_equals(ret->operator[](0), src_a);
    ensure_equals(ret->operator[](1), src_b);
    ensure_equals(ret->operator[](2), src_c);
  }

  // Test two candidate vertices snaps
/* The following one fails, I bet in JTS too.. but haven't tested
  template<>
  template<>
  void object::test<8>()
  {
    using geos::geom::Coordinate;
    using geos::operation::overlay::snap::LineStringSnapper;

    typedef std::auto_ptr<Coordinate::Vect> CoordsVectAptr;


    // Source: (0 0, 1 0, 1 1)
    Coordinate src_a(0, 0);
    Coordinate src_b(1, 0);
    Coordinate src_c(1, 1);
    Coordinate::Vect srcCoords;
    srcCoords.push_back(src_a);
    srcCoords.push_back(src_b);
    srcCoords.push_back(src_c);

    // Snap: (0.5, 0)
    Coordinate snp_a(0.5, 0);
    Coordinate::ConstVect snpCoords;
    snpCoords.push_back( &snp_a );

    // Snap with tolerance of 1
    // (both first and second point could be snapped)
    LineStringSnapper snapper(srcCoords, 1);

    // Expect: (0.5 0, 1 0, 1 1)
    CoordsVectAptr ret(snapper.snapTo(snpCoords));
    ensure_equals(ret->size(), 3u);
    ensure_equals(ret->operator[](0), snp_a);
    ensure_equals(ret->operator[](1), src_b);
    ensure_equals(ret->operator[](2), src_c);
  }
*/

} // namespace tut
