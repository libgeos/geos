/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_NODING_SNAPROUND_MCINDEXSNAPROUNDER_INL
#define GEOS_NODING_SNAPROUND_MCINDEXSNAPROUNDER_INL

#include <geos/noding/snapround/MCIndexSnapRounder.h>
#include <geos/noding/snapround/MCIndexPointSnapper.h>
#include <geos/noding/SegmentString.h>
#include <geos/geom/PrecisionModel.h>

#include <memory>

namespace geos {
namespace noding { // geos::noding
namespace snapround { // geos::noding::snapround

INLINE std::vector<SegmentString*>*
MCIndexSnapRounder::getNodedSubstrings() const
{
	return SegmentString::getNodedSubstrings(*nodedSegStrings);
}

INLINE
MCIndexSnapRounder::MCIndexSnapRounder(geom::PrecisionModel& nPm)
		:
		pm(nPm),
		scaleFactor(nPm.getScale()),
		pointSnapper(0)
{}

} // namespace geos::noding::snapround
} // namespace geos::noding
} // namespace geos

#endif // GEOS_NODING_SNAPROUND_MCINDEXSNAPROUNDER_INL

