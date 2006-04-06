/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_UTIL_MATH_H
#define GEOS_UTIL_MATH_H

namespace geos {
namespace util { // geos::util

/// Symmetric Rounding Algorithm
double sym_round(double val);

/// Equivalent to Java Math.rint()
double rint_vc(double val);

} // namespace geos.util
} // namespace geos


#endif // GEOS_UTIL_MATH_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/04/06 21:30:36  strk
 * Added missing rint_vc() forward declaration
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
