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
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../../headers/indexChain.h"
#include "stdio.h"

namespace geos {

MonotoneChainOverlapAction::MonotoneChainOverlapAction() {
	seg1=new LineSegment();
	seg2=new LineSegment();
}

MonotoneChainOverlapAction::~MonotoneChainOverlapAction() {
	delete seg1;
	delete seg2;
}


/**
 * This function can be overridden if the original chains are needed
 */
void MonotoneChainOverlapAction::overlap(indexMonotoneChain *mc1,int start1,indexMonotoneChain *mc2,int start2) {
	mc1->getLineSegment(start1,seg1);
	mc2->getLineSegment(start2,seg2);
	overlap(seg1,seg2);
}
}

