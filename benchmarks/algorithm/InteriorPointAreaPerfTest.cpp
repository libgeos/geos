/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011  Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: perf/operation/predicate/RectangleIntersectsPerfTest.java r378 (JTS-1.12)
 *
 **********************************************************************/


#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/util/GeometricShapeFactory.h>
#include <geos/precision/SimpleGeometryPrecisionReducer.h>
#include <geos/geom/util/SineStarFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Point.h>
#include <geos/profiler.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <memory>

using namespace geos::geom;
using namespace geos::io;
using namespace std;

class InteriorPointAreaPerfTest {
public:
    InteriorPointAreaPerfTest()
        :
        pm(),
        fact(GeometryFactory::create(&pm, 0))
    {
        showHeader();
    }

    void
    test(int nPts)
    {
        Coordinate origin(ORG_X, ORG_Y);
        std::unique_ptr<geos::geom::Polygon> sinePoly =
            createSineStar(origin, SIZE, nPts);

        /*
         * Make the geometry "crinkly" by rounding off the points.
         * This defeats the  MonotoneChain optimization in the full relate
         * algorithm, and provides a more realistic test.
         */
        using geos::precision::SimpleGeometryPrecisionReducer;
        double scale = nPts / SIZE;
        PrecisionModel p_pm(scale);
        SimpleGeometryPrecisionReducer reducer(&p_pm);
        std::unique_ptr<Geometry> sinePolyCrinkly(reducer.reduce(sinePoly.get()));
        sinePoly.reset();

        //cout << sinePolyCrinkly->toText() << endl;

        test(*sinePolyCrinkly);
    }

    const double ORG_X = 100.0;
    const double ORG_Y = 100.0;
    const double SIZE = 100.0;
    const int N_ARMS = 20;
    const double ARM_RATIO = 0.3;
    const int N_ITER = 100;

private:
    PrecisionModel pm;
    GeometryFactory::Ptr fact;

    void
    showHeader() {
        cout << "Interior Point Area perf test" << endl;
        cout << "# Iterations: " << N_ITER << endl;
        cout << "SineStar: origin: ("
                << ORG_X << ", " << ORG_Y
                << ")  size: " << SIZE
                << "  # arms: " << N_ARMS
                << "  arm ratio: " << ARM_RATIO
                << endl;
    }

    void
    test(geos::geom::Geometry& poly)
    {
        geos::util::Profile sw("");
        sw.start();

        for(int i = 0; i < N_ITER; i++) {
            std::unique_ptr<geos::geom::Point> pt( poly.getInteriorPoint() );
        }

        sw.stop();
        cout << poly.getNumPoints() << " points: " << sw.getTotFormatted() << endl;
    }

    std::unique_ptr<geos::geom::Polygon>
    createSineStar(const Coordinate& origin,
                   double size, int nPts)
    {
        using geos::geom::util::SineStarFactory;

        SineStarFactory gsf(fact.get());
        gsf.setCentre(origin);
        gsf.setSize(size);
        gsf.setNumPoints(nPts);
        gsf.setArmLengthRatio( ARM_RATIO );
        gsf.setNumArms( N_ARMS );
        std::unique_ptr<geos::geom::Polygon> poly = gsf.createSineStar();
        return poly;
    }
};

int
main()
{
    InteriorPointAreaPerfTest tester;

    tester.test(100);
    tester.test(1000);
    tester.test(10000);
    tester.test(100000);
    tester.test(1000000);
}

