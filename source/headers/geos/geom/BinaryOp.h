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
 *	- Try snaping input coordinates to a increasing grid (size from 1/25 to 1)
 *	- Try simplifiying input with increasing tolerance (from 0.01 to 0.04)
 *
 * If none of the step succeeds the original exception is thrown.
 *
 *
 **********************************************************************/

#include <geos/geom/Geometry.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/util/TopologyException.h>
#include <geos/precision/CommonBitsRemover.h>
#include <geos/precision/SimpleGeometryPrecisionReducer.h>
#include <geos/simplify/TopologyPreservingSimplifier.h>

#include <memory> // for auto_ptr

#define GEOS_DEBUG_BINARYOP 1


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

	// Try removing common bits
	try
	{
#if GEOS_DEBUG_BINARYOP
		std::cerr << "Trying with Common bits remover." << std::endl;
#endif
		precision::CommonBitsRemover cbr;

		cbr.add(g0);
		cbr.add(g1);

		GeomPtr rG0( cbr.removeCommonBits(g0->clone()) );
		GeomPtr rG1( cbr.removeCommonBits(g1->clone()) );

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
		throw origException;
	}
}

} // namespace geos::geom
} // namespace geos
