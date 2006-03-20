/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/indexQuadtree.h>
#include <geos/util.h>
#include <string>
#include <vector>

#if ! ASSUME_IEEE_DOUBLE
#include <cmath>
#endif

namespace geos {
namespace index { // geos.index
namespace quadtree { // geos.index.quadtree

using namespace std;

double
DoubleBits::powerOf2(int exp)
{
	if (exp>1023 || exp<-1022)
		throw util::IllegalArgumentException("Exponent out of bounds");
#if ASSUME_IEEE_DOUBLE
	int64 expBias=exp+EXPONENT_BIAS;
	int64 bits=expBias << 52;
	double ret;
	memcpy(&ret, &bits, sizeof(int64));
	return ret;
#else
	return pow(2.0, exp);
#endif
}

int
DoubleBits::exponent(double d)
{
	DoubleBits db(d);
	return db.getExponent();
}

double
DoubleBits::truncateToPowerOfTwo(double d)
{
	DoubleBits db(d);
	db.zeroLowerBits(52);
	return db.getDouble();
}

string DoubleBits::toBinaryString(double d) {
	DoubleBits db(d);
	return db.toString();
}

double
DoubleBits::maximumCommonMantissa(double d1, double d2)
{
	if (d1==0.0 || d2==0.0) return 0.0;
	DoubleBits db1(d1);
	DoubleBits db2(d2);
	if (db1.getExponent() != db2.getExponent()) return 0.0;
	int maxCommon=db1.numCommonMantissaBits(&db2);
	db1.zeroLowerBits(64-(12+maxCommon));
	return db1.getDouble();
}

DoubleBits::DoubleBits(double nx)
{
#if ASSUME_IEEE_DOUBLE
	memcpy(&xBits,&nx,sizeof(double));
#endif
	x = nx;
}

double DoubleBits::getDouble()
{
	return x;
}

/**
 * Determines the exponent for the number
 *
 * @return
 */
int64
DoubleBits::biasedExponent()
{
	int64 signExp=xBits>>52;
	int64 exp=signExp&0x07ff;
	//cerr<<"xBits:"<<xBits<<" signExp:"<<signExp<<" exp:"<<exp<<endl;
	return exp;
}

/*
 * Determines the exponent for the number
 *
 * @return
 */
int
DoubleBits::getExponent()
{
#if ASSUME_IEEE_DOUBLE
	return biasedExponent()-EXPONENT_BIAS;
#else
	return (int)((log(x)/log(2.0))+(x<1?-0.9:0.00000000001));
#endif
}

void
DoubleBits::zeroLowerBits(int nBits)
{
	long invMask=(1L<<nBits)-1L;
	long mask=~invMask;
	xBits&=mask;
}

int
DoubleBits::getBit(int i)
{
	long mask=(1L<<i);
	return (xBits&mask)!=0?1:0;
}

/**
 * This computes the number of common most-significant bits in the mantissa.
 * It does not count the hidden bit, which is always 1.
 * It does not determine whether the numbers have the same exponent - if they do
 * not, the value computed by this function is meaningless.
 * @param db
 * @return the number of common most-significant mantissa bits
 */
int DoubleBits::numCommonMantissaBits(DoubleBits *db) {
	for (int i=0;i<52;i++) {
		//int bitIndex=i+12;
		if (getBit(i)!=db->getBit(i))
			return i;
	}
	return 52;
}

/**
 * A representation of the Double bits formatted for easy readability
 * @return
 */
string DoubleBits::toString() {
	//String numStr = Long.toBinaryString(xBits);
//// 64 zeroes!
//String zero64 = "0000000000000000000000000000000000000000000000000000000000000000";
//String padStr =  zero64 + numStr;
//String bitStr = padStr.substring(padStr.length() - 64);
//String str = bitStr.substring(0, 1) + "  "
//+ bitStr.substring(1, 12) + "(" + getExponent() + ") "
//+ bitStr.substring(12)
//+ " [ " + x + " ]";
//return str;
	return "";
}

} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.22  2006/03/20 17:40:38  strk
 * <cmath> included when appropriate
 *
 * Revision 1.21  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.20  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.19  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.18  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.17  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.16  2004/11/04 08:49:13  strk
 * Unlinked new documentation.
 *
 * Revision 1.15  2004/11/03 08:22:25  strk
 * Slightly modified log/log2 based algo to better handle numbers in the
 * range 0-1.
 *
 * Revision 1.14  2004/11/02 15:49:59  strk
 *
 * Moved ASSUME_IEEE_DOUBLE define from DoubleBits.cpp to indexQuadtree.h.
 * Fixed a bug in powerOf2(). Made the !IEEE version less prone to
 * round-offs (still has approximation errors).
 *
 * Revision 1.13  2004/11/02 14:13:38  strk
 * Fixed bug in IEEE-based exponent and PowerOf2 computation, but disabled
 * at compile time.
 *
 * Revision 1.12  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.11  2004/09/21 09:47:01  strk
 * Removed useless auto_ptr usage in ::exponent
 *
 * Revision 1.10  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

