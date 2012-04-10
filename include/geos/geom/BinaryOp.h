/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: ORIGINAL WORK
 *
 **********************************************************************
 *
 * This file provides a single templated function, taking two
 * const Geometry pointers, applying a binary operator to them
 * and returning a result Geometry in an auto_ptr<>.
 *
 * The binary operator is expected to take two const Geometry pointers
 * and return a newly allocated Geometry pointer, possibly throwing
 * a TopologyException to signal it couldn't succeed due to robustness
 * issues.
 *
 * This function will catch TopologyExceptions and try again with
 * slightly modified versions of the input. The following heuristic
 * is used:
 *
 *	- Try with original input.
 *	- Try removing common bits from input coordinate values
 *	- Try snaping input geometries to each other
 *	- Try snaping input coordinates to a increasing grid (size from 1/25 to 1)
 *	- Try simplifiying input with increasing tolerance (from 0.01 to 0.04)
 *
 * If none of the step succeeds the original exception is thrown.
 *
 * Note that you can skip Grid snapping, Geometry snapping and Simplify policies
 * by a compile-time define when building geos.
 * See USE_TP_SIMPLIFY_POLICY, USE_PRECISION_REDUCTION_POLICY and
 * USE_SNAPPING_POLICY macros below.
 *
 *
 **********************************************************************/

#ifndef GEOS_GEOM_BINARYOP_H
#define GEOS_GEOM_BINARYOP_H

#include <geos/geom/Geometry.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/precision/CommonBitsRemover.h>
#include <geos/precision/SimpleGeometryPrecisionReducer.h>

#include <geos/operation/overlay/snap/GeometrySnapper.h>

#include <geos/simplify/TopologyPreservingSimplifier.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/operation/valid/TopologyValidationError.h>
#include <geos/util/TopologyException.h>
#include <geos/util.h>

#include <memory> // for auto_ptr

//#define GEOS_DEBUG_BINARYOP 1

#ifdef GEOS_DEBUG_BINARYOP
# include <iostream>
# include <iomanip>
# include <sstream>
#endif


/*
 * Always try original input first
 */
#ifndef USE_ORIGINAL_INPUT
# define USE_ORIGINAL_INPUT 1
#endif

/*
 * Define this to use PrecisionReduction policy
 * in an attempt at by-passing binary operation
 * robustness problems (handles TopologyExceptions)
 */
#ifndef USE_PRECISION_REDUCTION_POLICY
//# define USE_PRECISION_REDUCTION_POLICY 1
#endif

/*
 * Define this to use TopologyPreserving simplification policy
 * in an attempt at by-passing binary operation
 * robustness problems (handles TopologyExceptions)
 */
#ifndef USE_TP_SIMPLIFY_POLICY 
//# define USE_TP_SIMPLIFY_POLICY 1
#endif

/*
 * Use common bits removal policy.
 * If enabled, this would be tried /before/
 * Geometry snapping.
 */
#ifndef USE_COMMONBITS_POLICY
# define USE_COMMONBITS_POLICY 1
#endif

/*
 * Check validity of operation performed
 * by common bits removal policy.
 *
 * This matches what EnhancedPrecisionOp does in JTS
 * and fixes 5 tests of invalid outputs in our testsuite
 * (stmlf-cases-20061020-invalid-output.xml)
 * and breaks 1 test (robustness-invalid-output.xml) so much
 * to prevent a result.
 *
 */
#define GEOS_CHECK_COMMONBITS_VALIDITY 1

/*
 * Use snapping policy
 */
#ifndef USE_SNAPPING_POLICY
# define USE_SNAPPING_POLICY 1
#endif

namespace geos {
namespace geom { // geos::geom

inline bool
check_valid(const Geometry& g, const std::string& label)
{
	operation::valid::IsValidOp ivo(&g);
	if ( ! ivo.isValid() )
	{
#ifdef GEOS_DEBUG_BINARYOP
		using operation::valid::TopologyValidationError;
		TopologyValidationError* err = ivo.getValidationError();
		std::cerr << label << " is INVALID: "
			<< err->toString()
			<< " (" << std::setprecision(20)
			<< err->getCoordinate() << ")"
			<< std::endl;
#else
        (void)label;
#endif
		return false;
	} 
	return true;
}

/* A single component may become a multi component */
inline std::auto_ptr<Geometry>
fix_self_intersections(std::auto_ptr<Geometry> g, const std::string& label)
{
  // TODO: check for presence of self-intersections first ?
  return g->Union();
}


/// \brief
/// Apply a binary operation to the given geometries
/// after snapping them to each other after common-bits
/// removal.
///
template <class BinOp>
std::auto_ptr<Geometry>
SnapOp(const Geometry* g0, const Geometry *g1, BinOp _Op)
{
	typedef std::auto_ptr<Geometry> GeomPtr;

#define CBR_BEFORE_SNAPPING 1

	//using geos::precision::GeometrySnapper;
	using geos::operation::overlay::snap::GeometrySnapper;

	// Snap tolerance must be computed on the original
	// (not commonbits-removed) geoms
	double snapTolerance = GeometrySnapper::computeOverlaySnapTolerance(*g0, *g1);
#if GEOS_DEBUG_BINARYOP
	std::cerr<< std::setprecision(20) << "Computed snap tolerance: "<<snapTolerance<<std::endl;
#endif


#if CBR_BEFORE_SNAPPING
	// Compute common bits
	geos::precision::CommonBitsRemover cbr;
	cbr.add(g0); cbr.add(g1);
#if GEOS_DEBUG_BINARYOP
	std::cerr<<"Computed common bits: "<<cbr.getCommonCoordinate()<<std::endl;
#endif

	// Now remove common bits
	GeomPtr rG0( cbr.removeCommonBits(g0->clone()) );
	GeomPtr rG1( cbr.removeCommonBits(g1->clone()) );

#if GEOS_DEBUG_BINARYOP
	check_valid(*rG0, "CBR: removed-bits geom 0");
	check_valid(*rG1, "CBR: removed-bits geom 1");
#endif

	const Geometry& operand0 = *rG0;
	const Geometry& operand1 = *rG1;
#else // don't CBR before snapping
	const Geometry& operand0 = *g0;
	const Geometry& operand1 = *g1;
#endif


	GeometrySnapper snapper0( operand0 );
	GeomPtr snapG0( snapper0.snapTo(operand1, snapTolerance) );
	snapG0 = fix_self_intersections(snapG0, "SNAP: snapped geom 0");

	// NOTE: second geom is snapped on the snapped first one
	GeometrySnapper snapper1( operand1 );
	GeomPtr snapG1( snapper1.snapTo(*snapG0, snapTolerance) );
	snapG1 = fix_self_intersections(snapG1, "SNAP: snapped geom 1");

	// Run the binary op
	GeomPtr result( _Op(snapG0.get(), snapG1.get()) );

#if GEOS_DEBUG_BINARYOP
	check_valid(*result, "SNAP: result (before common-bits addition");
#endif

#if CBR_BEFORE_SNAPPING
	// Add common bits back in
	cbr.addCommonBits( result.get() );
	result = fix_self_intersections(result, "SNAP: result (after common-bits addition)");
#endif

#if GEOS_DEBUG_BINARYOP
	check_valid(*result, "SNAP: result (after common-bits addition");
#endif

	return result;
}

template <class BinOp>
std::auto_ptr<Geometry>
BinaryOp(const Geometry* g0, const Geometry *g1, BinOp _Op)
{
	typedef std::auto_ptr<Geometry> GeomPtr;

	GeomPtr ret;
	geos::util::TopologyException origException;

#ifdef USE_ORIGINAL_INPUT
	// Try with original input
	try
	{
#if GEOS_DEBUG_BINARYOP
		std::cerr << "Trying with original input." << std::endl;
#endif
		ret.reset(_Op(g0, g1));
		return ret;
	}
	catch (const geos::util::TopologyException& ex)
	{
		origException=ex;
#if GEOS_DEBUG_BINARYOP
		std::cerr << "Original exception: " << ex.what() << std::endl;
#endif
	}

//#if GEOS_DEBUG_BINARYOP
  // Should we just give up here ?
  check_valid(*g0, "Input geom 0");
  check_valid(*g1, "Input geom 1");
//#endif

#endif // USE_ORIGINAL_INPUT


#ifdef USE_COMMONBITS_POLICY
	// Try removing common bits (possibly obsoleted by snapping below)
	//
	// NOTE: this policy was _later_ implemented 
	//       in JTS as EnhancedPrecisionOp
	// TODO: consider using the now-ported EnhancedPrecisionOp
	//       here too
	// 
	try
	{
		GeomPtr rG0;
		GeomPtr rG1;
		precision::CommonBitsRemover cbr;

#if GEOS_DEBUG_BINARYOP
		std::cerr << "Trying with Common Bits Remover (CBR)" << std::endl;
#endif

		cbr.add(g0);
		cbr.add(g1);

		rG0.reset( cbr.removeCommonBits(g0->clone()) );
		rG1.reset( cbr.removeCommonBits(g1->clone()) );

#if GEOS_DEBUG_BINARYOP
		check_valid(*rG0, "CBR: geom 0 (after common-bits removal)");
		check_valid(*rG1, "CBR: geom 1 (after common-bits removal)");
#endif

		ret.reset( _Op(rG0.get(), rG1.get()) );

#if GEOS_DEBUG_BINARYOP
		check_valid(*ret, "CBR: result (before common-bits addition)");
#endif

		cbr.addCommonBits( ret.get() ); 

#if GEOS_DEBUG_BINARYOP
		check_valid(*ret, "CBR: result (after common-bits addition)");
#endif

		// Common-bits removal policy could introduce self-intersections
		ret = fix_self_intersections(ret, "CBR: result (after common-bits addition)");

#if GEOS_DEBUG_BINARYOP
		check_valid(*ret, "CBR: result (after common-bits addition and fix_self_intersections)");
#endif

#if GEOS_CHECK_COMMONBITS_VALIDITY
		// check that result is a valid geometry after the
		// reshift to orginal precision (see EnhancedPrecisionOp)
		using operation::valid::IsValidOp;
		using operation::valid::TopologyValidationError;
		IsValidOp ivo(ret.get());
		if ( ! ivo.isValid() )
		{
			TopologyValidationError* e = ivo.getValidationError();
			throw geos::util::TopologyException(
				"Result of overlay became invalid "
				"after re-addin common bits of operand "
				"coordinates: " + e->toString(),
			        e->getCoordinate());
		}
#endif // GEOS_CHECK_COMMONBITS_VALIDITY

		return ret;
	}
	catch (const geos::util::TopologyException& ex)
	{
        ::geos::ignore_unused_variable_warning(ex);
#if GEOS_DEBUG_BINARYOP
		std::cerr << "CBR: " << ex.what() << std::endl;
#endif
	}
#endif

	// Try with snapping
	//
	// TODO: possible optimization would be reusing the
	//       already common-bit-removed inputs and just
	//       apply geometry snapping, whereas the current
	//       SnapOp function does both.
// {
#if USE_SNAPPING_POLICY

#if GEOS_DEBUG_BINARYOP
	std::cerr << "Trying with snapping " << std::endl;
#endif

	try {
		ret = SnapOp(g0, g1, _Op);
#if GEOS_DEBUG_BINARYOP
	std::cerr << "SnapOp succeeded" << std::endl;
#endif
		return ret;
		
	}
	catch (const geos::util::TopologyException& ex)
	{
        ::geos::ignore_unused_variable_warning(ex);
#if GEOS_DEBUG_BINARYOP
		std::cerr << "SNAP: " << ex.what() << std::endl;
#endif
	}

#endif // USE_SNAPPING_POLICY }



// {
#if USE_PRECISION_REDUCTION_POLICY


	// Try reducing precision
	try
	{
		int maxPrecision=25;

		for (int precision=maxPrecision; precision; --precision)
		{
			std::auto_ptr<PrecisionModel> pm(new PrecisionModel(precision));
#if GEOS_DEBUG_BINARYOP
			std::cerr << "Trying with precision " << precision << std::endl;
#endif

			precision::SimpleGeometryPrecisionReducer reducer( pm.get() );
			GeomPtr rG0( reducer.reduce(g0) );
			GeomPtr rG1( reducer.reduce(g1) );

			try
			{
				ret.reset( _Op(rG0.get(), rG1.get()) );
				return ret;
			}
			catch (const geos::util::TopologyException& ex)
			{
				if ( precision == 1 ) throw ex;
#if GEOS_DEBUG_BINARYOP
				std::cerr << "Reduced with precision (" << precision << "): "
				          << ex.what() << std::endl;
#endif
			}

		}

	}
	catch (const geos::util::TopologyException& ex)
	{
#if GEOS_DEBUG_BINARYOP
		std::cerr << "Reduced: " << ex.what() << std::endl;
#endif
	}

#endif
// USE_PRECISION_REDUCTION_POLICY }

// {
#if USE_TP_SIMPLIFY_POLICY 

	// Try simplifying
	try
	{

		double maxTolerance = 0.04;
		double minTolerance = 0.01;
		double tolStep = 0.01;

		for (double tol = minTolerance; tol <= maxTolerance; tol += tolStep)
		{
#if GEOS_DEBUG_BINARYOP
			std::cerr << "Trying simplifying with tolerance " << tol << std::endl;
#endif

			GeomPtr rG0( simplify::TopologyPreservingSimplifier::simplify(g0, tol) );
			GeomPtr rG1( simplify::TopologyPreservingSimplifier::simplify(g1, tol) );

			try
			{
				ret.reset( _Op(rG0.get(), rG1.get()) );
				return ret;
			}
			catch (const geos::util::TopologyException& ex)
			{
				if ( tol >= maxTolerance ) throw ex;
#if GEOS_DEBUG_BINARYOP
				std::cerr << "Simplified with tolerance (" << tol << "): "
				          << ex.what() << std::endl;
#endif
			}

		}

		return ret;

	}
	catch (const geos::util::TopologyException& ex)
	{
#if GEOS_DEBUG_BINARYOP
		std::cerr << "Simplified: " << ex.what() << std::endl;
#endif
	}

#endif
// USE_TP_SIMPLIFY_POLICY }

	throw origException;
}


} // namespace geos::geom
} // namespace geos

#endif // GEOS_GEOM_BINARYOP_H
