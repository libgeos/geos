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

#include <geos/precision.h>

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
 * Revision 1.4  2006/03/02 16:21:26  strk
 * geos::precision namespace added
 *
 * Revision 1.3  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.2  2004/05/03 17:15:38  strk
 * leaks on exception fixed.
 *
 * Revision 1.1  2004/04/10 22:41:25  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

