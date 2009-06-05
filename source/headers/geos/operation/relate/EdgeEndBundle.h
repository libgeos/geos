/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/relate/EdgeEndBundle.java rev. 1.15 (JTS-1.7)
 *
 * NON-EXPOSED GEOS HEADER
 *
 **********************************************************************/

#ifndef GEOS_OP_RELATE_EDGEENDBUNDLE_H
#define GEOS_OP_RELATE_EDGEENDBUNDLE_H

#include <geos/geomgraph/EdgeEnd.h> // for EdgeEndBundle inheritance

#include <string>

// Forward declarations
namespace geos {
	namespace geom {
		class IntersectionMatrix;
	}
}


namespace geos {
namespace operation { // geos::operation
namespace relate { // geos::operation::relate

/** \brief
 * Contains all geomgraph::EdgeEnd objectss which start at the same point
 * and are parallel.
 */
class EdgeEndBundle: public geomgraph::EdgeEnd {
public:
	EdgeEndBundle(geomgraph::EdgeEnd *e);
	virtual ~EdgeEndBundle();
	geomgraph::Label *getLabel();
//Iterator iterator() //Not needed
	std::vector<geomgraph::EdgeEnd*>* getEdgeEnds();
	void insert(geomgraph::EdgeEnd *e);
	void computeLabel() ; 
	void updateIM(geom::IntersectionMatrix *im);
	std::string print();
protected:
	std::vector<geomgraph::EdgeEnd*> *edgeEnds;
	void computeLabelOn(int geomIndex);
	void computeLabelSides(int geomIndex);
	void computeLabelSide(int geomIndex,int side);
};

} // namespace geos:operation:relate
} // namespace geos:operation
} // namespace geos

#endif // GEOS_OP_RELATE_EDGEENDBUNDLE_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 13:11:29  strk
 * opRelate.h header split
 *
 **********************************************************************/

