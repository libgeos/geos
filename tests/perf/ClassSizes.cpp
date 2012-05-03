/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011  Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * - Monitor class sizes
 *
 **********************************************************************/


#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/Geometry.h>
#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geomgraph/EdgeEnd.h>
#include <geos/geomgraph/PlanarGraph.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/profiler.h>
#include <geos/platform.h>
#include <iostream>

using namespace std;
using namespace geos;

#define check(x) \
	{ cout << "Size of " << #x << " is " << sizeof(x) << endl; }

int
main()
{
	check(geomgraph::PlanarGraph);
	check(geomgraph::EdgeEnd);
	check(geomgraph::DirectedEdge);
	check(noding::NodedSegmentString);
	check(int64);
}

