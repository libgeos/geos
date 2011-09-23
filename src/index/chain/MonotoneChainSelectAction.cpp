/**********************************************************************
 * $Id: MonotoneChainSelectAction.cpp 2477 2009-05-06 17:10:01Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: index/chain/MonotoneChainSelectAction.java rev. 1.6 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/index/chain/MonotoneChainSelectAction.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/LineSegment.h>


namespace geos {
namespace index { // geos.index
namespace chain { // geos.index.chain

void
MonotoneChainSelectAction::select(MonotoneChain& mc, unsigned int start)
{
	mc.getLineSegment(start, selectedSegment);

	select(selectedSegment);
}

} // namespace geos.index.chain
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2006/03/22 18:12:32  strk
 * indexChain.h header split.
 *
 **********************************************************************/

