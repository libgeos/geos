//
// Test Suite for geos::operation::valid::IsValidOp class
// Ported from JTS junit/operation/valid/IsValidTest.java rev. 1.1

#include <tut/tut.hpp>
// geos
#include <geos/constants.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/operation/valid/TopologyValidationError.h>
#include <geos/constants.h> // for std::isnan
// std
#include <cmath>
#include <string>
#include <memory>

using namespace geos::geom;
//using namespace geos::operation;
using namespace geos::operation::valid;

namespace tut
{
    //
    // Test Group
    //

    struct test_isvalidop_data
    {
	typedef std::unique_ptr<Geometry> GeomPtr;
        typedef geos::geom::GeometryFactory GeometryFactory;

        geos::geom::PrecisionModel pm_;
        GeometryFactory::Ptr factory_;

        test_isvalidop_data()
			: pm_(1), factory_(GeometryFactory::create(&pm_, 0))
        {}
    };

    typedef test_group<test_isvalidop_data> group;
    typedef group::object object;

    group test_isvalidop_group("geos::operation::valid::IsValidOp");

    //
    // Test Cases
    //

    // 1 - testInvalidCoordinate
    template<>
    template<>
    void object::test<1>()
    {
	CoordinateSequence* cs = new CoordinateArraySequence();
	cs->add(Coordinate(0.0, 0.0));
	cs->add(Coordinate(1.0, geos::DoubleNotANumber));
	GeomPtr line ( factory_->createLineString(cs) );


	IsValidOp isValidOp(line.get());
	bool valid = isValidOp.isValid();

	TopologyValidationError* err = isValidOp.getValidationError();
    ensure(nullptr != err);
    const Coordinate& errCoord = err->getCoordinate();

	ensure_equals( err->getErrorType(),
	               TopologyValidationError::eInvalidCoordinate );

	ensure(0 != std::isnan(errCoord.y));
	ensure_equals(valid, false);
    }


} // namespace tut
