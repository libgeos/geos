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
 **********************************************************************/

#include <geos/indexChain.h>
#include <stdio.h>

namespace geos {

MonotoneChainOverlapAction::MonotoneChainOverlapAction() {
	overlapSeg1=new LineSegment();
	overlapSeg2=new LineSegment();
	tempEnv1=new Envelope();
	tempEnv2=new Envelope();

}

MonotoneChainOverlapAction::~MonotoneChainOverlapAction() {
	delete overlapSeg1;
	delete overlapSeg2;
	delete tempEnv1;
	delete tempEnv2;
}


/**
 * This function can be overridden if the original chains are needed
 */
void MonotoneChainOverlapAction::overlap(indexMonotoneChain *mc1,int start1,indexMonotoneChain *mc2,int start2) {
	mc1->getLineSegment(start1,overlapSeg1);
	mc2->getLineSegment(start2,overlapSeg2);
	overlap(overlapSeg1,overlapSeg2);
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.8  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.7  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.6  2004/03/25 02:23:55  ybychkov
 * All "index/" packages upgraded to JTS 1.4
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

