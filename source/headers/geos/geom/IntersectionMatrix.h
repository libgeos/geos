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

#ifndef GEOS_GEOM_INTERSECTIONMATRIX_H
#define GEOS_GEOM_INTERSECTIONMATRIX_H

#include <string>

#include <geos/inline.h>

namespace geos {
namespace geom { // geos::geom

class IntersectionMatrix {
public:
	IntersectionMatrix();
	IntersectionMatrix(const std::string& elements);
	IntersectionMatrix(const IntersectionMatrix &im);
	static bool matches(int actualDimensionValue, char requiredDimensionSymbol);
	static bool matches(const std::string& actualDimensionSymbols, const std::string& requiredDimensionSymbols);
	void add(IntersectionMatrix *im);
	void set(int row, int column, int dimensionValue);
	void set(const std::string& dimensionSymbols);
	void setAtLeast(int row, int column, int minimumDimensionValue);
	void setAtLeastIfValid(int row, int column, int minimumDimensionValue);
	void setAtLeast(std::string minimumDimensionSymbols);
	void setAll(int dimensionValue);
	int get(int row, int column);
	bool isDisjoint();
	bool isIntersects();
	bool isTouches(int dimensionOfGeometryA, int dimensionOfGeometryB);
	bool isCrosses(int dimensionOfGeometryA, int dimensionOfGeometryB);
	bool isWithin();
	bool isContains();
	bool isEquals(int dimensionOfGeometryA, int dimensionOfGeometryB);
	bool isOverlaps(int dimensionOfGeometryA, int dimensionOfGeometryB);
	bool matches(const std::string& requiredDimensionSymbols);
	IntersectionMatrix* transpose();
	std::string toString();
private:
	int matrix[3][3];
};


} // namespace geos::geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/IntersectionMatrix.inl"
//#endif

#endif // ndef GEOS_GEOM_INTERSECTIONMATRIX_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
