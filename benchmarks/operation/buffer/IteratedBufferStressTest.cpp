/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009  Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: perf/operation/buffer/IteratedBufferStressTest.java rev 1.1
 *
 * - Added exit condition when number of vertices is zero
 *
 **********************************************************************/


#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/Geometry.h>
#include <geos/profiler.h>
#include <iostream>

using namespace geos::geom;
using namespace geos::io;
using namespace std;

typedef unique_ptr<Geometry> GeomPtr;

GeomPtr
doBuffer(const Geometry& g, double dist)
{
    cout << "Buffering with dist = " << dist << endl;
    GeomPtr buf(g.buffer(dist));
    cout << "Buffer result has " <<  buf->getNumPoints() << " vertices" << endl;

//cout << *buf << endl;

    return buf;
}

// throws Exception
void
run(const Geometry* base)
{
    GeomPtr tmp;

    // profile here
    geos::util::Profile totalSW("buffer");
    double dist = 1.0;
    while(true) {

        totalSW.start();

        GeomPtr b1 = doBuffer(*base, dist);
        GeomPtr b2 = doBuffer(*b1, -dist);

        totalSW.stop();
        cout << "----------------------  "
             << totalSW << endl; // totalSW.getTimeString() << endl;

        dist += 1;
        base = b2.get();
        tmp = std::move(b2); // move as anti-optimisation?

        if(! base->getNumPoints()) {
            break;
        }
    }
}

int
main()
{
    PrecisionModel pm;
    GeometryFactory::Ptr gf = GeometryFactory::create(&pm);
    WKTReader rdr(gf.get());

    string inputWKT =
        "POLYGON ((110 320, 190 220, 60 200, 180 120, 120 40, 290 150, 410 40, 410 230, 500 340, 320 310, 260 370, 220 310, 110 320), (220 260, 250 180, 290 220, 360 150, 350 250, 260 280, 220 260))";

    GeomPtr base(rdr.read(inputWKT));
    run(base.get());
}

