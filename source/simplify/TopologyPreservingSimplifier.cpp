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
// for LineStringTransformer inheritance
#include <geos/geom/util/GeometryTransformer.h>
// for LineStringMapBuilderFilter inheritance
#include <geos/geom/GeometryComponentFilter.h>

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

typedef std::map<const geom::Geometry*, TaggedLineString> LinesMap;


namespace { // module-statics

class LineStringTransformer: public geom::util::GeometryTransformer
{

public:

	friend class TopologyPreservingSimplifier;

protected:

	CoordinateSequence::AutoPtr transformCoordinates(
			const CoordinateSequence* coords,
			const Geometry* parent);

private:

	LineStringTransformer(TopologyPreservingSimplifier& simp);

	LinesMap& linestringMap;

}

/*private*/
LineStringTransformer(LinesMap& nMap)
	:
	linestringMap(nMap)
{
}

/*protected*/
CoordinateSequence::AutoPtr
LineStringTrasnformer::transformCoordinates(
		const CoordinateSequence* coords,
		const Geometry* parent);
{
	if ( dynamic_cast<const LineString*>(parent) )
	{
		LinesMap::iterator it = linestringMap.find(parent);
		assert( it != linestringMap.end() );
		
		TaggedLineString& taggedLine = it->second;

		return taggedLine.getResultCoordinates();
	}

	// for anything else (e.g. points) just copy the coordinates
	return GeometryTransformer::transformCoordinates(coords, parent);
}

//----------------------------------------------------------------------

class LineStringMapBuilderFilter: public geom::GeometryComponentFilter
{

public:

	void filter_ro(const Geometry* geom)
	{
		if ( const LinearRing* lr=dynamic_cast<const LinearRing*>(
				parent) )
		{
			TaggedLineString taggedLine(lr, 4);
			linestringMap.put(geom, taggedLine);
      }
      else if (geom instanceof LineString) {
        TaggedLineString taggedLine = new TaggedLineString((LineString) geom, 2);
        linestringMap.put(geom, taggedLine);
      }
	}
};


} // end of module-statics

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
	LinesMap linestringMap;

	LineStringMapBuilderFilter lsmbf(linestringMap);
	inputGeom->apply_ro(lsmbf);

	for (LinesMap::iterator
		it=linestringMap.begin(), itEnd=linestringMap.end();
		it != itEnd;
		++it)
	{
		lineSimplifier->simplifyLine(&(it->second)); 
	}


	LineStringTransformer trans(linestringMap);
	Geometry result = trans.transform(inputGeom);

	return result;
}

} // namespace geos::simplify
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/04/13 14:25:17  strk
 * TopologyPreservingSimplifier initial port
 *
 **********************************************************************/
