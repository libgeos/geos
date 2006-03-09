/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOM_DIMENSION_H
#define GEOS_GEOM_DIMENSION_H

#include <geos/inline.h>

namespace geos {
namespace geom { // geos::geom

/// Constants representing the dimensions of a point, a curve and a surface.
//
/// Also, constants representing the dimensions of the empty geometry and
/// non-empty geometries, and a wildcard dimension meaning "any dimension".
///
class Dimension {
public:
	enum {
		DONTCARE=-3,	/// Dimension value for any dimension (= {FALSE, TRUE}).
		True,			/// Dimension value of non-empty geometries (= {P, L, A}).
		False,			/// Dimension value of the empty geometry (-1).
		P,				/// Dimension value of a point (0).
		L,				/// Dimension value of a curve (1).
		A				/// Dimension value of a surface (2).
	};
	//static const int P = 0;			/// Dimension value of a point (0).
	//static const int L = 1;			/// Dimension value of a curve (1).
	//static const int A = 2;			/// Dimension value of a surface (2).
	//static const int False = -1;	/// Dimension value of the empty geometry (-1).
	//static const int True = -2;		/// Dimension value of non-empty geometries (= {P, L, A}).
	//static const int DONTCARE = -3;	/// Dimension value for any dimension (= {FALSE, TRUE}).
	static char toDimensionSymbol(int dimensionValue);
	static int toDimensionValue(char dimensionSymbol);
};

} // namespace geos::geom
} // namespace geos

#ifdef USE_INLINE
# include "geos/geom/Envelope.inl"
#endif

#endif // ndef GEOS_GEOM_DIMENSION_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
