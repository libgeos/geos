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
 * Revision 1.2  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 *
 **********************************************************************/


#include <geos/opLinemerge.h>

namespace geos {

/**
* Constructs a LineMergeEdge with vertices given by the specified LineString.
*/
LineMergeEdge::LineMergeEdge(LineString *newLine) {
	line=newLine;
}
/**
* Returns the LineString specifying the vertices of this edge.
*/
LineString* LineMergeEdge::getLine() {
	return line;
}
}
