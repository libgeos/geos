/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: simplify/TopologyPreservingSimplifier.java rev. 1.4 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/simplify/TopologyPreservingSimplifier.h>
#include <geos/simplify/TaggedLinesSimplifier.h>


//#include <geos/simplify/DouglasPeuckerLineSimplifier.h>
//#include <geos/geom/Geometry.h> // for AutoPtr typedefs
//#include <geos/geom/MultiPolygon.h> 
//#include <geos/geom/CoordinateSequence.h> // for AutoPtr typedefs
//#include <geos/geom/GeometryFactory.h> 
//#include <geos/geom/CoordinateSequenceFactory.h> 
//#include <geos/geom/util/GeometryTransformer.h> // for DPTransformer inheritance
//#include <geos/util/IllegalArgumentException.h> 

#include <memory> // for auto_ptr
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

using namespace geos::geom;

namespace geos {
namespace simplify { // geos::simplify

/*public static*/
std::auto_ptr<geom::Geometry>
TopologyPreservingSimplifier::simplify(
		const geom::Geometry* geom,
		double tolerance)
{
	TopologyPreservingSimplifier tss(geom);
        tss.setDistanceTolerance(distanceTolerance);
	return tss.getResultGeometry();
}

/*public*/
TopologyPreservingSimplifier::TopologyPreservingSimplifier(const Geometry* geom)
	:
	inputGeom(geom),
	lineSimplifier(new TaggedLinesSimplifier())
{
}

/*public*/
TopologyPreservingSimplifier::setDistanceTolerance(double d)
{
	using geos::util::IllegalArgumentException;

	if ( d < 0.0 )
		throw IllegalArgumentException("Tolerance must be non-negative");

	lineSimplifier->setDistanceTolerance(d);
}

/*public*/
std::auto_ptr<geom::Geometry> 
TopologyPreservingSimplifier::getResultGeometry()
{
	LineStringMapBuilderFilter lsmbf;
	inputGeom->apply(new lsmbf);
	lineSimplifier->simplify(linestringMap.begin(), linestringMap.end());

	LineStringTransformer trans;
	Geometry result = trans.transform(inputGeom);

	return result;
}

TopologyPreservingSimplifier::

} // namespace geos::simplify
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/04/13 10:39:12  strk
 * Initial implementation of TaggedLinesSimplifier class
 *
 **********************************************************************/
