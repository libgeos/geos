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

#ifndef GEOS_ALGORITHM_ROBUSTDETERMINANT_H
#define GEOS_ALGORITHM_ROBUSTDETERMINANT_H


namespace geos {
namespace algorithm { // geos::algorithm

class RobustDeterminant {
public:
	static int signOfDet2x2(double x1,double y1,double x2,double y2);
};

} // namespace geos::algorithm
} // namespace geos



#endif // GEOS_ALGORITHM_ROBUSTDETERMINANT_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

