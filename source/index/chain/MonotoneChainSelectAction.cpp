/**********************************************************************
 * $Id$
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
 **********************************************************************/

#include <geos/index/chain/MonotoneChainSelectAction.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/LineSegment.h>


namespace geos {
namespace index { // geos.index
namespace chain { // geos.index.chain

MonotoneChainSelectAction::MonotoneChainSelectAction()
{
	selectedSegment=new geom::LineSegment();
	tempEnv1=new geom::Envelope();
}

MonotoneChainSelectAction::~MonotoneChainSelectAction()
{
	delete selectedSegment;
	delete tempEnv1;
}

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

