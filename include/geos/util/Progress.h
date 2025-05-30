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

namespace geos {
namespace util { // geos::util

/** Signature of a progression function.
 *
 * Such function takes a progression ratio (between 0 and 1), and an optional
 * message.
 */
typedef std::function<void(double, const char*)> ProgressFunction;

/** Do progress function related processing for an iteration loop of iterCount iterations.
 *
 * This function will invoke (*progressFunction) every notificationInterval
 *iteration.
 *
 * This function will return, or throw a geos::util::InterruptedException.
 *
 * A typical use is:
 * \code
 * const size_t notificationInterval = std::max<size_t>(1, iterCount / 100);
 * for (size_t i = 0, iNotify = 0; i < iterCount; ++i) {
 *     // do something useful
 *     if (progressFunction) {
 *         geos::util::ProgressFunctionIteration(*progressFunction, i, iterCount, iNotify, notificationInterval);
 *     }
 * }
 * if (progressFunction) {
 *   (*progressFunction)(1.0, nullptr);
 * }
 * \endcode
 *
 * @param progressFunction Progress function
 * @param i Current index of loop iteration.
 * @param iterCount Total number of loop iteration.
 * @param iNotify Notification counter, updated by this function. Must be set by the caller (before the loop) to 0.
 * @param notificationInterval Notification interval (e.g. iterCount / 100).
 */
void ProgressFunctionIteration(ProgressFunction& progressFunction, size_t i, size_t iterCount, size_t& iNotify, size_t notificationInterval);

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
} // namespace geos

