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
 * Revision 1.6  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/indexQuadtree.h>

namespace geos {

bool IntervalSize::isZeroWidth(double mn,double mx){
	double width=mx-mn;
	if (width==0.0) return true;
	double maxAbs=max(fabs(mn),fabs(mx));
	double scaledInterval=width/maxAbs;
	int level=DoubleBits::exponent(scaledInterval);
	return level<=MIN_BINARY_EXPONENT;
}
}

