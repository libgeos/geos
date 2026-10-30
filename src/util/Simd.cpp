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

#include <geos/util/Simd.h>

#include "SimdAvx512.h"

#include <cstdlib>
#include <cstring>

namespace geos {
namespace util {
namespace simd {

namespace {

bool
detectAvx512()
{
#if GEOS_SIMD_AVX512
    // __builtin_cpu_supports also checks that the OS saves the AVX-512 state
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx512f") &&
           __builtin_cpu_supports("avx512dq") &&
           __builtin_cpu_supports("avx512vl") &&
           __builtin_cpu_supports("avx512bw");
#else
    return false;
#endif
}

bool
disabledByEnvironment()
{
    const char* val = std::getenv("GEOS_DISABLE_AVX512");
    return val != nullptr && val[0] != '\0' && std::strcmp(val, "0") != 0;
}

const bool avx512Supported = detectAvx512();

} // anonymous namespace

namespace detail {

// Zero-initialized (scalar) until dynamic initialization runs, so any
// use during static initialization safely takes the scalar path.
std::atomic<bool> avx512Enabled{avx512Supported && !disabledByEnvironment()};

} // namespace geos::util::simd::detail

bool
isAvx512Supported()
{
    return avx512Supported;
}

bool
isAvx512Enabled()
{
    return detail::useAvx512();
}

bool
setAvx512Enabled(bool enabled)
{
    return detail::avx512Enabled.exchange(enabled && avx512Supported);
}

} // namespace geos::util::simd
} // namespace geos::util
} // namespace geos
