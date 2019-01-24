/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011      Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

//
// Test Suite for geos::algorithm::MinimumBoundingCircle

#include <tut/tut.hpp>
// geos
#include <geos/algorithm/MinimumBoundingCircle.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>

// std
#include <sstream>
#include <string>
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // dummy data, not used
    struct test_minimumboundingcircle_data {
        typedef geos::geom::Geometry Geometry;
        typedef geos::geom::Coordinate Coordinate;
        typedef geos::geom::PrecisionModel PrecisionModel;
        typedef geos::geom::GeometryFactory GeometryFactory;
        typedef geos::algorithm::MinimumBoundingCircle MinimumBoundingCircle;

        geos::io::WKTReader reader;
        std::unique_ptr<Geometry> geom;
        std::unique_ptr<Geometry> geomOut;
        GeometryFactory::Ptr geomFact = GeometryFactory::create();

        test_minimumboundingcircle_data()
        {}

        void doMinimumBoundingCircleTest(std::string wktIn, std::string wktOut,
            geos::geom::Coordinate &centreOut, double radiusOut)
        {

            geom.reset(reader.read(wktIn));
            MinimumBoundingCircle mbc(geom.get());
            std::vector<Coordinate> exPts = mbc.getExtremalPoints();
            std::unique_ptr<Geometry> actual(geomFact->createMultiPoint(exPts));
            double actualRadius = mbc.getRadius();
            geos::geom::Coordinate actualCentre = mbc.getCentre();
            //cout << "Centre = " << actualCentre << " Radius = " << actualRadius;

            geomOut.reset(reader.read(wktOut));
            bool isEqual = geomOut->equals(geom.get());
            // need this hack because apparently equals does not work for MULTIPOINT EMPTY
            if (geomOut->isEmpty() && geom->isEmpty())
                isEqual = true;
            // if (!isEqual) {
            //   System.out.println("Actual = " + actual + ", Expected = " + expected);
            // }
            ensure(isEqual);

            if (centreOut.isNull()) {
                ensure(centreOut.distance(actualCentre) < 0.0001);
            }
            if (radiusOut >= 0) {
                ensure(fabs(radiusOut - actualRadius) < 0.0001);
            }
        }

        void doMinimumBoundingCircleTest(std::string wktIn, std::string wktOut)
        {
            geos::geom::Coordinate c;
            c.setNull();
            doMinimumBoundingCircleTest(wktIn, wktOut, c, -1.0);
        }

    };

    typedef test_group<test_minimumboundingcircle_data> group;
    typedef group::object object;

    group test_minimumboundingcircle_group("geos::algorithm::MinimumBoundingCircle");






    //
    // Test Cases
    //


    template<>
    template<>
    void object::test<1>()
    {
        Coordinate c(10, 10);
        doMinimumBoundingCircleTest(
            "POINT (10 10)",
            "POINT (10 10)",
            c,
            0);

    }

} // namespace tut

