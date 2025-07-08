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

#pragma once

#include <functional>

#include <geos/export.h>

namespace geos::util {

/** Signature of a progress function.
 *
 * Such function takes a progress ratio (between 0 and 1), and an optional
 * message.
 */
using ProgressFunction = std::function<void(double, const char*)>;

class GEOS_DLL Progress {

public:
    Progress(const ProgressFunction* pCallback, size_t pIterCount) :
        callback(pCallback),
        i(0),
        iterCount(pIterCount),
        notificationInterval(std::max<size_t>(1, iterCount / 100)),
        iNotify(0) {}

    void setResolution(double res) {
        notificationInterval = std::max<size_t>(1, static_cast<std::size_t>(static_cast<double>(iterCount) * res));
    }

    void update() {
        if (callback) {
            if (iNotify + 1 == notificationInterval) {
                (*callback)(static_cast<double>(i + 1)/static_cast<double>(iterCount), nullptr);
                iNotify = 0;
            }
            else {
                ++iNotify;
            }
        }
    }

    void finish() const {
        if (callback) {
            (*callback)(1.0, nullptr);
        }
    }

private:
    const ProgressFunction* callback;
    std::size_t i;
    std::size_t iterCount;
    std::size_t notificationInterval;
    std::size_t iNotify;
};

/** Create a scaled progress function.
 *
 * Sometimes when an operation wants to report progress, it actually
 * invokes several subprocesses which also take a ProgressFunction,
 * and it is desirable to map the progress of each sub operation into
 * a portion of 0.0 to 1.0 progress of the overall process. The scaled
 * progress function can be used for this.
 *
 * For each subsection a scaled progress function is created and
 * instead of passing the overall progress func down to the sub functions,
 * the scale progress function is passed instead.
 *
 * @param ratioMin the value to which 0.0 in the sub operation is mapped.
 * @param ratioMax the value to which 1.0 is the sub operation is mapped.
 * @param progressFunction the overall progress function.
 *
 * @return scaled progress function.
 */
ProgressFunction CreateScaledProgressFunction(double ratioMin, double ratioMax,
                                              ProgressFunction& progressFunction);


} // namespace geos::util

