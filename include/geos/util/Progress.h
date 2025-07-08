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
#include <optional>

#include <geos/export.h>

namespace geos::util {

/** A ProgressFunction is a wrapper around an optional user-defined callback,
 * taking a progress ratio (between 0 and 1) and an optional message. If not
 * provided with a callback at construction, the ProgressFunction will do nothing
 * when invoked.
 */
class GEOS_DLL ProgressFunction {
public:
    ProgressFunction() : m_function(std::nullopt) {}

    ProgressFunction(std::function<void(double, const char*)> f) : m_function(f) {}

    void operator()(double percentage, const char* message) const {
        if (m_function.has_value()) {
            m_function.value()(percentage, message);
        }
    }

   /** Create a ProgressFunction to manage a subset of the work reported
    * by this ProgressFunction.
    *
    * Sometimes when an operation wants to report progress, it actually
    * invokes several subprocesses which also take a ProgressFunction,
    * and it is desirable to map the progress of each sub operation into
    * a portion of 0.0 to 1.0 progress of the overall process. The scaled
    * progress function can be used for this.
    *
    * For each subsection a scaled progress function is created and
    * instead of passing the overall progress func down to the sub functions,
    * the scaled progress function is passed instead.
    *
    * @param from the completion fraction (0 to 1) to which 0.0 in the sub operation is mapped.
    * @param to the completion fraction (0 to 1) to which 1.0 is the sub operation is mapped.
    *
    * @return scaled progress function.
    */
    ProgressFunction subProgress(double from, double to) const;

    bool isSpecified() const {
        return m_function.has_value();
    }

private:
    std::optional<std::function<void(double, const char*)>> m_function;
};

static const ProgressFunction defaultProgress;

/** A ProgressContext manages the invocation of a ProgressFunction at a specified frequency.
 */
class GEOS_DLL ProgressContext {

public:
    /** Create a ProgressContext
     *
     * @param pCallback the function to call with progress updates
     * @param pIterCount the total number of expected iterations
     */
    ProgressContext(ProgressFunction pCallback, size_t pIterCount) :
        callback(pCallback),
        i(0),
        iterCount(pIterCount),
        notificationInterval(std::max<size_t>(1, iterCount / 100)),
        iNotify(0) {}

    /** Set the resolution of the progress reporting as a fraction from 0 to 1. By default,
     *  the progress function will be called for each 1% change in progress.
     */
    void setResolution(double res) {
        notificationInterval = std::max<size_t>(1, static_cast<std::size_t>(static_cast<double>(iterCount) * res));
    }

    /** Update the progress. This method should be called once per iteration. */
    void update() {
        if (callback.isSpecified()) {
            if (iNotify + 1 == notificationInterval) {
                callback(static_cast<double>(i + 1)/static_cast<double>(iterCount), nullptr);
                iNotify = 0;
            }
            else {
                ++iNotify;
            }
        }
    }

    void finish() const {
        callback(1.0, nullptr);
    }

private:
    const ProgressFunction callback;
    std::size_t i;
    std::size_t iterCount;
    std::size_t notificationInterval;
    std::size_t iNotify;
};

} // namespace geos::util

