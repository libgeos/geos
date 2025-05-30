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

/** Signature of a progression and cancel function.
 *
 * Such function takes a progression ratio (between 0 and 1), and an optional
 * message, and returns true if computation must continue, or false if it
 * must be interrupted.
 */
typedef std::function<bool(double, const char*)> ProgressFunction;

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

} // namespace geos::util
} // namespace geos

