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

#include <geos/geom/Geometry.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/util/TopologyException.h>
#include <geos/precision/CommonBitsRemover.h>
#include <geos/precision/SimpleGeometryPrecisionReducer.h>
#include <geos/precision/GeometrySnapper.h>
#include <geos/simplify/TopologyPreservingSimplifier.h>

#include <memory> // for auto_ptr

#define GEOS_DEBUG_BINARYOP 1

/*
 * Define this to use PrecisionReduction policy
 * in an attempt at by-passing binary operation
 * robustness problems (handles TopologyExceptions)
 */
#ifndef USE_PRECISION_REDUCTION_POLICY
# define USE_PRECISION_REDUCTION_POLICY 1
#endif

/*
 * Define this to use TopologyPreserving simplification policy
 * in an attempt at by-passing binary operation
 * robustness problems (handles TopologyExceptions)
 */
#ifndef USE_TP_SIMPLIFY_POLICY 
# define USE_TP_SIMPLIFY_POLICY 1
#endif

/*
 * Use snapping policy
 */
#ifndef USE_SNAPPING_POLICY
# define USE_SNAPPING_POLICY 1
#endif

namespace geos {
namespace geom { // geos::geom

template <class BinOp>
std::auto_ptr<Geometry>
BinaryOp(const Geometry* g0, const Geometry *g1, BinOp _Op)
{
	typedef std::auto_ptr<Geometry> GeomPtr;

	GeomPtr ret;
	util::TopologyException origException;

	// Try with original input
	try
	{
#if GEOS_DEBUG_BINARYOP
	//std::cerr << "Trying with original input." << std::endl;
#endif
		ret.reset(_Op(g0, g1));
		return ret;
	}
	catch (const util::TopologyException& ex)
	{
		origException=ex;
#if GEOS_DEBUG_BINARYOP
		std::cerr << "Original exception: " << ex.what() << std::endl;
#endif
	}


	// Try removing common bits (possibly obsoleted by snapping below)

	// keep these out of try block to reuse in case of snapping
	GeomPtr rG0;
	GeomPtr rG1;
	precision::CommonBitsRemover cbr;

	try
	{
#if GEOS_DEBUG_BINARYOP
		std::cerr << "Trying with Common bits remover." << std::endl;
#endif

		cbr.add(g0);
		cbr.add(g1);

		rG0.reset( cbr.removeCommonBits(g0->clone()) );
		rG1.reset( cbr.removeCommonBits(g1->clone()) );

#if GEOS_DEBUG_BINARYOP
		if ( ! rG0->isValid() )
		{
			std::cerr << " CBR: geom 0 is invalid!" << std::endl;
		}

		if ( ! rG1->isValid() )
		{
			std::cerr << " CBR: geom 1 is invalid!" << std::endl;
		}
#endif

		//throw util::TopologyException("Skip op on commonbits-removed");
		ret.reset( _Op(rG0.get(), rG1.get()) );

		cbr.addCommonBits( ret.get() );

		return ret;
	}
	catch (const util::TopologyException& ex)
	{
#if GEOS_DEBUG_BINARYOP
		std::cerr << "CBR: " << ex.what() << std::endl;
#endif
	}

	// Try with snapping
// {
#if USE_SNAPPING_POLICY

#if GEOS_DEBUG_BINARYOP
	std::cerr << "Trying with snapping " << std::endl;
#endif

	try {
		// Snap the commonbits-removed geoms, to have less snaps
		// Snap each geometry on the other

		using geos::precision::GeometrySnapper;

		// Snap tolerance must be computed on the original
		// (not commonbits-removed) geoms
		double snapTolerance = GeometrySnapper::computeSnapTolerance(*g0, *g1);
		std::cerr<<"Computed snap tolerance: "<<snapTolerance<<std::endl;

		GeometrySnapper snapper0( *rG0 );
		GeomPtr snapG0( snapper0.snapTo(*rG1, snapTolerance) );

		// NOTE: second geom is snapped on the snapped first one
		GeometrySnapper snapper1( *rG1 );
		GeomPtr snapG1( snapper0.snapTo(*rG0, snapTolerance) );

		// Check input validity
#if GEOS_DEBUG_BINARYOP
		if ( ! snapG0->isValid() )
		{
			std::cerr << " SNAP: snapped geom 0 is invalid!" << std::endl;
		}

		if ( ! snapG1->isValid() )
		{
			std::cerr << " SNAP: snapped geom 1 is invalid!" << std::endl;
		}
#endif

		// Run the binary op

		ret.reset( _Op(snapG0.get(), snapG1.get()) );

		// Add common bits
		cbr.addCommonBits( ret.get() );

		return ret;

	}
	catch (const util::TopologyException& ex)
	{
#if GEOS_DEBUG_BINARYOP
		std::cerr << "SNAP: " << ex.what() << std::endl;
#endif
	}

#endif // USE_SNAPPIN_POLICY }





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
			catch (const util::TopologyException& ex)
			{
				if ( precision == 1 ) throw ex;
#if GEOS_DEBUG_BINARYOP
				std::cerr << "Reduced with precision (" << precision << "): "
				          << ex.what() << std::endl;
#endif
			}

		}

	}
	catch (const util::TopologyException& ex)
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
			catch (const util::TopologyException& ex)
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
	catch (const util::TopologyException& ex)
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
