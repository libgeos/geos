/**********************************************************************
 * $Id: CommonBitsOp.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

#include <geos/precision/CommonBitsOp.h>
#include <geos/precision/CommonBitsRemover.h>
#include <geos/geom/Geometry.h>

#include <vector>
#include <string>
#include <memory>
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace precision { // geos.precision

/*public*/
CommonBitsOp::CommonBitsOp()
	:
	returnToOriginalPrecision(true)

{
#if GEOS_DEBUG
	std::cerr << "CommonBitsOp[" << this
	          << "]::CommonBitsOp()" << std::endl;
#endif
}

/*public*/
CommonBitsOp::CommonBitsOp(bool nReturnToOriginalPrecision)
	:
	returnToOriginalPrecision(nReturnToOriginalPrecision)
{
#if GEOS_DEBUG
	std::cerr << "CommonBitsOp[" << this
	          << "]::CommonBitsOp(bool " 
		  << nReturnToOriginalPrecision << ")"
		  << std::endl;
#endif
}

/*public*/
Geometry*
CommonBitsOp::intersection(
		const Geometry* geom0,
		const Geometry* geom1)
{
	auto_ptr<Geometry> rgeom0;
	auto_ptr<Geometry> rgeom1;
	removeCommonBits(geom0, geom1, rgeom0, rgeom1);
	return computeResultPrecision(rgeom0->intersection(rgeom1.get()));
}

/*public*/
Geometry*
CommonBitsOp::Union(
		const Geometry* geom0,
		const Geometry* geom1)
{
	auto_ptr<Geometry> rgeom0;
	auto_ptr<Geometry> rgeom1;
	removeCommonBits(geom0, geom1, rgeom0, rgeom1);
	return computeResultPrecision(rgeom0->Union(rgeom1.get()));
}

/*public*/
Geometry*
CommonBitsOp::difference(
		const Geometry* geom0,
		const Geometry* geom1)
{
	auto_ptr<Geometry> rgeom0;
	auto_ptr<Geometry> rgeom1;
	removeCommonBits(geom0, geom1, rgeom0, rgeom1);
	return computeResultPrecision(rgeom0->difference(rgeom1.get()));
}

/*public*/
Geometry*
CommonBitsOp::symDifference(
		const Geometry* geom0,
		const Geometry* geom1)
{
	auto_ptr<Geometry> rgeom0;
	auto_ptr<Geometry> rgeom1;
	removeCommonBits(geom0, geom1, rgeom0, rgeom1);
	return computeResultPrecision(rgeom0->symDifference(rgeom1.get()));
}

/*public*/
Geometry*
CommonBitsOp::buffer(const Geometry* geom0, double distance)
{
	auto_ptr<Geometry> rgeom0(removeCommonBits(geom0));
	return computeResultPrecision(rgeom0->buffer(distance));
}

/*public*/
Geometry*
CommonBitsOp::computeResultPrecision(Geometry* result)
{
	assert(cbr.get());
	if (returnToOriginalPrecision)
		cbr->addCommonBits(result);
	return result;
}

/*private*/
Geometry*
CommonBitsOp::removeCommonBits(const Geometry* geom0)
{
	cbr.reset(new CommonBitsRemover());
	cbr->add(geom0);

#if GEOS_DEBUG
	const Coordinate& commonCoord = cbr->getCommonCoordinate();
	cerr << "CommonBitsRemover bits: " << commonCoord.x << ", " << commonCoord.y << endl;
#endif

	Geometry* geom=cbr->removeCommonBits(geom0->clone());
	return geom;
}

/*private*/
void
CommonBitsOp::removeCommonBits(
		const geom::Geometry* geom0,
		const geom::Geometry* geom1,
		std::auto_ptr<geom::Geometry>& rgeom0,
		std::auto_ptr<geom::Geometry>& rgeom1)
	
{
	cbr.reset(new CommonBitsRemover());

	cbr->add(geom0);
	cbr->add(geom1);

#if GEOS_DEBUG
	const Coordinate& commonCoord = cbr->getCommonCoordinate();
	cerr << "CommonBitsRemover bits: " << commonCoord.x << ", " << commonCoord.y << endl;
#endif

	rgeom0.reset(cbr->removeCommonBits(geom0->clone()));
	rgeom1.reset(cbr->removeCommonBits(geom1->clone()));

}

} // namespace geos.precision
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/04/13 23:28:26  strk
 * "always build before commit" (forgot a closing paren)
 *
 * Revision 1.9  2006/04/13 23:23:52  strk
 * fixed bug in binary ops failing to consistently reduce operands.
 *
 * Revision 1.8  2006/04/07 08:27:12  strk
 * debugging blocks
 *
 * Revision 1.7  2006/04/06 14:36:51  strk
 * Cleanup in geos::precision namespace (leaks plugged, auto_ptr use, ...)
 *
 * Revision 1.6  2006/03/23 09:17:19  strk
 * precision.h header split, minor optimizations
 *
 **********************************************************************/

