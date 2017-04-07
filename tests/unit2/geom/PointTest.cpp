////////////////////////////////////////////////////////////////////////////////
//
// geos::geom::Point class test
//
// Copyright (C) 2017 Mateusz Loskot <mateusz@loskot.net>
//
// This is free software; you can redistribute and/or modify it under
// the terms of the GNU Lesser General Public Licence as published
// by the Free Software Foundation.
// See the COPYING file for more information.
////////////////////////////////////////////////////////////////////////////////
#include <catch.hpp>

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
using namespace geos::geom;

TEST_CASE("Point")
{
	GeometryFactory::unique_ptr factory{GeometryFactory::create()};

	SECTION("empty")
	{
		std::unique_ptr<Point> p{factory->createPoint()};
		REQUIRE(p->isEmpty());
	}
}
