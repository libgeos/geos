#ifndef GEOS_TRIANGULATE_QUADEDGE_QUADEDGESUBDIVISION_H
#define GEOS_TRIANGULATE_QUADEDGE_QUADEDGESUBDIVISION_H

#include <memory>
#include <list>
#include <stack>
#include <set>

#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/MultiLineString.h>
#include <geos/triangulate/quadedge/QuadEdgeLocator.h>
#include <geos/triangulate/quadedge/Vertex.h>

namespace geos{
namespace geom{
	class CoordinateSequence;
	class GeometryCollection;
	class GeometryFactory;
	class Coordinate;
	class Geometry;

}

namespace triangulate{

namespace quadedge{

class GEOS_DLL mytest{
public:
	void myfun(vector<geom::Geometry *> &cells , const geom::GeometryFactory& geomFact);
};
}
}
}

