// $Id$
// 
// Test Suite for geos::geom::Coordinate class.

// TUT
#include <tut.h>
// GEOS
#include <geos/geom/Coordinate.h>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_coordinate_data
    {
        const double x;
        const double y;
        const double z;

        test_coordinate_data()
            : x(1.234), y(2.345), z(3.456)
        {}
    };

    typedef test_group<test_coordinate_data> group;
    typedef group::object object;

    group test_coordinate_group("geos::geom::Coordiante");

    //
    // Test Cases
    //

    // Test of default constructor
    template<>
    template<>
    void object::test<1>()
    {
        // TODO - mloskot - discuss about adding default constructor
        ensure("NOTE: Coordinate has no default constructor.", true);
    }

    // Test of overriden constructor
    template<>
    template<>
    void object::test<2>()
    {
        geos::geom::Coordinate coord;
        ensure_equals( coord.x, 0.0 );
        ensure_equals( coord.y, 0.0 );
        ensure_equals( coord.z, DoubleNotANumber );
    }

    // Test of copy constructor and assignment operator
    template<>
    template<>
    void object::test<3>()
    {
        // Create exemplar object
        geos::geom::Coordinate original(x, y, z);
        ensure_equals( original.x, x );
        ensure_equals( original.y, y );
        ensure_equals( original.z, z );
        
        // Use copy ctor
        geos::geom::Coordinate copy(original);
        ensure_equals( copy.x, x );
        ensure_equals( copy.y, y );
        ensure_equals( copy.z, z );

        // Use assignment operator
        geos::geom::Coordinate assigned; // uses default ctor
        assigned = original;
        ensure_equals( assigned.x, x );
        ensure_equals( assigned.y, y );
        ensure_equals( assigned.z, z );
    }

    // Test of equality operators
    template<>
    template<>
    void object::test<4>()
    {
        geos::geom::Coordinate original(x, y, z);
        ensure_equals( original.x, x );
        ensure_equals( original.y, y );
        ensure_equals( original.z, z );
        
        // Use copy ctor
        geos::geom::Coordinate copy(original);

        // Compare copy with original
        ensure_equals( original, copy );

        copy.x += 3.14; // permutation
        ensure_not_equals( original, copy );
    }

    // Test of equals()
    template<>
    template<>
    void object::test<5>()
    {
        geos::geom::Coordinate first(x, y);
        geos::geom::Coordinate second(x, y);

        ensure( first.equals( second ) );
    }

    // Test of equals2D()
    template<>
    template<>
    void object::test<6>()
    {
        geos::geom::Coordinate first(x, y);
        geos::geom::Coordinate second(x, y);

        ensure( first.equals2D( second ) );
    }

    // Test of equals3D()
    template<>
    template<>
    void object::test<7>()
    {
        geos::geom::Coordinate first(x, y);
        geos::geom::Coordinate second(x, y);

        // Test with z = NaN
        ensure( first.equals3D( second ) );

        // Test all valid coordinates
        first.z = second.z = z;
        ensure( first.equals3D( second ) );
    }

    // Test of distance()
    template<>
    template<>
    void object::test<8>()
    {
        geos::geom::Coordinate first(x, y, z);
        geos::geom::Coordinate second(x, y, z);

        // Both coordinates are same
        ensure_equals( first.distance( second ), 0 );
        ensure_equals( first.distance( second ), second.distance( first ) );

        // Move second coordinate
        second.x += 10.0;
        second.y += 10.0;
        second.z += 10.0;
        ensure_not_equals( first.distance( second ), 0 );
        ensure_equals( first.distance( second ), second.distance( first ) );
        // TODO - mloskot - I'm not sure if this will work on every platform.
        // In case of any problems, just remove it.
        ensure_equals( first.distance( second ), 14.142135623730951);
    }

    // Test of set/getNull()
    template<>
    template<>
    void object::test<9>()
    {
        // Create not null coordinate
        geos::geom::Coordinate not_null_coord(x, y, z);
        ensure_equals( not_null_coord.x, x );
        ensure_equals( not_null_coord.y, y );
        ensure_equals( not_null_coord.z, z );

        // Make it null and check
        not_null_coord.setNull();
        ensure_equals( not_null_coord.x, DoubleNotANumber );
        ensure_equals( not_null_coord.y, DoubleNotANumber );
        ensure_equals( not_null_coord.z, DoubleNotANumber );

        // Build in static null instance
        geos::geom::Coordinate null_coord;
        null_coord = geos::geom::Coordinate::getNull();
        ensure_equals( null_coord.x, DoubleNotANumber );
        ensure_equals( null_coord.y, DoubleNotANumber );
        ensure_equals( null_coord.z, DoubleNotANumber );
    }

} // namespace tut

