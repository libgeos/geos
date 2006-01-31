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
 * Revision 1.7  2006/01/31 19:07:34  strk
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
 * Revision 1.6  2005/12/08 14:14:07  strk
 * ElevationMatrixFilter used for both elevation and Matrix fill,
 * thus removing CoordinateSequence copy in ElevetaionMatrix::add(Geometry *).
 * Changed CoordinateFilter::filter_rw to be a const method: updated
 * all apply_rw() methods to take a const CoordinateFilter.
 *
 * Revision 1.5  2005/11/10 15:20:32  strk
 * Made virtual overloads explicit.
 *
 * Revision 1.4  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.3  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/06 17:58:22  strk
 * Removed deprecated Geometry constructors based on PrecisionModel and
 * SRID specification. Removed SimpleGeometryPrecisionReducer capability
 * of changing Geometry's factory. Reverted Geometry::factory member
 * to be a reference to external factory.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.4  2004/05/03 17:15:38  strk
 * leaks on exception fixed.
 *
 * Revision 1.3  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.2  2004/04/13 13:31:11  strk
 * prototype mismatch fixed
 *
 * Revision 1.1  2004/04/10 22:41:25  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

#ifndef GEOS_PRECISION_H
#define GEOS_PRECISION_H

#include <geos/platform.h>
#include <geos/geom.h>
#include <geos/geomUtil.h>
#include <string>
#include <vector>

using namespace std;
using namespace geos::geom::util;

namespace geos {

/*
 * Determines the maximum number of common most-significant
 * bits in the mantissa of one or numbers.
 * Can be used to compute the double-precision number which
 * is represented by the common bits.
 * If there are no common bits, the number computed is 0.0.
 *
 */
class CommonBits {
private:
	bool isFirst;
	int commonMantissaBitsCount;
	int64 commonBits;
	int64 commonSignExp;
public:
	/**
	* Computes the bit pattern for the sign and exponent of a
	* double-precision number.
	* @param num
	* @return the bit pattern for the sign and exponent
	*/
	static int64 signExpBits(int64 num);
	/**
	* This computes the number of common most-significant bits in the mantissas
	* of two double-precision numbers.
	* It does not count the hidden bit, which is always 1.
	* It does not determine whether the numbers have the same exponent - if they do
	* not, the value computed by this function is meaningless.
	* @param db
	* @return the number of common most-significant mantissa bits
	*/
	static int numCommonMostSigMantissaBits(int64 num1, int64 num2);
	/**
	* Zeroes the lower n bits of a bitstring.
	* @param bits the bitstring to alter
	* @param i the number of bits to zero
	* @return the zeroed bitstring
	*/
	static int64 zeroLowerBits(int64 bits, int nBits);
	/**
	* Extracts the i'th bit of a bitstring.
	* @param bits the bitstring to extract from
	* @param i the bit to extract
	* @return the value of the extracted bit
	*/
	static int getBit(int64 bits, int i);
	CommonBits();
	void add(double num);
	double getCommon();
	/**
	* A representation of the Double bits formatted for easy readability
	*/
	string toString(int64 bits){return "";};
};

class CommonCoordinateFilter: public CoordinateFilter {
private:
	CommonBits *commonBitsX;
	CommonBits *commonBitsY;
public:
	CommonCoordinateFilter();
	~CommonCoordinateFilter();
	void filter_rw(Coordinate *coord) const;
	void filter_ro(const Coordinate *coord);
	Coordinate* getCommonCoordinate();
};

class Translater: public CoordinateFilter {
private:
	Coordinate trans;
public:
	Translater(Coordinate &newTrans);
	void filter_rw(Coordinate *coord) const;
	void filter_ro(const Coordinate *coord){}; //Not used
};

/*
 * Allow computing and removing common mantissa bits from one or
 * more Geometries.
 *
 */
class CommonBitsRemover {
private:
	Coordinate commonCoord;
	CommonCoordinateFilter *ccFilter;
public:
	CommonBitsRemover();
	~CommonBitsRemover();
	/**
	* Add a geometry to the set of geometries whose common bits are
	* being computed.  After this method has executed the
	* common coordinate reflects the common bits of all added
	* geometries.
	*
	* @param geom a Geometry to test for common bits
	*/
	void add(Geometry *geom);
	/**
	* The common bits of the Coordinates in the supplied Geometries.
	*/
	Coordinate& getCommonCoordinate();
	/**
	* Removes the common coordinate bits from a Geometry.
	* The coordinates of the Geometry are changed.
	*
	* @param geom the Geometry from which to remove the common coordinate bits
	* @return the shifted Geometry
	*/
	Geometry* removeCommonBits(Geometry *geom);
	/**
	* Adds the common coordinate bits back into a Geometry.
	* The coordinates of the Geometry are changed.
	*
	* @param geom the Geometry to which to add the common coordinate bits
	* @return the shifted Geometry
	*/
	void addCommonBits(Geometry *geom);
};

/*
 * Provides versions of Geometry spatial functions which use
 * common bit removal to reduce the likelihood of robustness problems.
 * <p>
 * In the current implementation no rounding is performed on the
 * reshifted result geometry, which means that it is possible
 * that the returned Geometry is invalid.
 * Client classes should check the validity of the returned result themselves.
 */
class CommonBitsOp {
private:
	bool returnToOriginalPrecision;
	CommonBitsRemover *cbr;
	/**
	* Computes a copy of the input {@link Geometry} with the calculated common bits
	* removed from each coordinate.
	* @param geom0 the Geometry to remove common bits from
	* @return a copy of the input Geometry with common bits removed
	*/
	Geometry* removeCommonBits(Geometry *geom0);
	/**
	* Computes a copy of each input {@link Geometry}s with the calculated common bits
	* removed from each coordinate.
	* @param geom0 a Geometry to remove common bits from
	* @param geom1 a Geometry to remove common bits from
	* @return an array containing copies
	* of the input Geometry's with common bits removed
	*/
	vector<Geometry*>* removeCommonBits(Geometry *geom0, Geometry *geom1);
public:
	/**
	* Creates a new instance of class, which reshifts result {@link Geometry}s.
	*/
	CommonBitsOp();
	/**
	* Creates a new instance of class, specifying whether
	* the result {@link Geometry}s should be reshifted.
	*
	* @param returnToOriginalPrecision
	*/
	CommonBitsOp(bool nReturnToOriginalPrecision);
	/**
	* Computes the set-theoretic intersection of two {@link Geometry}s, using enhanced precision.
	* @param geom0 the first Geometry
	* @param geom1 the second Geometry
	* @return the Geometry representing the set-theoretic intersection of the input Geometries.
	*/
	Geometry* intersection(Geometry *geom0, Geometry *geom1);
	/**
	* Computes the set-theoretic union of two {@link Geometry}s, using enhanced precision.
	* @param geom0 the first Geometry
	* @param geom1 the second Geometry
	* @return the Geometry representing the set-theoretic union of the input Geometries.
	*/
	Geometry* Union(Geometry *geom0, Geometry *geom1);
	/**
	* Computes the set-theoretic difference of two {@link Geometry}s, using enhanced precision.
	* @param geom0 the first Geometry
	* @param geom1 the second Geometry, to be subtracted from the first
	* @return the Geometry representing the set-theoretic difference of the input Geometries.
	*/
	Geometry* difference(Geometry *geom0, Geometry *geom1);
	/**
	* Computes the set-theoretic symmetric difference of two geometries,
	* using enhanced precision.
	* @param geom0 the first Geometry
	* @param geom1 the second Geometry
	* @return the Geometry representing the set-theoretic symmetric difference of the input Geometries.
	*/
	Geometry* symDifference(Geometry *geom0, Geometry *geom1);
	/**
	* Computes the buffer a geometry,
	* using enhanced precision.
	* @param geom0 the Geometry to buffer
	* @param distance the buffer distance
	* @return the Geometry representing the buffer of the input Geometry.
	*/
	Geometry* buffer(Geometry *geom0, double distance);
	/**
	* If required, returning the result to the orginal precision if required.
	* <p>
	* In this current implementation, no rounding is performed on the
	* reshifted result geometry, which means that it is possible
	* that the returned Geometry is invalid.
	*
	* @param result the result Geometry to modify
	* @return the result Geometry with the required precision
	*/
	Geometry* computeResultPrecision(Geometry *result);
};


/*
 * Provides versions of Geometry spatial functions which use
 * enhanced precision techniques to reduce the likelihood of robustness
 * problems.
 */
class EnhancedPrecisionOp {
public:
	/**
	* Computes the set-theoretic intersection of two {@link Geometry}s, using enhanced precision.
	* @param geom0 the first Geometry
	* @param geom1 the second Geometry
	* @return the Geometry representing the set-theoretic intersection of the input Geometries.
	*/
	static Geometry* intersection(Geometry *geom0, Geometry *geom1);
	/**
	* Computes the set-theoretic union of two {@link Geometry}s, using enhanced precision.
	* @param geom0 the first Geometry
	* @param geom1 the second Geometry
	* @return the Geometry representing the set-theoretic union of the input Geometries.
	*/
	static Geometry* Union(Geometry *geom0, Geometry *geom1);
	/**
	* Computes the set-theoretic difference of two {@link Geometry}s, using enhanced precision.
	* @param geom0 the first Geometry
	* @param geom1 the second Geometry
	* @return the Geometry representing the set-theoretic difference of the input Geometries.
	*/
	static Geometry* difference(Geometry *geom0, Geometry *geom1);
	/**
	* Computes the set-theoretic symmetric difference of two {@link Geometry}s, using enhanced precision.
	* @param geom0 the first Geometry
	* @param geom1 the second Geometry
	* @return the Geometry representing the set-theoretic symmetric difference of the input Geometries.
	*/
	static Geometry* symDifference(Geometry *geom0, Geometry *geom1);
	/**
	* Computes the buffer of a {@link Geometry}, using enhanced precision.
	* This method should no longer be necessary, since the buffer algorithm
	* now is highly robust.
	*
	* @param geom0 the first Geometry
	* @param distance the buffer distance
	* @return the Geometry representing the buffer of the input Geometry.
	*/
	static Geometry* buffer(Geometry *geom, double distance);
};

class SimpleGeometryPrecisionReducer;

class PrecisionReducerCoordinateOperation: public CoordinateOperation {
using CoordinateOperation::edit;
private:
	SimpleGeometryPrecisionReducer *sgpr;
public:
	PrecisionReducerCoordinateOperation(SimpleGeometryPrecisionReducer *newSgpr);
	CoordinateSequence* edit(const CoordinateSequence *coordinates, const Geometry *geom);
};

/*
 * Reduces the precision of a {@link Geometry}
 * according to the supplied {@link PrecisionModel}, without
 * attempting to preserve valid topology.
 * <p>
 * The topology of the resulting geometry may be invalid if
 * topological collapse occurs due to coordinates being shifted.
 * It is up to the client to check this and handle it if necessary.
 * Collapses may not matter for some uses.  An example
 * is simplifying the input to the buffer algorithm.
 * The buffer algorithm does not depend on the validity of the input geometry.
 *
 */
class SimpleGeometryPrecisionReducer {
private:
	PrecisionModel *newPrecisionModel;
	bool removeCollapsed;
	//bool changePrecisionModel;
public:
	SimpleGeometryPrecisionReducer(PrecisionModel *pm);
	/**
	* Sets whether the reduction will result in collapsed components
	* being removed completely, or simply being collapsed to an (invalid)
	* Geometry of the same type.
	*
	* @param removeCollapsed if <code>true</code> collapsed components will be removed
	*/
	void setRemoveCollapsedComponents(bool nRemoveCollapsed);

	/*
	* Sets whether the {@link PrecisionModel} of the new reduced Geometry
	* will be changed to be the {@link PrecisionModel} supplied to
	* specify the reduction.  The default is to not change the precision model
	*
	* @param changePrecisionModel if <code>true</code> the precision model of the created Geometry will be the
	* the precisionModel supplied in the constructor.
	*/
	//void setChangePrecisionModel(bool nChangePrecisionModel);

	PrecisionModel* getPrecisionModel();

	bool getRemoveCollapsed();
	Geometry* reduce(const Geometry *geom);
};

}
#endif
