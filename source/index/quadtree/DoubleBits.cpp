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

#include <geos/indexQuadtree.h>
#include <geos/util.h>

namespace geos {

double
DoubleBits::powerOf2(int exp)
{
	if (exp>1023 || exp<-1022)
		throw new IllegalArgumentException("Exponent out of bounds");
#if 0
	long expBias=exp+EXPONENT_BIAS;
	int64 bits=(int64)expBias << 52;
	return (double) bits;
#else
	return pow(2.0, exp);
#endif
}

int
DoubleBits::exponent(double d)
{
	//DoubleBits db(d);
	//return db.getExponent();
	return (int)log(d)/log(2.0);
}

double
DoubleBits::truncateToPowerOfTwo(double d)
{
	auto_ptr<DoubleBits> db(new DoubleBits(d));
	db->zeroLowerBits(52);
	return db->getDouble();
}

string DoubleBits::toBinaryString(double d) {
	auto_ptr<DoubleBits> db(new DoubleBits(d));
	return db->toString();
}

double
DoubleBits::maximumCommonMantissa(double d1, double d2)
{
	if (d1==0.0 || d2==0.0) return 0.0;
	auto_ptr<DoubleBits> db1(new DoubleBits(d1));
	DoubleBits *db2=new DoubleBits(d2);
	if (db1->getExponent()!=db2->getExponent()) return 0.0;
	int maxCommon=db1->numCommonMantissaBits(db2);
	delete db2;
	db1->zeroLowerBits(64-(12+maxCommon));
	return db1->getDouble();
}

DoubleBits::DoubleBits(double nx)
{
	xBits=(int64)nx;
}

double DoubleBits::getDouble()
{
	return (double)xBits;
}

/**
 * Determines the exponent for the number
 *
 * @return
 */
int64
DoubleBits::biasedExponent()
{
	int64 signExp=(int64)(xBits>>52);
	int64 exp=signExp&0x07ff;
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
	return biasedExponent()-EXPONENT_BIAS;
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
		int bitIndex=i+12;
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

} // namespace geos

/**********************************************************************
 * $Log$
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

