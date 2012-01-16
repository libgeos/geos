/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/OverlayOp.java rev. 1.31 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_OP_OVERLAY_OVERLAYOP_H
#define GEOS_OP_OVERLAY_OVERLAYOP_H

#include <geos/export.h>

#include <geos/operation/GeometryGraphOperation.h> // for inheritance
#include <geos/geomgraph/EdgeList.h> // for composition
#include <geos/algorithm/PointLocator.h> // for composition
#include <geos/geomgraph/PlanarGraph.h> // for inline (GeometryGraph->PlanarGraph)

#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
	namespace geom {
		class Geometry;
		class Coordinate;
		class GeometryFactory;
		class Polygon;
		class LineString;
		class Point;
	}
	namespace geomgraph {
		class Label;
		class Edge;
		class Node;
	}
	namespace operation {
		namespace overlay {
			class ElevationMatrix;
		}
	}
}

namespace geos {
namespace operation { // geos::operation
namespace overlay { // geos::operation::overlay

/// Computes the overlay of two Geometry. 
//
/// The overlay can be used to determine any
/// boolean combination of the geometries.
///
class GEOS_DLL OverlayOp: public GeometryGraphOperation {

public:

	/// The spatial functions supported by this class.
	//
	/// These operations implement various boolean combinations of
	/// the resultants of the overlay.
	///
	enum OpCode {
		opINTERSECTION=1,
		opUNION,
		opDIFFERENCE,
		opSYMDIFFERENCE
	};

	static geom::Geometry* overlayOp(const geom::Geometry *geom0,
			const geom::Geometry *geom1,
			OpCode opCode);
		//throw(TopologyException *);

	static bool isResultOfOp(const geomgraph::Label& label, OpCode opCode);

	/// This method will handle arguments of Location.NULL correctly
	//
	/// @return true if the locations correspond to the opCode
	///
	static bool isResultOfOp(int loc0, int loc1, OpCode opCode);

	/// Construct an OverlayOp with the given Geometry args.
	// 
	/// Ownership of passed args will remain to caller, and
	/// the OverlayOp won't change them in any way.
	///
	OverlayOp(const geom::Geometry *g0, const geom::Geometry *g1);

	virtual ~OverlayOp(); // FIXME: virtual ?

	geom::Geometry* getResultGeometry(OpCode funcCode);
		// throw(TopologyException *);

	geomgraph::PlanarGraph& getGraph() { return graph; }

	/** \brief
	 * This method is used to decide if a point node should be included
	 * in the result or not.
	 *
	 * @return true if the coord point is covered by a result Line
	 * or Area geometry
	 */
	bool isCoveredByLA(const geom::Coordinate& coord);

	/** \brief
	 * This method is used to decide if an L edge should be included
	 * in the result or not.
	 *
	 * @return true if the coord point is covered by a result Area geometry
	 */
	bool isCoveredByA(const geom::Coordinate& coord);

	/*
	 * @return true if the coord is located in the interior or boundary of
	 * a geometry in the list.
	 */

protected:

	/** \brief
	 * Insert an edge from one of the noded input graphs.
	 *
	 * Checks edges that are inserted to see if an
	 * identical edge already exists.
	 * If so, the edge is not inserted, but its label is merged
	 * with the existing edge.
	 */
	void insertUniqueEdge(geomgraph::Edge *e);

private:

	algorithm::PointLocator ptLocator;

	const geom::GeometryFactory *geomFact;

	geom::Geometry *resultGeom;

	geomgraph::PlanarGraph graph;

	geomgraph::EdgeList edgeList;

	std::vector<geom::Polygon*> *resultPolyList;

	std::vector<geom::LineString*> *resultLineList;

	std::vector<geom::Point*> *resultPointList;

	void computeOverlay(OpCode opCode); // throw(TopologyException *);

	void insertUniqueEdges(std::vector<geomgraph::Edge*> *edges);

	/*
	 * If either of the GeometryLocations for the existing label is
	 * exactly opposite to the one in the labelToMerge,
	 * this indicates a dimensional collapse has happened.
	 * In this case, convert the label for that Geometry to a Line label
	 */
	//Not needed
	//void checkDimensionalCollapse(geomgraph::Label labelToMerge, geomgraph::Label existingLabel);

	/** \brief
	 * Update the labels for edges according to their depths.
	 *
	 * For each edge, the depths are first normalized.
	 * Then, if the depths for the edge are equal,
	 * this edge must have collapsed into a line edge.
	 * If the depths are not equal, update the label
	 * with the locations corresponding to the depths
	 * (i.e. a depth of 0 corresponds to a Location of EXTERIOR,
	 * a depth of 1 corresponds to INTERIOR)
	 */
	void computeLabelsFromDepths();

	/** \brief
	 * If edges which have undergone dimensional collapse are found,
	 * replace them with a new edge which is a L edge
	 */
	void replaceCollapsedEdges();

	/** \brief
	 * Copy all nodes from an arg geometry into this graph.
	 *
	 * The node label in the arg geometry overrides any previously
	 * computed label for that argIndex.
	 * (E.g. a node may be an intersection node with
	 * a previously computed label of BOUNDARY,
	 * but in the original arg Geometry it is actually
	 * in the interior due to the Boundary Determination Rule)
	 */
	void copyPoints(int argIndex);

	/** \brief
	 * Compute initial labelling for all DirectedEdges at each node.
	 *
	 * In this step, DirectedEdges will acquire a complete labelling
	 * (i.e. one with labels for both Geometries)
	 * only if they
	 * are incident on a node which has edges for both Geometries
	 */
	void computeLabelling(); // throw(TopologyException *);

	/** 
	 * For nodes which have edges from only one Geometry incident on them,
	 * the previous step will have left their dirEdges with no
	 * labelling for the other Geometry. 
	 * However, the sym dirEdge may have a labelling for the other
	 * Geometry, so merge the two labels.
	 */
	void mergeSymLabels();

	void updateNodeLabelling();

	/** 
	 * Incomplete nodes are nodes whose labels are incomplete.
	 *
	 * (e.g. the location for one Geometry is NULL).
	 * These are either isolated nodes,
	 * or nodes which have edges from only a single Geometry incident
	 * on them.
	 *
	 * Isolated nodes are found because nodes in one graph which
	 * don't intersect nodes in the other are not completely
	 * labelled by the initial process of adding nodes to the nodeList.
	 * To complete the labelling we need to check for nodes that
	 * lie in the interior of edges, and in the interior of areas.
	 * 
	 * When each node labelling is completed, the labelling of the
	 * incident edges is updated, to complete their labelling as well.
	 */
	void labelIncompleteNodes();

	/** \brief
	 * Label an isolated node with its relationship to the target geometry.
	 */
	void labelIncompleteNode(geomgraph::Node *n, int targetIndex);

	/** \brief
	 * Find all edges whose label indicates that they are in the result
	 * area(s), according to the operation being performed. 
	 *
	 * Since we want polygon shells to be
	 * oriented CW, choose dirEdges with the interior of the result
	 * on the RHS.
	 * Mark them as being in the result.
	 * Interior Area edges are the result of dimensional collapses.
	 * They do not form part of the result area boundary.
	 */
	void findResultAreaEdges(OpCode opCode);

	/**
	 * If both a dirEdge and its sym are marked as being in the result,
	 * cancel them out.
	 */
	void cancelDuplicateResultEdges();

	/**
	 * @return true if the coord is located in the interior or boundary of
	 * a geometry in the list.
	 */
	bool isCovered(const geom::Coordinate& coord,
			std::vector<geom::Geometry*> *geomList);

	/**
	 * @return true if the coord is located in the interior or boundary of
	 * a geometry in the list.
	 */
	bool isCovered(const geom::Coordinate& coord,
			std::vector<geom::Polygon*> *geomList);

	/**
	 * @return true if the coord is located in the interior or boundary of
	 * a geometry in the list.
	 */
	bool isCovered(const geom::Coordinate& coord,
			std::vector<geom::LineString*> *geomList);

	/**
	 * Build a Geometry containing all Geometries in the given vectors.
	 * Takes element's ownership, vector control is left to caller. 
	 */
	geom::Geometry* computeGeometry(
			std::vector<geom::Point*> *nResultPointList,
			std::vector<geom::LineString*> *nResultLineList,
			std::vector<geom::Polygon*> *nResultPolyList);

	/// Caches for memory management 
	std::vector<geomgraph::Edge *>dupEdges;

	/** \brief
	 * Merge Z values of node with those of the segment or vertex in
	 * the given Polygon it is on.
	 */
	int mergeZ(geomgraph::Node *n, const geom::Polygon *poly) const;

	/**
	 * Merge Z values of node with those of the segment or vertex in
	 * the given LineString it is on.
	 * @returns 1 if an intersection is found, 0 otherwise.
	 */
	int mergeZ(geomgraph::Node *n, const geom::LineString *line) const;

	/**
	 * Average Z of input geometries
	 */
	double avgz[2];
	bool avgzcomputed[2];

	double getAverageZ(int targetIndex);
	static double getAverageZ(const geom::Polygon *poly);

	ElevationMatrix *elevationMatrix;

	/// Throw TopologyException if an obviously wrong result has
	/// been computed.
	void checkObviouslyWrongResult(OpCode opCode);

};

/** \brief
 * OverlayOp::overlayOp Adapter for use with geom::BinaryOp
 */
struct overlayOp {

        OverlayOp::OpCode opCode;

        overlayOp(OverlayOp::OpCode code)
                :
                opCode(code)
        {}

        geom::Geometry* operator() (const geom::Geometry* g0,
                                    const geom::Geometry* g1)
        {
                return OverlayOp::overlayOp(g0, g1, opCode);
        }

};

} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // ndef GEOS_OP_OVERLAY_OVERLAYOP_H

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/07/05 20:19:29  strk
 * added checks for obviously wrong result of difference and intersection ops
 *
 * Revision 1.5  2006/06/05 15:36:34  strk
 * Given OverlayOp funx code enum a name and renamed values to have a lowercase prefix. Drop all of noding headers from installed header set.
 *
 * Revision 1.4  2006/05/24 15:17:44  strk
 * Reduced number of installed headers in geos/operation/ subdir
 *
 * Revision 1.3  2006/04/14 15:04:36  strk
 * fixed missing namespace qualification in overlay::overlayOp
 *
 * Revision 1.2  2006/04/14 14:35:47  strk
 * Added overlayOp() adapter for use in templates expecting binary ops
 *
 * Revision 1.1  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 **********************************************************************/

