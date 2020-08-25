/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/


#include <iostream>
#include <fstream>

#include <geos/io/WKTWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/util/GEOSException.h>

using namespace std;
using namespace geos::io;
using namespace geos::geom;
using namespace geos::util;

int
main(int /*argc*/, char** /*argv*/)
{

    try {
        ofstream out("WKTOut");
        ifstream in("WKTIn");
        string instr;
        string outstr;
        PrecisionModel pm;
        GeometryFactory::Ptr gf = GeometryFactory::create(&pm, 10);
        WKTReader* r = new WKTReader(gf.get());
        WKTWriter* w = new WKTWriter();
        Geometry* g;

        cout << "Start Testing:" << endl;
        while(!in.eof()) {
            getline(in, instr);
            if(instr != "") {
                g = r->read(instr).release();
                outstr = w->write(g);
                out << "----------" << endl;
                out << instr << endl;
                out << outstr << endl;
                out << "----------" << endl << endl;
            }
        }
        out.flush();
        out.close();
        cout << "End of Testing" << endl;
        delete r;
        delete w;

    }
    catch(const GEOSException& ge) {
        cout << ge.what() << endl;
    }

    return 0;
}
