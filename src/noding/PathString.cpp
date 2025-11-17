/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding/PathString.h>
#include <geos/util.h>

namespace geos::noding {

std::vector<PathString*>
PathString::toRawPointerVector(const std::vector<std::unique_ptr<PathString>> & pathStrings)
{
    std::vector<PathString*> ret(pathStrings.size());
    for (std::size_t i = 0; i < pathStrings.size(); i++) {
        ret[i] = pathStrings[i].get();
    }
    return ret;
}

}