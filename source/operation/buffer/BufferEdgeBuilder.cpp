/*
 * $Log$
 * Revision 1.6  2003/11/06 17:47:33  strk
 * Added support for LinearRing, removed memory leaks in ::addLineString
 *
 */

#include "../../headers/opBuffer.h"
#include <typeinfo>

namespace geos {

BufferEdgeBuilder::BufferEdgeBuilder(CGAlgorithms *newCga,LineIntersector *li,double newDistance,const PrecisionModel *precisionModel,int quadrantSegments) {
	cga=newCga;
	distance=newDistance;
	//lineBuilder=new BufferMultiLineBuilder(cga, li);
	lineBuilder=new BufferLineBuilder(cga,li,precisionModel,quadrantSegments);
	edgeList=new vector<Edge*>();
}

BufferEdgeBuilder::~BufferEdgeBuilder() {
	delete lineBuilder;
//	for (int i=0;i<(int)edgeList->size();i++) {
//		delete (*edgeList)[i];
//	}
	delete edgeList;
}

vector<Edge*>* BufferEdgeBuilder::getEdges(const Geometry *geom){
	add(geom);
	return edgeList;
}

void BufferEdgeBuilder::addEdges(vector<CoordinateList*> *lineList, int leftLoc, int rightLoc){
	for(int i=0;i<(int)lineList->size();i++) {
		CoordinateList *coords=(*lineList)[i];
		addEdge(coords, leftLoc, rightLoc);
	}
}

/**
*Creates an edge for a coordinate list which is a ring of a buffer,
*and adds it to the list of buffer edges.
*The ring may be oriented in either direction.
*If the ring is oriented CW, the locations will be:
*<br>Left: Location.EXTERIOR
*<br>Right: Location.INTERIOR
*/
void BufferEdgeBuilder::addEdge(const CoordinateList *coord, int leftLoc, int rightLoc){
	// don't add null buffers!
	if (coord->getSize()<2) return;
	// add the edge for a coordinate list which is a ring of a buffer
	Edge *e=new Edge(CoordinateListFactory::internalFactory->createCoordinateList(coord),new Label(0,Location::BOUNDARY, leftLoc, rightLoc));
	edgeList->push_back(e);
}

void BufferEdgeBuilder::add(const Geometry *g){
	if (g->isEmpty()) return;
	if (typeid(*g)==typeid(Polygon)) addPolygon((Polygon*) g);
	else if (typeid(*g)==typeid(LineString) || typeid(*g)==typeid(LinearRing)) addLineString((LineString*) g);
	else if (typeid(*g)==typeid(Point)) addPoint((Point*) g);
	else if (typeid(*g)==typeid(MultiPoint)) addCollection((MultiPoint*) g);
	else if (typeid(*g)==typeid(MultiLineString)) addCollection((MultiLineString*) g);
	else if (typeid(*g)==typeid(MultiPolygon)) addCollection((MultiPolygon*) g);
	else if (typeid(*g)==typeid(GeometryCollection)) addCollection((GeometryCollection*) g);
	else throw new UnsupportedOperationException(typeid(*g).name());
}

void BufferEdgeBuilder::addCollection(const GeometryCollection *gc) {
	for (int i=0;i<gc->getNumGeometries(); i++) {
		const Geometry *g=gc->getGeometryN(i);
		add(g);
	}
}

/**
*Add a Point to the graph.
*/
void BufferEdgeBuilder::addPoint(const Point *p) {
	if (distance<=0.0) return;
	CoordinateList *coord=p->getCoordinates();
	vector<CoordinateList*> *lineList=lineBuilder->getLineBuffer(coord,distance);
	addEdges(lineList,Location::EXTERIOR, Location::INTERIOR);
	delete coord;
}


void BufferEdgeBuilder::addLineString(const LineString *line) {
	if (distance<=0.0) return;
	CoordinateList *coord=CoordinateList::removeRepeatedPoints(line->getCoordinatesRO());
	vector<CoordinateList*> *lineList=lineBuilder->getLineBuffer(coord,distance);
	addEdges(lineList,Location::EXTERIOR, Location::INTERIOR);
	delete coord;
}

void BufferEdgeBuilder::addPolygon(const Polygon *p) {
	double lineDistance=distance;
	int side=Position::LEFT;
	if (distance<0.0) {
		lineDistance=-distance;
		side=Position::RIGHT;
	}
	int holeSide=(side==Position::LEFT) ? Position::RIGHT : Position::LEFT;
	addPolygonRing((LinearRing*) p->getExteriorRing(),lineDistance,side,Location::EXTERIOR,Location::INTERIOR);
	for (int i=0; i<p->getNumInteriorRing(); i++) {
		// Holes are topologically labelled opposite to the shell, since
		// the interior of the polygon lies on their opposite side
		// (on the left, if the hole is oriented CCW)
		addPolygonRing((LinearRing*) p->getInteriorRingN(i),lineDistance,Position::opposite(side),Location::INTERIOR,Location::EXTERIOR);
	}
}

/**
*The side and left and right topological location arguments assume that the ring is oriented CW.
*If the ring is in the opposite orientation,
*the left and right locations must be interchanged and the side flipped.
*
*@param lr the LinearRing around which to create the buffer
*@param distance the distance at which to create the buffer
*@param side the side of the ring on which to construct the buffer line
*@param cwLeftLoc the location on the L side of the ring (if it is CW)
*@param cwRightLoc the location on the R side of the ring (if it is CW)
*/
void BufferEdgeBuilder::addPolygonRing(const LinearRing *lr, double distance, int side, int cwLeftLoc, int cwRightLoc){
	CoordinateList *coord=CoordinateList::removeRepeatedPoints(lr->getCoordinatesRO());
	int leftLoc=cwLeftLoc;
	int rightLoc=cwRightLoc;
	if (cga->isCCW(coord)) {
		leftLoc=cwRightLoc;
		rightLoc=cwLeftLoc;
		side=Position::opposite(side);
	}
	vector<CoordinateList*> *lineList=lineBuilder->getRingBuffer(coord,side,distance);
	addEdges(lineList,leftLoc,rightLoc);
	/*
	Edge e=new Edge(coord,
	new Label(0, Location.BOUNDARY, left, right));
	insertEdge(e);
	// insert the endpoint as a node, to mark that it is on the boundary
	insertPoint(argIndex, coord[0], Location.BOUNDARY);
	*/

	delete coord;
}
}

