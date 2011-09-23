/**********************************************************************
 * $Id: IntersectionMatrix.cpp 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/IntersectionMatrix.java rev. 1.18
 *
 **********************************************************************/

#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Location.h>
#include <geos/util/IllegalArgumentException.h>

#include <sstream>
#include <cassert>

using namespace std;

namespace geos {
namespace geom { // geos::geom

const int IntersectionMatrix::firstDim = 3;
const int IntersectionMatrix::secondDim = 3;
	
/*public*/
IntersectionMatrix::IntersectionMatrix()
{
	//matrix = new int[3][3];
	setAll(Dimension::False);
}

/*public*/
IntersectionMatrix::IntersectionMatrix(const string& elements)
{
	setAll(Dimension::False);
	set(elements);
}

/*public*/
IntersectionMatrix::IntersectionMatrix(const IntersectionMatrix& other)
{
	matrix[Location::INTERIOR][Location::INTERIOR] = other.matrix[Location::INTERIOR][Location::INTERIOR];
	matrix[Location::INTERIOR][Location::BOUNDARY] = other.matrix[Location::INTERIOR][Location::BOUNDARY];
	matrix[Location::INTERIOR][Location::EXTERIOR] = other.matrix[Location::INTERIOR][Location::EXTERIOR];
	matrix[Location::BOUNDARY][Location::INTERIOR] = other.matrix[Location::BOUNDARY][Location::INTERIOR];
	matrix[Location::BOUNDARY][Location::BOUNDARY] = other.matrix[Location::BOUNDARY][Location::BOUNDARY];
	matrix[Location::BOUNDARY][Location::EXTERIOR] = other.matrix[Location::BOUNDARY][Location::EXTERIOR];
	matrix[Location::EXTERIOR][Location::INTERIOR] = other.matrix[Location::EXTERIOR][Location::INTERIOR];
	matrix[Location::EXTERIOR][Location::BOUNDARY] = other.matrix[Location::EXTERIOR][Location::BOUNDARY];
	matrix[Location::EXTERIOR][Location::EXTERIOR] = other.matrix[Location::EXTERIOR][Location::EXTERIOR];
}

/*public*/
void
IntersectionMatrix::add(IntersectionMatrix* other)
{
	for(int i = 0; i < firstDim; i++) {
		for(int j = 0; j < secondDim; j++) {
			setAtLeast(i, j, other->get(i, j));
		}
	}
}

/*public*/
bool
IntersectionMatrix::matches(const string& requiredDimensionSymbols) const
{
	if (requiredDimensionSymbols.length() != 9) {
		ostringstream s;
		s << "IllegalArgumentException: Should be length 9, is "
				<< "[" << requiredDimensionSymbols << "] instead" << endl;
		throw util::IllegalArgumentException(s.str());
	}
	for (int ai = 0; ai < firstDim; ai++) {
		for (int bi = 0; bi < secondDim; bi++) {
			if (!matches(matrix[ai][bi],requiredDimensionSymbols[3*ai+bi])) {
				return false;
			}
		}
	}
	return true;
}

/*public static*/
bool
IntersectionMatrix::matches(int actualDimensionValue,
	char requiredDimensionSymbol)
{

	if (requiredDimensionSymbol=='*') return true;

	if (requiredDimensionSymbol=='T' && (actualDimensionValue >= 0 ||
		actualDimensionValue==Dimension::True))
	{
		return true;
	}

	if (requiredDimensionSymbol=='F' &&
		actualDimensionValue==Dimension::False)
	{
		return true;
	}

	if (requiredDimensionSymbol=='0' &&
		actualDimensionValue==Dimension::P)
	{
		return true;
	}

	if (requiredDimensionSymbol=='1' &&
		actualDimensionValue==Dimension::L)
	{
		return true;
	}

	if (requiredDimensionSymbol=='2' &&
		actualDimensionValue==Dimension::A)
	{
		return true;
	}

	return false;
}

/*public static*/
bool
IntersectionMatrix::matches(const string& actualDimensionSymbols,
	const string& requiredDimensionSymbols)
{
	IntersectionMatrix m(actualDimensionSymbols);
	bool result=m.matches(requiredDimensionSymbols);
	
	return result;
}

/*public*/
void
IntersectionMatrix::set(int row, int col, int dimensionValue)
{
	assert( row >= 0 && row < firstDim );
	assert( col >= 0 && col < secondDim );
		
	matrix[row][col] = dimensionValue;
}

/*public*/
void
IntersectionMatrix::set(const string& dimensionSymbols)
{
	size_t limit = dimensionSymbols.length();

	for (size_t i = 0; i < limit; i++)
	{
		int row = i / firstDim;
		int col = i % secondDim;
		matrix[row][col] = Dimension::toDimensionValue(dimensionSymbols[i]);
	}
}

/*public*/
void
IntersectionMatrix::setAtLeast(int row, int col, int minimumDimensionValue)
{
	assert( row >= 0 && row < firstDim );
	assert( col >= 0 && col < secondDim );
	
	if (matrix[row][col] < minimumDimensionValue)
	{
		matrix[row][col] = minimumDimensionValue;
	}
}

/*public*/
void
IntersectionMatrix::setAtLeastIfValid(int row, int col, int minimumDimensionValue)
{
	assert( row >= 0 && row < firstDim );
	assert( col >= 0 && col < secondDim );
	
	if (row >= 0 && col >= 0)
	{
		setAtLeast(row, col, minimumDimensionValue);
	}
}

/*public*/
void
IntersectionMatrix::setAtLeast(string minimumDimensionSymbols)
{
	size_t limit = minimumDimensionSymbols.length();

	for (size_t i = 0; i < limit; i++)
	{
		int row = i / firstDim;
		int col = i % secondDim;
		setAtLeast(row, col, Dimension::toDimensionValue(minimumDimensionSymbols[i]));
	}
}

/*public*/
void
IntersectionMatrix::setAll(int dimensionValue)
{
	for (int ai = 0; ai < firstDim; ai++) {
		for (int bi = 0; bi < secondDim; bi++) {
			matrix[ai][bi] = dimensionValue;
		}
	}
}

/*public*/
int
IntersectionMatrix::get(int row, int col) const
{
	assert( row >= 0 && row < firstDim );
	assert( col >= 0 && col < secondDim );
	
	return matrix[row][col];
}

/*public*/
bool
IntersectionMatrix::isDisjoint() const
{
	return
		matrix[Location::INTERIOR][Location::INTERIOR]==Dimension::False
		&&
		matrix[Location::INTERIOR][Location::BOUNDARY]==Dimension::False
		&&
		matrix[Location::BOUNDARY][Location::INTERIOR]==Dimension::False
		&&
		matrix[Location::BOUNDARY][Location::BOUNDARY]==Dimension::False;
}

/*public*/
bool
IntersectionMatrix::isIntersects() const
{
	return !isDisjoint();
}

/*public*/
bool
IntersectionMatrix::isTouches(int dimensionOfGeometryA,
		int dimensionOfGeometryB) const
{
	if (dimensionOfGeometryA > dimensionOfGeometryB)
	{
		//no need to get transpose because pattern matrix is symmetrical
		return isTouches(dimensionOfGeometryB, dimensionOfGeometryA);
	}
	if ((dimensionOfGeometryA==Dimension::A && dimensionOfGeometryB==Dimension::A)
		||
		(dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::L)
		||
		(dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::A)
		||
		(dimensionOfGeometryA==Dimension::P && dimensionOfGeometryB==Dimension::A)
		||
		(dimensionOfGeometryA==Dimension::P && dimensionOfGeometryB==Dimension::L))
	{
		return matrix[Location::INTERIOR][Location::INTERIOR]==Dimension::False &&
			  (matches(matrix[Location::INTERIOR][Location::BOUNDARY], 'T') ||
			   matches(matrix[Location::BOUNDARY][Location::INTERIOR], 'T') ||
			   matches(matrix[Location::BOUNDARY][Location::BOUNDARY], 'T'));
	}
	return false;
}

/*public*/
bool
IntersectionMatrix::isCrosses(int dimensionOfGeometryA,
		int dimensionOfGeometryB) const
{
	if ((dimensionOfGeometryA==Dimension::P && dimensionOfGeometryB==Dimension::L) ||
		(dimensionOfGeometryA==Dimension::P && dimensionOfGeometryB==Dimension::A) ||
		(dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::A)) {
		return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
			   matches(matrix[Location::INTERIOR][Location::EXTERIOR], 'T');
	}
	if ((dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::P) ||
		(dimensionOfGeometryA==Dimension::A && dimensionOfGeometryB==Dimension::P) ||
		(dimensionOfGeometryA==Dimension::A && dimensionOfGeometryB==Dimension::L)) {
		return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
			   matches(matrix[Location::EXTERIOR][Location::INTERIOR], 'T');
	}
	if (dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::L) {
		return matrix[Location::INTERIOR][Location::INTERIOR]==0;
	}
	return false;
}

/*public*/
bool
IntersectionMatrix::isWithin() const
{
	return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
		   matrix[Location::INTERIOR][Location::EXTERIOR]==Dimension::False &&
		   matrix[Location::BOUNDARY][Location::EXTERIOR]==Dimension::False;
}

/*public*/
bool
IntersectionMatrix::isContains() const
{
	return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
		   matrix[Location::EXTERIOR][Location::INTERIOR]==Dimension::False &&
		   matrix[Location::EXTERIOR][Location::BOUNDARY]==Dimension::False;
}

/*public*/
bool
IntersectionMatrix::isEquals(int dimensionOfGeometryA,
		int dimensionOfGeometryB) const
{
	if (dimensionOfGeometryA != dimensionOfGeometryB) {
		return false;
	}
	return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
		   matrix[Location::EXTERIOR][Location::INTERIOR]==Dimension::False &&
		   matrix[Location::INTERIOR][Location::EXTERIOR]==Dimension::False &&
		   matrix[Location::EXTERIOR][Location::BOUNDARY]==Dimension::False &&
		   matrix[Location::BOUNDARY][Location::EXTERIOR]==Dimension::False;
}

/*public*/
bool
IntersectionMatrix::isOverlaps(int dimensionOfGeometryA,
		int dimensionOfGeometryB) const
{
	if ((dimensionOfGeometryA==Dimension::P && dimensionOfGeometryB==Dimension::P) ||
		(dimensionOfGeometryA==Dimension::A && dimensionOfGeometryB==Dimension::A)) {
		return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
			   matches(matrix[Location::INTERIOR][Location::EXTERIOR], 'T') &&
			   matches(matrix[Location::EXTERIOR][Location::INTERIOR],'T');
	}
	if (dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::L) {
		return matrix[Location::INTERIOR][Location::INTERIOR]==1 &&
			   matches(matrix[Location::INTERIOR][Location::EXTERIOR], 'T') &&
			   matches(matrix[Location::EXTERIOR][Location::INTERIOR], 'T');
	}
	return false;
}

/*public*/
bool
IntersectionMatrix::isCovers() const
{
	bool hasPointInCommon =
		matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T')
		||
		matches(matrix[Location::INTERIOR][Location::BOUNDARY], 'T')
		||
		matches(matrix[Location::BOUNDARY][Location::INTERIOR], 'T')
		||
		matches(matrix[Location::BOUNDARY][Location::BOUNDARY], 'T');

	return hasPointInCommon
		&&
		matrix[Location::EXTERIOR][Location::INTERIOR] ==
			Dimension::False
		&&
		matrix[Location::EXTERIOR][Location::BOUNDARY] ==
			Dimension::False;
}

/*public*/
bool
IntersectionMatrix::isCoveredBy() const
{
	bool hasPointInCommon =
		matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T')
		||
		matches(matrix[Location::INTERIOR][Location::BOUNDARY], 'T')
		||
		matches(matrix[Location::BOUNDARY][Location::INTERIOR], 'T')
		||
		matches(matrix[Location::BOUNDARY][Location::BOUNDARY], 'T');

	return
		hasPointInCommon
		&&
		matrix[Location::INTERIOR][Location::EXTERIOR] ==
			Dimension::False
		&&
		matrix[Location::BOUNDARY][Location::EXTERIOR] ==
			Dimension::False;
}

//Not sure
IntersectionMatrix*
IntersectionMatrix::transpose()
{		
	int temp = matrix[1][0];
	matrix[1][0] = matrix[0][1];
	matrix[0][1] = temp;
	temp = matrix[2][0];
	matrix[2][0] = matrix[0][2];
	matrix[0][2] = temp;
	temp = matrix[2][1];
	matrix[2][1] = matrix[1][2];
	matrix[1][2] = temp;
	return this;
}

/*public*/
string
IntersectionMatrix::toString() const
{
	string result("");
	for (int ai = 0; ai < firstDim; ai++) {
		for (int bi = 0; bi < secondDim; bi++) {
			result += Dimension::toDimensionSymbol(matrix[ai][bi]);
		}
	}
	return result;
}

std::ostream&
operator<< (std::ostream&os, const IntersectionMatrix& im)
{
	return os << im.toString();
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.26  2006/06/12 10:10:39  strk
 * Fixed getGeometryN() to take size_t rather then int, changed unsigned int parameters to size_t.
 *
 * Revision 1.25  2006/05/17 17:41:10  strk
 * Added output operator + test
 *
 * Revision 1.24  2006/05/17 17:24:17  strk
 * Added port info, fixed isCoveredBy() comment.
 *
 * Revision 1.23  2006/05/17 17:20:10  strk
 * added isCovers() and isCoveredBy() public methods to IntersectionMatrix and associated tests.
 *
 * Revision 1.22  2006/04/09 01:41:48  mloskot
 * Added comments for doxygen based on JTS docs. Added row/col dimension consts. Added asserts in functions to check if given row/col is in range.
 *
 * Revision 1.21  2006/03/22 16:58:34  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 * Revision 1.20  2006/03/13 21:54:56  strk
 * Streamlined headers inclusion.
 *
 * Revision 1.19  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.18  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.17  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.16  2006/03/02 11:00:26  strk
 * Changed IntersectionMatrix funx taking strings to take const string& instead
 *
 * Revision 1.15  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.14  2006/01/31 19:07:33  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.13  2005/11/09 08:57:07  strk
 * IntersectionMatrix made a concrete and final type.
 * Cleanups in class definition.
 *
 * Revision 1.12  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.11  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

