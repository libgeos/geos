/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.2  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.15  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.14  2004/03/19 09:48:46  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.13  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_OPERATION_H
#define GEOS_OPERATION_H

#include <memory>
//#include <iostream>
//#include <string>
#include <vector>
#include <map>
//#include <algorithm>
//#include "math.h"
#include <geos/platform.h>
#include <geos/geomgraph.h>
#include <geos/geom.h>
#include <geos/geosAlgorithm.h>

using namespace std;

namespace geos {

/*
 * The base class for operations that require {@link GeometryGraph)s.
 *
 */
class GeometryGraphOperation {
friend class Unload;
public:
	GeometryGraphOperation(const Geometry *g0,const Geometry *g1);
	GeometryGraphOperation(const Geometry *g0);
	virtual ~GeometryGraphOperation();
	const Geometry* getArgGeometry(int i) const;
protected:
	static CGAlgorithms *cga;
	static LineIntersector *li;
	const PrecisionModel* resultPrecisionModel;
	/**
	* The operation args into an array so they can be accessed by index
	*/
	vector<GeometryGraph*> *arg;  // the arg(s) of the operation
	void setComputationPrecision(const PrecisionModel* pm);
};

class EndpointInfo{
public:
	Coordinate pt;
	bool isClosed;
	int degree;
    	EndpointInfo(const Coordinate& newPt);
	void addEndpoint(bool newIsClosed);
};

/*
 * Tests whether a {@link Geometry} is simple.
 * Only {@link Geometry}s whose definition allows them
 * to be simple or non-simple are tested.  (E.g. Polygons must be simple
 * by definition, so no test is provided.  To test whether a given Polygon is valid,
 * use <code>Geometry#isValid</code>)
 *
 */
class IsSimpleOp {
public:
	IsSimpleOp();
	bool isSimple(const LineString *geom);
	bool isSimple(const MultiLineString *geom);
	bool isSimple(const MultiPoint *mp);
	bool isSimpleLinearGeometry(const Geometry *geom);
private:
	bool hasNonEndpointIntersection(GeometryGraph *graph);
	bool hasClosedEndpointIntersection(GeometryGraph *graph);
	void addEndpoint(map<Coordinate,EndpointInfo*,CoordLT> *endPoints, const Coordinate& p,bool isClosed);
};
}
#endif
