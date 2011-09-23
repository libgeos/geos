/**********************************************************************
 * $Id: CommonBits.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

#include <geos/platform.h> // for int64
#include <geos/precision/CommonBits.h>

namespace geos {
namespace precision { // geos.precision

/*static public*/
int64
CommonBits::signExpBits(int64 num)
{
	return num >> 52;
}

/*static public*/
int
CommonBits::numCommonMostSigMantissaBits(int64 num1, int64 num2)
{
	int count = 0;
	for (int i = 52; i >= 0; i--){
		if (getBit(num1, i) != getBit(num2, i))
			return count;
		count++;
	}
	return 52;
}

/*static public*/
int64
CommonBits::zeroLowerBits(int64 bits, int nBits)
{
	int64 invMask = (1<< nBits)-1;
	int64 mask = ~ invMask;
	int64 zeroed = bits & mask;
	return zeroed;
}

/*static public*/
int
CommonBits::getBit(int64 bits, int i)
{
	int64 mask = (1 << i);
	return (bits & mask) != 0 ? 1 : 0;
}

/*public*/
CommonBits::CommonBits() {
	isFirst = true;
	commonMantissaBitsCount = 53;
	commonBits = 0;
}

/*public*/
void
CommonBits::add(double num)
{
	int64 numBits=(int64)num;
	if (isFirst) {
		commonBits = numBits;
		commonSignExp = signExpBits(commonBits);
		isFirst = false;
		return;
	}
	int64 numSignExp = signExpBits(numBits);
	if (numSignExp != commonSignExp) {
		commonBits = 0;
		return;
	}
	//    System.out.println(toString(commonBits));
	//    System.out.println(toString(numBits));
	commonMantissaBitsCount = numCommonMostSigMantissaBits(commonBits, numBits);
	commonBits = zeroLowerBits(commonBits, 64 - (12 + commonMantissaBitsCount));
	//    System.out.println(toString(commonBits));
}

/*public*/
double
CommonBits::getCommon()
{
	return (double)commonBits;
}

///**
//* A representation of the Double bits formatted for easy readability
//*/
//String toString(int64 bits)
//{
//double x = Double.int64BitsToDouble(bits);
//String numStr = int64.toBinaryString(bits);
//String padStr = "0000000000000000000000000000000000000000000000000000000000000000" + numStr;
//String bitStr = padStr.substring(padStr.length() - 64);
//String str = bitStr.substring(0, 1) + "  "
//+ bitStr.substring(1, 12) + "(exp) "
//+ bitStr.substring(12)
//+ " [ " + x + " ]";
//return str;
//}

} // namespace geos.precision
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/03/23 09:17:19  strk
 * precision.h header split, minor optimizations
 *
 **********************************************************************/

