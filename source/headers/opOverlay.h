#ifndef GEOS_OPOVERLAY_H
#define GEOS_OPOVERLAY_H

#include <string>
#include <vector>
#include <map>
#include "platform.h"
#include "graph.h"


class OverlayNodeFactory: public NodeFactory {
public:
	OverlayNodeFactory() {};
};

/**
 * A MaximalEdgeRing is a ring of edges which may contain nodes of degree > 2.
 * A MaximalEdgeRing may represent two different spatial entities:
 * <ul>
 * <li>a single polygon possibly containing inversions (if the ring is oriented CW)
 * <li>a single hole possibly containing exversions (if the ring is oriented CCW)
 * </ul>
 * If the MaximalEdgeRing represents a polygon,
 * the interior of the polygon is strongly connected.
 * <p>
 * These are the form of rings used to define polygons under some spatial data models.
 * However, under the OGC SFS model, {@link MinimalEdgeRings} are required.
 * A MaximalEdgeRing can be converted to a list of MinimalEdgeRings using the
 * {@link #buildMinimalRings() } method.
 *
 * @version 1.2
 * @see com.vividsolutions.jts.operation.overlay.MinimalEdgeRing
 */
class MaximalEdgeRing: public EdgeRing {
public:
	MaximalEdgeRing(DirectedEdge *start, GeometryFactory *geometryFactory, CGAlgorithms *cga);
	DirectedEdge* getNext(DirectedEdge *de);
	void setEdgeRing(DirectedEdge* de,EdgeRing* er);
protected:
	void linkDirectedEdgesForMinimalEdgeRings();
	vector<EdgeRing*>* buildMinimalRings();
};


/**
 * A MinimalEdgeRing is a ring of edges with the property that no node
 * has degree greater than 2.  These are the form of rings required
 * to represent polygons under the OGC SFS spatial data model.
 *
 * @version 1.2
 * @see com.vividsolutions.jts.operation.overlay.MaximalEdgeRing
 */
class MinimalEdgeRing: public EdgeRing {
public:
	MinimalEdgeRing(DirectedEdge *start,GeometryFactory *geometryFactory,CGAlgorithms *cga);
	DirectedEdge* getNext(DirectedEdge *de);
	void setEdgeRing(DirectedEdge *de,EdgeRing *er);
};
#endif
