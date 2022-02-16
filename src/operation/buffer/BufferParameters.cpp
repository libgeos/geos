/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009  Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/BufferParameters.java r278 (JTS-1.12)
 *
 **********************************************************************/

#include <cstdlib> // for std::abs()
#include <cmath> // for cos

#include <geos/constants.h>
#include <geos/operation/buffer/BufferParameters.h>


namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

// public static const
const double BufferParameters::DEFAULT_MITRE_LIMIT = 5.0;

// public
BufferParameters::BufferParameters()
    :
    quadrantSegments(DEFAULT_QUADRANT_SEGMENTS),
    endCapStyle(CAP_ROUND),
    joinStyle(JOIN_ROUND),
    mitreLimit(DEFAULT_MITRE_LIMIT),
    _isSingleSided(false)
{}

// public
BufferParameters::BufferParameters(int p_quadrantSegments)
    :
    quadrantSegments(DEFAULT_QUADRANT_SEGMENTS),
    endCapStyle(CAP_ROUND),
    joinStyle(JOIN_ROUND),
    mitreLimit(DEFAULT_MITRE_LIMIT),
    _isSingleSided(false)
{
    setQuadrantSegments(p_quadrantSegments);
}

// public
BufferParameters::BufferParameters(int p_quadrantSegments,
                                   EndCapStyle p_endCapStyle)
    :
    quadrantSegments(DEFAULT_QUADRANT_SEGMENTS),
    endCapStyle(CAP_ROUND),
    joinStyle(JOIN_ROUND),
    mitreLimit(DEFAULT_MITRE_LIMIT),
    _isSingleSided(false)
{
    setQuadrantSegments(p_quadrantSegments);
    setEndCapStyle(p_endCapStyle);
}

// public
BufferParameters::BufferParameters(int p_quadrantSegments,
                                   EndCapStyle p_endCapStyle,
                                   JoinStyle p_joinStyle,
                                   double p_mitreLimit)
    :
    quadrantSegments(DEFAULT_QUADRANT_SEGMENTS),
    endCapStyle(CAP_ROUND),
    joinStyle(JOIN_ROUND),
    mitreLimit(DEFAULT_MITRE_LIMIT),
    _isSingleSided(false)
{
    setQuadrantSegments(p_quadrantSegments);
    setEndCapStyle(p_endCapStyle);
    setJoinStyle(p_joinStyle);
    setMitreLimit(p_mitreLimit);
}

// public
void
BufferParameters::setQuadrantSegments(int quadSegs)
{
    quadrantSegments = quadSegs;
}

// public static
double
BufferParameters::bufferDistanceError(int quadSegs)
{
    double alpha = MATH_PI / 2.0 / quadSegs;
    return 1 - cos(alpha / 2.0);
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos
