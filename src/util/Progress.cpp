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

ProgressFunction
ProgressFunction::subProgress(double from, double to) const {
    if (m_function.has_value()) {
        return ProgressFunction([from, to, this](double ratio, const char* msg)
        {
            m_function.value()(from + (to - from) * ratio, msg);
        });
    }

    return ProgressFunction();
}

}