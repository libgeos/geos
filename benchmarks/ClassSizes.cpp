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
 * - Monitor class sizes
 *
 **********************************************************************/


#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/FixedSizeCoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geomgraph/Depth.h>
#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/EdgeEnd.h>
#include <geos/geomgraph/PlanarGraph.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/profiler.h>
#include <geos/constants.h>
#include <iostream>
#include <geos/geomgraph/index/SweepLineEvent.h>
#include <geos/triangulate/quadedge/QuadEdge.h>
#include <geos/triangulate/quadedge/QuadEdgeQuartet.h>
#include <geos/triangulate/quadedge/Vertex.h>

using namespace std;
using namespace geos;

#define check(x) \
	{ cout << "Size of " << #x << " is " << sizeof(x) << endl; }

int
main()
{
    check(geomgraph::Depth);
    check(geomgraph::DirectedEdge);
    check(geomgraph::DirectedEdgeStar);
    check(geomgraph::Edge);
    check(geomgraph::EdgeEnd);
    check(geomgraph::PlanarGraph);
    check(geomgraph::TopologyLocation);
    check(geomgraph::index::SweepLineEvent);
    check(noding::NodedSegmentString);
    check(geom::Geometry);
    check(geom::Point);
    check(geom::LineString);
    check(geom::LinearRing);
    check(geom::Polygon);
    check(geom::GeometryCollection);
    check(geom::MultiPoint);
    check(geom::MultiLineString);
    check(geom::MultiPolygon);
    check(geom::CoordinateArraySequence);
    check(geom::FixedSizeCoordinateSequence<1>);
    check(geom::FixedSizeCoordinateSequence<2>);
    check(triangulate::quadedge::QuadEdge);
    check(triangulate::quadedge::QuadEdgeQuartet);
    check(triangulate::quadedge::Vertex);
    check(int64);
}

