#ifndef GEOS_OPERATION_H
#define GEOS_OPERATION_H

//#include <iostream>
//#include <string>
#include <vector>
#include <map>
//#include <algorithm>
//#include "math.h"
#include "platform.h"
#include "graph.h"
#include "geom.h"
#include "geosAlgorithm.h"

using namespace std;

class GeometryGraphOperation {
public:
	static CGAlgorithms *cga;
	static LineIntersector *li;
	GeometryGraphOperation(Geometry *g0,Geometry *g1);
	GeometryGraphOperation(Geometry *g0);
	Geometry* getArgGeometry(int i);
protected:
	/**
	* The operation args into an array so they can be accessed by index
	*/
	vector<GeometryGraph*> arg;  // the arg(s) of the operation
	bool makePrecise;
	void setComputationPrecision(PrecisionModel pm);
};

class EndpointInfo{
public:
	Coordinate pt;
	bool isClosed;
	int degree;
    EndpointInfo(Coordinate newPt);
	void addEndpoint(bool newIsClosed);
};
/**
 * This class tests whether some kinds of Geometry are simple.
 * Note that only Geometry's for which their definition allows them
 * to be simple or non-simple are tested.  (E.g. Polygons must be simple
 * by definition, so no test is provided.  To test whether a given Polygon is valid,
 * use <code>Geometry#isValid</code>)
 */
class IsSimpleOp {
public:
	IsSimpleOp();
	bool isSimple(LineString *geom);
	bool isSimple(MultiLineString *geom);
	bool isSimple(MultiPoint *mp);
	bool isSimpleLinearGeometry(Geometry *geom);
private:
	bool hasNonEndpointIntersection(GeometryGraph *graph);
	bool hasClosedEndpointIntersection(GeometryGraph *graph);
	void addEndpoint(map<Coordinate,EndpointInfo*,CoordLT> *endPoints,Coordinate p,bool isClosed);
};

#endif
