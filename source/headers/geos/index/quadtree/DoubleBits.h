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

#ifndef GEOS_IDX_QUADTREE_DOUBLEBITS_H
#define GEOS_IDX_QUADTREE_DOUBLEBITS_H

#include <geos/platform.h> // for int64

#include <string>

namespace geos {
namespace index { // geos::index
namespace quadtree { // geos::index::quadtree

class DoubleBits {
public:
	static const int EXPONENT_BIAS=1023;
	static double powerOf2(int exp);
	static int exponent(double d);
	static double truncateToPowerOfTwo(double d);
	static std::string toBinaryString(double d);
	static double maximumCommonMantissa(double d1, double d2);
	DoubleBits(double nx);
	double getDouble();
	int64 biasedExponent();
	int getExponent();
	void zeroLowerBits(int nBits);
	int getBit(int i);
	int numCommonMantissaBits(DoubleBits *db);
	std::string toString();
private:
	double x;
//	long long xBits;
//	long xBits;
	int64 xBits;
};

} // namespace geos::index::quadtree
} // namespace geos::index
} // namespace geos

#endif // GEOS_IDX_QUADTREE_DOUBLEBITS_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 **********************************************************************/

