/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/PolygonNodeTopology.h>
#include <geos/operation/valid/PolygonRingSelfNode.h>


namespace geos {      // geos
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using geos::algorithm::PolygonNodeTopology;

/* public */
bool
PolygonRingSelfNode::isExterior(bool isInteriorOnRight) const
{
    (void)e11; // unused variable
    /**
     * Note that either corner and either of the other edges could be used to test.
     * The situation is fully symmetrical.
     */
    bool bIsInteriorSeg = PolygonNodeTopology::isInteriorSegment(&nodePt, e00, e01, e10);
    bool bIsExterior = isInteriorOnRight ? ! bIsInteriorSeg : bIsInteriorSeg;
    return bIsExterior;
}


} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
