#ifndef GEOS_OPERATION_H
#define GEOS_OPERATION_H

#include <memory>
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

namespace geos {

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
