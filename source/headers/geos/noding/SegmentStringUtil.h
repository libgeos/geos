/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 *
 **********************************************************************/

#ifndef GEOS_NODING_SEGMENTSTRINGUTIL_H
#define GEOS_NODING_SEGMENTSTRINGUTIL_H

#include <geos/noding/NodedSegmentString.h>
#include <geos/geom/LineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/util/LinearComponentExtracter.h>

namespace geos {
namespace noding { // geos::noding

/** \brief
 * Utility methods for processing {@link SegmentString}s.
 * 
 * @author Martin Davis
 *
 */
class SegmentStringUtil
{
private:
protected:
public:
	/**
	 * Extracts all linear components from a given {@link Geometry}
	 * to {@link SegmentString}s.
	 * The SegmentString data item is set to be the source Geometry.
	 * 
	 * @param geom the geometry to extract from
	 * @return a List of SegmentStrings
	 */
	static void extractSegmentStrings( const geom::Geometry * g, SegmentString::ConstVect & segStr)
	{
		geom::LineString::ConstVect lines;
		geom::util::LinearComponentExtracter::getLines( *g, lines);

		for (size_t i=0, n=lines.size(); i<n; i++)
		{
			geom::LineString * line = (geom::LineString *)(lines[i]);

			geom::CoordinateSequence * pts = line->getCoordinates();

			segStr.push_back( new NodedSegmentString( pts, g));
		}
	}

};

} // geos::noding
} // geos

#endif // GEOS_NODING_SEGMENTSTRINGUTIL_H
/**********************************************************************
 * $Log$
 **********************************************************************/

