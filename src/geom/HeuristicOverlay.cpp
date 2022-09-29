/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2013-2020 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: ORIGINAL WORK
 *
 **********************************************************************
 *
 * This file provides a single function, taking two
 * const Geometry pointers, applying a binary operator to them
 * and returning a result Geometry in an unique_ptr<>.
 *
 **********************************************************************/

#include <geos/geom/HeuristicOverlay.h>
#include <geos/operation/overlayng/OverlayNGRobust.h>

namespace geos {
namespace geom { // geos::geom

using operation::overlayng::OverlayNG;
using operation::overlayng::OverlayNGRobust;

std::unique_ptr<Geometry>
HeuristicOverlay(const Geometry* g0, const Geometry* g1, int opCode)
{
    std::unique_ptr<Geometry> ret;

/**************************************************************************/

/*
* overlayng::OverlayNGRobust carries out the following steps
*
* 1. Perform overlay operation using PrecisionModel(float).
*    If no exception return result.
* 2. Perform overlay operation using SnappingNoder(tolerance), starting
*    with a very very small tolerance and increasing it for 5 iterations.
*    The SnappingNoder moves only nodes that are within tolerance of
*    other nodes and lines, leaving all the rest undisturbed, for a very
*    clean result, if it manages to create one.
*    If a result is found with no exception, return.
* 3. Perform overlay operation using a PrecisionModel(scale), which
*    uses a SnapRoundingNoder. Every vertex will be noded to the snapping
*    grid, resulting in a modified geometry. The SnapRoundingNoder approach
*    reliably produces results, assuming valid inputs.
*
* Running overlayng::OverlayNGRobust at this stage should guarantee
* that none of the other heuristics are ever needed.
*/
        if (g0 == nullptr && g1 == nullptr) {
            return std::unique_ptr<Geometry>(nullptr);
        }
        else if (g0 == nullptr) {
            // Use a unary union for the one-parameter case, as the pairwise
            // union with one parameter is very intolerant to invalid
            // collections and multi-polygons.
            ret = OverlayNGRobust::Union(g1);
        }
        else if (g1 == nullptr) {
            // Use a unary union for the one-parameter case, as the pairwise
            // union with one parameter is very intolerant to invalid
            // collections and multi-polygons.
            ret = OverlayNGRobust::Union(g0);
        }
        else {
            ret = OverlayNGRobust::Overlay(g0, g1, opCode);
        }

        return ret;
}

} // namespace geos::geom
} // namespace geos
