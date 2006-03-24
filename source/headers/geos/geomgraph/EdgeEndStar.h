/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_EDGEENDSTAR_H
#define GEOS_GEOMGRAPH_EDGEENDSTAR_H

#include <set>
#include <string>
#include <vector>
#include <algorithm> // for inlines (find)

#include <geos/geomgraph/EdgeEnd.h>  // for EdgeEndLT
#include <geos/geom/Coordinate.h>  // for p0,p1

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geomgraph {
		class GeometryGraph;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph


/**
 * A EdgeEndStar is an ordered list of EdgeEnds around a node.
 * They are maintained in CCW order (starting with the positive x-axis)
 * around the node for efficient lookup and topology building.
 *
 * @version 1.4
 */
class EdgeEndStar {
public:

	typedef std::set<EdgeEnd *, EdgeEndLT> container;

	typedef container::iterator iterator;
	typedef container::reverse_iterator reverse_iterator;

	EdgeEndStar();

	virtual ~EdgeEndStar() {};

	virtual void insert(EdgeEnd *e)=0;

	virtual geom::Coordinate& getCoordinate();

	virtual unsigned int getDegree() { return edgeMap.size(); }

	virtual iterator begin() { return edgeMap.begin(); }
	virtual iterator end() { return edgeMap.end(); }

	virtual reverse_iterator rbegin() { return edgeMap.rbegin(); }
	virtual reverse_iterator rend() { return edgeMap.rend(); }

	virtual container &getEdges() { return edgeMap; }

	virtual EdgeEnd* getNextCW(EdgeEnd *ee);

	virtual void computeLabelling(std::vector<GeometryGraph*> *geom);
		// throw(TopologyException *);

	virtual int getLocation(int geomIndex,
		const geom::Coordinate& p,
		std::vector<GeometryGraph*> *geom); 

	virtual bool isAreaLabelsConsistent();

	virtual void propagateSideLabels(int geomIndex);
		// throw(TopologyException *);

	//virtual int findIndex(EdgeEnd *eSearch);
	virtual iterator find(EdgeEnd *eSearch) {
		return edgeMap.find(eSearch);
	}

	virtual std::string print();

protected:

	EdgeEndStar::container edgeMap;

	/**
	 * Insert an EdgeEnd into the map.
	 */
	virtual void insertEdgeEnd(EdgeEnd *e) { edgeMap.insert(e); }

private:

	int ptInAreaLocation[2];

	virtual void computeEdgeEndLabels();

	virtual bool checkAreaLabelsConsistent(int geomIndex);

};

} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/EdgeEndStar.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGEENDSTAR_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

