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

#include <geos/util/Interrupt.h>
#include <geos/util/Progress.h>

namespace geos {
namespace util { // geos::util

void ProgressFunctionIteration(ProgressFunction& progressFunction, size_t i,
                               size_t iterCount, size_t& iNotify,
                               size_t notificationInterval) {
    if (iNotify + 1 == notificationInterval) {
        if (!progressFunction(static_cast<double>(i + 1)/static_cast<double>(iterCount), nullptr)) {
            geos::util::Interrupt();
        }
        iNotify = 0;
    }
    else {
        ++iNotify;
    }
}

} // namespace geos::util
} // namespace geos

