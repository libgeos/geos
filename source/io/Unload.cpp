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
 * Revision 1.2  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/unload.h"
#include "../headers/opValid.h"
#include "../headers/graph.h"
#include "../headers/operation.h"
#include "../headers/opRelate.h"
#include "../headers/indexQuadtree.h"

namespace geos {

Unload::Unload(void)
{
}

Unload::~Unload(void)
{
}

void Unload::Release()
{
	delete CoordinateListFactory::internalFactory;
	delete Geometry::cgAlgorithms;
	delete Geometry::geometryChangedFilter;
	delete IsValidOp::cga;
	delete PlanarGraph::cga;
	delete PlanarGraph::li;
	delete GeometryGraphOperation::cga;
	delete GeometryGraphOperation::li;
	delete RelateComputer::li;
	delete RelateComputer::ptLocator;
	delete EdgeEnd::cga;
	delete QuadTreeRoot::origin;
}
}
