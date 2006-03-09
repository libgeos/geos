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


#ifndef GEOS_GEOMGRAPH_LABEL_H
#define GEOS_GEOMGRAPH_LABEL_H

#include <geos/geomgraph/TopologyLocation.h> 

#include <geos/inline.h>

namespace geos {
namespace geomgraph { // geos.geomgraph

class Label {
public:
	static Label* toLineLabel(const Label& label);
	Label(int onLoc);
	Label(int geomIndex, int onLoc);
	Label(int onLoc, int leftLoc, int rightLoc);
	Label(const Label &l);
	Label();
	virtual ~Label();
	Label(int geomIndex,int onLoc,int leftLoc,int rightLoc);
	void flip();
	int getLocation(int geomIndex, int posIndex) const;
	int getLocation(int geomIndex) const;
	void setLocation(int geomIndex, int posIndex, int location);
	void setLocation(int geomIndex, int location);
	void setAllLocations(int geomIndex, int location);
	void setAllLocationsIfNull(int geomIndex, int location);
	void setAllLocationsIfNull(int location);
	void merge(const Label &lbl);
	int getGeometryCount() const;
	bool isNull(int geomIndex) const;
	bool isAnyNull(int geomIndex) const;
	bool isArea() const;
	bool isArea(int geomIndex) const;
	bool isLine(int geomIndex) const;
	bool isEqualOnSide(const Label &lbl, int side) const;
	bool allPositionsEqual(int geomIndex, int loc) const;
	void toLine(int geomIndex);
	std::string toString() const;
protected:
	TopologyLocation elt[2];
};

} // namespace geos.geomgraph
} // namespace geos


//#ifdef USE_INLINE
//# include "geos/geomgraph/Label.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_LABEL_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

