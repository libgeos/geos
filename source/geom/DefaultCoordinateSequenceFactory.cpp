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
 **********************************************************************/

#include <geos/geom.h>
#include <stdio.h>

namespace geos {

static DefaultCoordinateSequenceFactory defaultCoordinateSequenceFactory;

const CoordinateSequenceFactory *
DefaultCoordinateSequenceFactory::instance()
{
	return &defaultCoordinateSequenceFactory;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.3  2005/04/29 11:52:40  strk
 * Added new JTS interfaces for CoordinateSequence and factories,
 * removed example implementations to reduce maintainance costs.
 * Added first implementation of WKBWriter, made ByteOrderDataInStream
 * a template class.
 *
 * Revision 1.2  2004/07/19 10:37:03  strk
 * defaultCoordinateSequenceFactory made module-static (use DefaultCoordinateSequenceFactory::instance() instead)
 *
 * Revision 1.1  2004/07/08 19:38:56  strk
 * renamed from *List* equivalents
 *
 **********************************************************************/

