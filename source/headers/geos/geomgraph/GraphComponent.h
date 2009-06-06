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
 * Last port: geomgraph/GraphComponent.java rev. 1.3 (JTS-1.10)
 *
 * EXPOSED GEOS HEADER
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_GRAPHCOMPONENT_H
#define GEOS_GEOMGRAPH_GRAPHCOMPONENT_H

#include <geos/export.h>
#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom {
		class IntersectionMatrix;
	}
	namespace geomgraph {
		class Label;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph


class GEOS_DLL GraphComponent {
public:
	GraphComponent();

	/*
	 * GraphComponent takes ownership of the given Label.
	 * newLabel is deleted by destructor.
	 */
	GraphComponent(Label* newLabel); 
	virtual ~GraphComponent();
	Label* getLabel();
	virtual void setLabel(Label* newLabel);
	virtual void setInResult(bool isInResult) { isInResultVar=isInResult; }
	virtual bool isInResult() const { return isInResultVar; }
	virtual void setCovered(bool isCovered);
	virtual bool isCovered() const { return isCoveredVar; }
	virtual bool isCoveredSet() const { return isCoveredSetVar; }
	virtual bool isVisited() const { return isVisitedVar; }
	virtual void setVisited(bool isVisited) { isVisitedVar = isVisited; }
	virtual bool isIsolated() const=0;
	virtual void updateIM(geom::IntersectionMatrix *im);
protected:
	Label* label;
	virtual void computeIM(geom::IntersectionMatrix *im)=0;
private:
	bool isInResultVar;
	bool isCoveredVar;
	bool isCoveredSetVar;
	bool isVisitedVar;
};

} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/GraphComponent.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_GRAPHCOMPONENT_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

