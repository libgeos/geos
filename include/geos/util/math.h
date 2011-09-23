/**********************************************************************
 * $Id: math.h 2854 2010-01-16 20:12:23Z mloskot $
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

namespace geos { namespace util {

/// Symmetric Rounding Algorithm
double sym_round(double val);

/// Asymmetric Rounding Algorithm
double java_math_round(double val);

/// Equivalent to Java Math.rint()
double rint_vc(double val);

/// Default rounding method for GEOS
///
/// @note Always use this rounding method, to easy easy switching
/// between different rounding method for the whole codebase.
inline double round(double val)
{
	return java_math_round(val);
}

}} // namespace geos::util

#endif // GEOS_UTIL_MATH_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/05/04 14:05:31  strk
 * * source/headers/geos/util/math.h: provided an util::round() method being an inline proxy to call appropriate default rounding function for the whole GEOS codebase. Currently pointing at util::java_math_round() being the last being used.
 *
 * Revision 1.3  2006/04/07 12:37:53  mloskot
 * Added java_math_round() function for Asymmetric Arithmetic Rounding. Small fixes in Unit Tests.
 *
 * Revision 1.2  2006/04/06 21:30:36  strk
 * Added missing rint_vc() forward declaration
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
