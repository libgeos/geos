/**********************************************************************
 * $Id: Position.h 2557 2009-06-08 09:30:55Z strk $
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
 **********************************************************************
 *
 * Last port: geomgraph/Position.java rev. 1.4 (JTS-1.10)
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_POSITION_H
#define GEOS_GEOMGRAPH_POSITION_H

#include <geos/export.h>
#include <map>
#include <vector>
#include <string>

#include <geos/inline.h>


namespace geos {
namespace geomgraph { // geos.geomgraph

class GEOS_DLL Position {
public:
	enum {
		/*
		 * An indicator that a Location is <i>on</i>
		 * a GraphComponent
		 */
		ON=0,

		/*
		 * An indicator that a Location is to the
		 * <i>left</i> of a GraphComponent
		 */  
		LEFT,

		/*
		 * An indicator that a Location is to the
		 * <i>right</i> of a GraphComponent
		 */  
		RIGHT
	};

	/**
	 * Returns LEFT if the position is RIGHT, RIGHT if
	 * the position is LEFT, or the position otherwise.
	 */
	static int opposite(int position);
};

} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/Position.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_POSITION_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

