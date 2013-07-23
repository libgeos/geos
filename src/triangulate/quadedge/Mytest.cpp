#include <geos/triangulation/quadedgem/mytest.h>

#include <iostream>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/GEOSException.h>
#include <geos/triangulate/quadedge/QuadEdge.h>
#include <geos/triangulate/quadedge/QuadEdgeLocator.h>
#include <geos/triangulate/quadedge/LastFoundQuadEdgeLocator.h>
#include <geos/triangulate/quadedge/LocateFailureException.h>
#include <geos/triangulate/quadedge/TriangleVisitor.h>


using namespace std;

namespace geos{
namespace triangulate{
namespace quadedge{

void mytest::myfun(vector<geom::Geometry *> &cells , const geom::GeometryFactory& geomFact)
{
	cout << "this is my function" << endl;

	wEF WEF WEF 
}

}
}
}
