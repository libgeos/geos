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
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 *
 **********************************************************************/


#include "../../headers/opLinemerge.h"

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
