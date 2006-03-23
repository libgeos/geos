/**********************************************************************
 * $Id$
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

using namespace std;
using namespace geos::geom;

namespace geos {
namespace precision { // geos.precision

/*public*/
CommonBitsOp::CommonBitsOp()
{
	returnToOriginalPrecision=true;
}

/*public*/
CommonBitsOp::CommonBitsOp(bool nReturnToOriginalPrecision)
{
	returnToOriginalPrecision=nReturnToOriginalPrecision;
}

/*public*/
Geometry*
CommonBitsOp::intersection(Geometry* geom0, Geometry* geom1)
{
	vector<Geometry*> *geom=removeCommonBits(geom0, geom1);
	return computeResultPrecision(((*geom)[0])->intersection((*geom)[1]));
}

/*public*/
Geometry*
CommonBitsOp::Union(Geometry* geom0, Geometry* geom1)
{
	vector<Geometry*> *geom=removeCommonBits(geom0, geom1);
	return computeResultPrecision(((*geom)[0])->Union((*geom)[1]));
}

/*public*/
Geometry*
CommonBitsOp::difference(Geometry* geom0, Geometry* geom1)
{
	vector<Geometry*> *geom=removeCommonBits(geom0, geom1);
	return computeResultPrecision(((*geom)[0])->difference((*geom)[1]));
}

/*public*/
Geometry*
CommonBitsOp::symDifference(Geometry* geom0, Geometry* geom1)
{
	vector<Geometry*> *geom=removeCommonBits(geom0, geom1);
	return computeResultPrecision(((*geom)[0])->symDifference((*geom)[1]));
}

/*public*/
Geometry*
CommonBitsOp::buffer(Geometry* geom0, double distance)
{
	Geometry *geom=removeCommonBits(geom0);
	return computeResultPrecision(geom->buffer(distance));
}

/*public*/
Geometry*
CommonBitsOp::computeResultPrecision(Geometry* result)
{
	if (returnToOriginalPrecision)
		cbr->addCommonBits(result);
	return result;
}

/*private*/
Geometry*
CommonBitsOp::removeCommonBits(Geometry* geom0)
{
	cbr=new CommonBitsRemover();
	cbr->add(geom0);
	Geometry* geom=cbr->removeCommonBits(geom0->clone());
	delete cbr;
	return geom;
}

/*private*/
vector<Geometry*>*
CommonBitsOp::removeCommonBits(Geometry* geom0, Geometry* geom1)
{
	cbr=new CommonBitsRemover();
	cbr->add(geom0);
	cbr->add(geom1);
	vector<Geometry*> *gv=new vector<Geometry*>();
	gv->push_back(cbr->removeCommonBits(geom0->clone()));
	gv->push_back(cbr->removeCommonBits(geom1->clone()));
	delete cbr;
	return gv;
}

} // namespace geos.precision
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/03/23 09:17:19  strk
 * precision.h header split, minor optimizations
 *
 **********************************************************************/

