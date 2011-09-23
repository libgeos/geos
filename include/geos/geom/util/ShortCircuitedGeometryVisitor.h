/**********************************************************************
 * $Id: ShortCircuitedGeometryVisitor.h 2556 2009-06-06 22:22:28Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOM_UTIL_SHORTCIRCUITEDGEOMETRYVISITOR_H
#define GEOS_GEOM_UTIL_SHORTCIRCUITEDGEOMETRYVISITOR_H

#include <geos/export.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Geometry;
	}
}


namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

/** \brief
 * A visitor to Geometry elements which can
 * be short-circuited by a given condition
 *
 * Last port: geom/util/ShortCircuitedGeometryVisitor.java rev. 1.1 (JTS-1.7)
 */
class GEOS_DLL ShortCircuitedGeometryVisitor
{

private:

	bool done;

protected:

	virtual void visit(const Geometry &element)=0;
	virtual bool isDone()=0;

public:

	ShortCircuitedGeometryVisitor()
		:
		done(false)
		{}

	void applyTo(const Geometry &geom);

	virtual ~ShortCircuitedGeometryVisitor() {}

};

} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/util/ShortCircuitedGeometryVisitor.inl"
//#endif

#endif

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/06/08 11:20:24  strk
 * Added missing virtual destructor to abstract classes.
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
