/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.9  2004/12/13 13:53:03  strk
 * Fixed uninitialized Coordinate.
 *
 * Revision 1.8  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geom.h>
#include <geos/util.h>
#include <stdio.h>

namespace geos {

TopologyException::TopologyException(string msg): GEOSException("TopologyException", msg) {
	pt=NULL;
}

TopologyException::TopologyException(string msg,const Coordinate *newPt): GEOSException("TopologyException", msg+" "+newPt->toString()) {
	if ( newPt ) pt=new Coordinate(*newPt);
	else pt=NULL;
}

TopologyException::~TopologyException() {
	delete pt;
}

Coordinate* TopologyException::getCoordinate() {
	return pt;
}


}

