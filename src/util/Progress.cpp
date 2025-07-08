/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 Even Rouault <even.rouault@spatialys.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/util/Progress.h>

namespace geos::util {

ProgressFunction CreateScaledProgressFunction(double ratioMin, double ratioMax,
                                              ProgressFunction& progressFunction)
{
    return [ratioMin, ratioMax, &progressFunction](double ratio, const char* msg)
    {
        progressFunction(ratioMin + (ratioMax - ratioMin) * ratio, msg);
    };
}

} // namespace geos::util
