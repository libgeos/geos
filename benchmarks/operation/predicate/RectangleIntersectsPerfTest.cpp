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
#include <geos/profiler.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <memory>

using namespace geos::geom;
using namespace geos::io;
using namespace std;

class RectangleIntersectsPerfTest {
public:
    RectangleIntersectsPerfTest()
        :
        pm(),
        fact(GeometryFactory::create(&pm, 0))
    {}

    void
    test(int nPts)
    {
        double size = 100;
        Coordinate origin(0, 0);
        Geometry::Ptr sinePoly(
            createSineStar(origin, size, nPts)->getBoundary()
        );

        /*
         * Make the geometry "crinkly" by rounding off the points.
         * This defeats the  MonotoneChain optimization in the full relate
         * algorithm, and provides a more realistic test.
         */
        using geos::precision::SimpleGeometryPrecisionReducer;
        PrecisionModel p_pm(size / 10);
        SimpleGeometryPrecisionReducer reducer(&p_pm);
        Geometry::Ptr sinePolyCrinkly(reducer.reduce(sinePoly.get()));
        sinePoly.reset();

        Geometry& target = *sinePolyCrinkly;

        testRectangles(target, 30, 5);
    }


private:

    static const int MAX_ITER = 10;

    static const int NUM_AOI_PTS = 2000;
    static const int NUM_LINES = 5000;
    static const int NUM_LINE_PTS = 1000;

    PrecisionModel pm;
    GeometryFactory::Ptr fact;

    void
    testRectangles(const Geometry& target, int nRect, double rectSize)
    {
        vector<const Geometry*> rects;
        createRectangles(*target.getEnvelopeInternal(), nRect, rectSize, rects);
        test(rects, target);
        for (const Geometry* g: rects) {
            delete g;
        }
    }

    void
    test(vector<const Geometry*>& rect, const Geometry& g)
    {
        typedef vector<const Geometry*>::size_type size_type;

        geos::util::Profile sw("");
        sw.start();

        for(int i = 0; i < MAX_ITER; i++) {
            for(size_type j = 0; j < rect.size(); j++) {
                rect[j]->intersects(&g);
            }
        }

        sw.stop();
        cout << g.getNumPoints() << " points: " << sw.getTot() << " usecs" << endl;

    }

    // Push newly created geoms to rectLit
    void
    createRectangles(const Envelope& env, int nRect, double,
                     vector<const Geometry*>& rectList)
    {
        int nSide =  1 + (int)sqrt((double) nRect);
        double dx = env.getWidth() / nSide;
        double dy = env.getHeight() / nSide;

        for(int i = 0; i < nSide; i++) {
            for(int j = 0; j < nSide; j++) {
                double baseX = env.getMinX() + i * dx;
                double baseY = env.getMinY() + j * dy;
                Envelope envRect(
                    baseX, baseX + dx,
                    baseY, baseY + dy);
                Geometry* rect = fact->toGeometry(&envRect).release();
                rectList.push_back(rect);
            }
        }
    }

    Polygon::Ptr
    createSineStar(const Coordinate& origin,
                   double size, int nPts)
    {
        using geos::geom::util::SineStarFactory;

        SineStarFactory gsf(fact.get());
        gsf.setCentre(origin);
        gsf.setSize(size);
        gsf.setNumPoints(nPts);
        gsf.setArmLengthRatio(2);
        gsf.setNumArms(20);
        Polygon::Ptr poly = gsf.createSineStar();
        return poly;
    }


};

int
main()
{

    RectangleIntersectsPerfTest tester;

    tester.test(500);
    tester.test(100000);
}

