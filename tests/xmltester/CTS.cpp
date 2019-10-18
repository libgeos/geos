/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 **********************************************************************/


// CTS.cpp : Testing class
//

#include <iostream>

#include <geos/io/ByteOrderDataInStream.h>
#include <geos/io/ByteOrderValues.h>
#include <geos/io/ParseException.h>
#include <geos/io/WKBConstants.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/io/CLocalizer.h>
#include <geos/geom.h>

using namespace std;
using namespace geos;

int
main(int argc, char** argv)
{
    try {
        cout << "Start:" << endl;


        cout << "End" << endl;
    }
    catch(const GEOSException& ge) {
        cout << ge->toString() << endl;
    }

    return 0;
}

