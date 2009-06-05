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
 **********************************************************************
 *
 * Last port: geomgraph/EdgeEndStar.java rev. 1.4 (JTS-1.7)
 *
 * EXPOSED GEOS HEADER
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_EDGEENDSTAR_H
#define GEOS_GEOMGRAPH_EDGEENDSTAR_H

#include <geos/geomgraph/EdgeEnd.h>  // for EdgeEndLT
#include <geos/geom/Coordinate.h>  // for p0,p1

#include <geos/inline.h>

#include <set>
#include <string>
#include <vector>
#include <algorithm> // for inlines (find)

// Forward declarations
namespace geos {
	namespace geomgraph {
		class GeometryGraph;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph


/** \brief
 * A EdgeEndStar is an ordered list of EdgeEnds around a node.
 *
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

	/** \brief
	 * Insert a EdgeEnd into this EdgeEndStar
	 */
	virtual void insert(EdgeEnd *e)=0;

	/** \brief
	 * @return the coordinate for the node this star is based at
	 *         or NULL if this is still an unbound star.
	 * Be aware that the returned pointer will point to
	 * a Coordinate owned by the specific EdgeEnd happening
	 * to be the first in the star (ordered CCW)
	 */
	virtual geom::Coordinate& getCoordinate();

	virtual size_t getDegree();

	virtual iterator begin();

	virtual iterator end();

	virtual reverse_iterator rbegin();

	virtual reverse_iterator rend();

	virtual container &getEdges();


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
	virtual iterator find(EdgeEnd *eSearch);

	virtual std::string print();

protected:

	/** \brief
	 * A map which maintains the edges in sorted order
	 * around the node
	 */
	EdgeEndStar::container edgeMap;

	/** \brief
	 * Insert an EdgeEnd into the map.
	 */
	virtual void insertEdgeEnd(EdgeEnd *e) { edgeMap.insert(e); }

private:

	/** \brief
	 * The location of the point for this star in
	 * Geometry i Areas
	 */
	int ptInAreaLocation[2];

	virtual void computeEdgeEndLabels();

	virtual bool checkAreaLabelsConsistent(int geomIndex);

};

inline size_t
EdgeEndStar::getDegree()
{
	return edgeMap.size();
}

inline EdgeEndStar::iterator
EdgeEndStar::begin()
{
	return edgeMap.begin();
}

inline EdgeEndStar::container&
EdgeEndStar::getEdges()
{
	return edgeMap;
}

inline EdgeEndStar::reverse_iterator
EdgeEndStar::rend()
{
	return edgeMap.rend();
}

inline EdgeEndStar::iterator
EdgeEndStar::end()
{
	return edgeMap.end();
}

inline EdgeEndStar::reverse_iterator
EdgeEndStar::rbegin()
{
	return edgeMap.rbegin();
}

inline EdgeEndStar::iterator
EdgeEndStar::find(EdgeEnd *eSearch)
{
	return edgeMap.find(eSearch);
}


} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/EdgeEndStar.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGEENDSTAR_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/06/12 10:49:43  strk
 * unsigned int => size_t
 *
 * Revision 1.3  2006/04/04 13:35:55  strk
 * Port info, assertion checking, indentation
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

