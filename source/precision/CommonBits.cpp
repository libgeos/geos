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
 * Revision 1.2  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/10 22:41:25  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include <geos/precision.h>

namespace geos {
/**
* Computes the bit pattern for the sign and exponent of a
* double-precision number.
* @param num
* @return the bit pattern for the sign and exponent
*/
int64 CommonBits::signExpBits(int64 num){
	return num >> 52;
}

/**
* This computes the number of common most-significant bits in the mantissas
* of two double-precision numbers.
* It does not count the hidden bit, which is always 1.
* It does not determine whether the numbers have the same exponent - if they do
* not, the value computed by this function is meaningless.
* @param db
* @return the number of common most-significant mantissa bits
*/
int CommonBits::numCommonMostSigMantissaBits(int64 num1, int64 num2) {
	int count = 0;
	for (int i = 52; i >= 0; i--){
		if (getBit(num1, i) != getBit(num2, i))
			return count;
		count++;
	}
	return 52;
}

/**
* Zeroes the lower n bits of a bitstring.
* @param bits the bitstring to alter
* @param i the number of bits to zero
* @return the zeroed bitstring
*/
int64 CommonBits::zeroLowerBits(int64 bits, int nBits){
	int64 invMask = (1<< nBits)-1;
	int64 mask = ~ invMask;
	int64 zeroed = bits & mask;
	return zeroed;
}

/**
* Extracts the i'th bit of a bitstring.
* @param bits the bitstring to extract from
* @param i the bit to extract
* @return the value of the extracted bit
*/
int CommonBits::getBit(int64 bits, int i){
	int64 mask = (1 << i);
	return (bits & mask) != 0 ? 1 : 0;
}

CommonBits::CommonBits() {
	isFirst = true;
	commonMantissaBitsCount = 53;
	commonBits = 0;
}

void CommonBits::add(double num) {
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

double CommonBits::getCommon(){
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

}
