/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_OPERATION_H
#define GEOS_OPERATION_H

#include <memory>
#include <vector>
#include <map>
#include <geos/platform.h>
#include <geos/geomgraph.h>
#include <geos/geom.h>
#include <geos/geosAlgorithm.h>

//using namespace std;

namespace geos {
namespace operation { // geos.operation

/*
 * \brief
 * The base class for operations that require GeometryGraph
 */
class GeometryGraphOperation {
friend class Unload;
public:
	GeometryGraphOperation(const Geometry *g0,const Geometry *g1);
	GeometryGraphOperation(const Geometry *g0);
	virtual ~GeometryGraphOperation();
	const Geometry* getArgGeometry(int i) const;
protected:
	// to be obsoleted probably, CGAlgorithms is an all-static class
	//static algorithm::CGAlgorithms *cga;
	static algorithm::LineIntersector *li;
	const PrecisionModel* resultPrecisionModel;
	/*
	 * The operation args into an array so they can be accessed by index
	 */
	std::vector<geomgraph::GeometryGraph*> arg;  // the arg(s) of the operation
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
	bool hasNonEndpointIntersection(geomgraph::GeometryGraph &graph);
	bool hasClosedEndpointIntersection(geomgraph::GeometryGraph &graph);
	void addEndpoint(map<const Coordinate*,EndpointInfo*,CoordinateLessThen>&endPoints, const Coordinate *p,bool isClosed);
};

} // namespace geos.operation
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.5  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.4  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.3  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
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

