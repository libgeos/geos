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
 * Revision 1.7  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.6  2004/03/25 02:23:55  ybychkov
 * All "index/*" packages upgraded to JTS 1.4
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/indexChain.h>
#include <stdio.h>

namespace geos {

MonotoneChainSelectAction::MonotoneChainSelectAction() {
	selectedSegment=new LineSegment();
	tempEnv1=new Envelope();
}

MonotoneChainSelectAction::~MonotoneChainSelectAction() {
	delete selectedSegment;
	delete tempEnv1;
}

/**
* This function can be overridden if the original chain is needed
*/
void MonotoneChainSelectAction::select(indexMonotoneChain *mc,int start) {
	mc->getLineSegment(start,selectedSegment);
	select(selectedSegment);
}
}

