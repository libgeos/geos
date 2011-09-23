/**********************************************************************
 * $Id: RelateNodeFactory.cpp 2543 2009-06-05 10:23:23Z strk $
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
 * Last port: operation/relate/RelateNodeFactory.java rev. 1.11 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/operation/relate/RelateNodeFactory.h>
#include <geos/operation/relate/RelateNode.h>
#include <geos/operation/relate/EdgeEndBundleStar.h>

using namespace geos::geom;
using namespace geos::geomgraph;

namespace geos {
namespace operation { // geos.operation
namespace relate { // geos.operation.relate

Node*
RelateNodeFactory::createNode(const Coordinate &coord) const
{
	return new RelateNode(coord, new EdgeEndBundleStar());
}

const NodeFactory &
RelateNodeFactory::instance() 
{
	static const RelateNodeFactory rnf;
	return rnf;
}


} // namespace geos.operation.relate
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/03/21 13:11:29  strk
 * opRelate.h header split
 *
 * Revision 1.9  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.8  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 **********************************************************************/

