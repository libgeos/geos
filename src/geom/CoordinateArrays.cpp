/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <algorithm>

#include <geos/geom/CoordinateArrays.h>

namespace geos {
namespace geom { // geos::geom

/* public static */
bool
CoordinateArrays::hasRepeatedPoints(const std::vector<Coordinate>& coords)
{
    for (std::size_t i = 1; i < coords.size(); ++i) {
        if (coords.at(i-1).equals(coords.at(i))) {
            return true;
        }
    }
    return false;
}

/* public static */
void
CoordinateArrays::removeRepeatedPoints(std::vector<Coordinate>& coords)
{
    coords.erase(std::unique(coords.begin(), coords.end()), coords.end());
}

/* public static */
bool
CoordinateArrays::hasRepeatedOrInvalid(const std::vector<Coordinate>& coords)
{
    for (std::size_t i = 0; i < coords.size(); ++i) {

        if (! coords.at(i).isValid())
            return true;

        if (i > 0 && coords.at(i) == coords.at(i-i))
            return true;
    }
    return false;
}

/* public static */
void
CoordinateArrays::removeRepeatedOrInvalidPoints(std::vector<Coordinate>& coords)
{
    if (!hasRepeatedOrInvalid(coords)) return;

    removeRepeatedPoints(coords);

    for (auto it = coords.begin(); it != coords.end(); ) {
        if (!(*it).isValid()) {
            it = coords.erase(it);
        } else {
            ++it;
        }
    }
    return;
}

/* public static */
bool
CoordinateArrays::isRing(const std::vector<Coordinate>& pts) {
    if (pts.size() < 4)
        return false;
    if (!pts.at(0).equals2D(pts.at(pts.size()-1)))
        return false;
    return true;
}



} // namespace geos::geom
} // namespace geos
