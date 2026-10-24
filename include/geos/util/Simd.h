/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

namespace geos {
namespace util {
namespace simd {

/**
 * \brief Runtime selection of SIMD code paths.
 *
 * On x86-64 builds made with GCC or Clang, some inner loops used by
 * intersection operations and predicates (overlay ring clipping and clip
 * envelope computation, point-in-ring tests, monotone chain construction,
 * segment intersection orientation tests, ring orientation and repeated
 * point checks) have AVX-512 implementations that are selected at runtime
 * when the CPU supports AVX-512 F/DQ/VL/BW. The SIMD paths produce results
 * identical to the scalar paths.
 *
 * The AVX-512 paths can be disabled by setting the environment
 * variable `GEOS_DISABLE_AVX512` to a non-empty value other than `0`,
 * or at build time with the CMake option `GEOS_ENABLE_AVX512=OFF`.
 */

/// Returns `true` if GEOS was built with AVX-512 code paths and the
/// running CPU and operating system support them.
GEOS_DLL bool isAvx512Supported();

/// Returns `true` if the AVX-512 code paths are currently in use.
GEOS_DLL bool isAvx512Enabled();

/**
 * Enables or disables the AVX-512 code paths. Enabling has no effect
 * if they are not supported. Intended for testing and benchmarking;
 * it should not be called while other threads are using GEOS.
 *
 * @return the previous state
 */
GEOS_DLL bool setAvx512Enabled(bool enabled);

} // namespace geos::util::simd
} // namespace geos::util
} // namespace geos
