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
 * Revision 1.2.4.1  2005/11/29 16:58:17  strk
 * Back-ported WKB IO and C api.
 * Added required higher dimensional interfaces for CoordinateSequence
 *
 * Revision 1.2  2004/07/19 10:37:03  strk
 * defaultCoordinateSequenceFactory made module-static (use DefaultCoordinateSequenceFactory::instance() instead)
 *
 * Revision 1.1  2004/07/08 19:38:56  strk
 * renamed from *List* equivalents
 *
 **********************************************************************/


#include <geos/geom.h>
#include <stdio.h>

namespace geos {

static DefaultCoordinateSequenceFactory defaultCoordinateSequenceFactory;

CoordinateSequence *
DefaultCoordinateSequenceFactory::create(vector<Coordinate> *coords) const
{
	return new DefaultCoordinateSequence(coords);
}

CoordinateSequence *
DefaultCoordinateSequenceFactory::create(unsigned int size, unsigned int dims) const
{
	return new DefaultCoordinateSequence(size);
}

const CoordinateSequenceFactory *
DefaultCoordinateSequenceFactory::instance()
{
	//static DefaultCoordinateSequenceFactory *inst = new DefaultCoordinateSequenceFactory;
	//return inst;
	//return &DefaultCoordinateSequenceFactory::internalFactory;
	return &defaultCoordinateSequenceFactory;
}

}
