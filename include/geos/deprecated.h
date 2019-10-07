/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019   Nicklas Larsson
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 * Based on parts of QtCore (qcompilerdetection.h and qglobal.h):
 **********************************************************************
 *
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (C) 2016 Intel Corporation.
 * Contact: https://www.qt.io/licensing/
 *
 * This file is part of the QtCore module of the Qt Toolkit.
 *
 * $QT_BEGIN_LICENSE:LGPL$
 * Commercial License Usage
 * Licensees holding valid commercial Qt licenses may use this file in
 * accordance with the commercial license agreement provided with the
 * Software or, alternatively, in accordance with the terms contained in
 * a written agreement between you and The Qt Company. For licensing terms
 * and conditions see https://www.qt.io/terms-conditions. For further
 * information use the contact form at https://www.qt.io/contact-us.
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU Lesser
 * General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPL3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or (at your option) the GNU General
 * Public license version 3 or any later version approved by the KDE Free
 * Qt Foundation. The licenses are as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-2.0.html and
 * https://www.gnu.org/licenses/gpl-3.0.html.
 *
 * $QT_END_LICENSE$
 *
 **********************************************************************/
#ifndef GEOS_DEPRECATED_H
#define GEOS_DEPRECATED_H

#include <geos/version.h>

/**
 * \file deprecated.h
 * \brief Definitions of GEOS deprecation warning macros.
 *
 * Deprecated declarations can be attributed with a basic deprecation macro
 * such as `GEOS_DEPRECATED`, which issues a compilation warning. Using a
 * versioned form e.g. `GEOS_DEPRECATED_VERSION_3_1` (in this case the
 * deprecation version is GEOS major version 3 and the minor version is 1) a
 * warning is issued only if `GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO` is set
 * to a version higher than the deprecation version.
 *
 * By default \ref GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO is set to
 * `0x020000`, e.g. <3.0, however it may be re-defined at compilation. The
 * version is defined as a hex in the form `0xMMmmpp`, where MM=major, mm=minor,
 * pp=patch).
 *
 * Deprecation macros with version checking (e.g. `GEOS_DEPRECATED_VERSION_3_1`)
 * only issues warnings and will not cause build failure.
 *
 * A *hard* deprecation can be achieved with a
 * \ref GEOS_DEPRECATED_SINCE(major, minor) conditional clause. In this case
 * the build will fail, if using a deprecated declaration predating
 * `GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO`.
 *
 * Defining \ref GEOS_NO_DEPRECATED_WARNINGS at compilation will suppress all
 * warnings and prevent eventual build failures. Alternatively, setting
 * `GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO=0x000000` will also prevent
 * build failure, but not suppress warnings.
 *
 * ### EXAMPLE
 *
 * GEOS_VERSION_HEX=0x030800 (3.8.0)
 *
 *     GEOS_DEPRECATED_VERSION_X_3_2
 *     void function1() {};
 *
 *     GEOS_DEPRECATED_VERSION_X_3_6
 *     void function2() {};
 *
 *     #if GEOS_DEPRECATED_SINCE(3, 6)
 *         GEOS_DEPRECATED size_t
 *         function3() {};
 *     #endif
 *
 *     #if GEOS_DEPRECATED_SINCE(3, 2)
 *         GEOS_DEPRECATED size_t
 *         function4() {};
 *     #endif
 *
 *
 * #### SCENARIO 1
 *
 * Default (`GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO=0x020000`, i.e. <3.0)
 *
 *  - function1(): deprecation warning issued
 *  - function2(): deprecation warning issued
 *  - function3(): deprecation warning issued
 *  - function4(): deprecation warning issued
 *
 * #### SCENARIO 2
 *
 *  \ref GEOS_NO_DEPRECATED_WARNINGS -- passed to compiler
 *
 * NO deprecation warnings issued or build error.
 *
 * #### SCENARIO 3
 *
 * `GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO=0x030200` (3.2.0) -- passed to compiler
 *
 *  - function1(): deprecation warning NOT issued
 *  - function2(): deprecation warning NOT issued
 *  - function3(): deprecation warning NOT issued
 *  - function4(): build error if used
 *
 * #### SCENARIO 4
 *
 * `GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO=0x030400` (3.4.0) -- passed to compiler
 *
 *  - function1(): deprecation warning issued
 *  - function2(): deprecation warning NOT issued
 *  - function3(): deprecation warning issued
 *  - function4(): build error if used
 *
 * #### SCENARIO 5
 *
 * `GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO=0x030600` (3.6.0) -- passed to compiler
 *
 *  - function1(): deprecation warning issued
 *  - function2(): deprecation warning issued
 *  - function3(): build error if used
 *  - function4(): build error if used
 */

/**
 * \name Internal macros
 *
 * \warning These are used for internally for compiler detection, do not use.
 * \cond
 */
#if defined(_MSC_VER)
#  ifdef __clang__
#    define G_CC_CLANG
#  endif
#  define G_CC_MSVC (_MSC_VER)

#  define G_DECL_DEPRECATED __declspec(deprecated)
#  ifndef G_CC_CLANG
#    define G_DECL_DEPRECATED_X(text) __declspec(deprecated(text))
#  endif

/* Intel C++ disguising as Visual C++: the `using' keyword avoids warnings */
#  if defined(__INTEL_COMPILER)
#    define G_DECL_VARIABLE_DEPRECATED
#  endif

/* ARM Realview Compiler Suite
   RVCT compiler also defines __EDG__ and __GNUC__ (if --gnu flag is given),
   so check for it before that */
#elif defined(__ARMCC__) || defined(__CC_ARM)
#  define G_DECL_DEPRECATED __attribute__ ((__deprecated__))

#elif defined(__GNUC__)
#  define G_CC_GNU          (__GNUC__ * 100 + __GNUC_MINOR__)
#  if defined(__MINGW32__)
#    define G_CC_MINGW
#  endif
#  if defined(__INTEL_COMPILER)
/* Intel C++ also masquerades as GCC */
#    define G_CC_INTEL      (__INTEL_COMPILER)
#    ifdef __clang__
/* Intel C++ masquerades as Clang masquerading as GCC */
#      define G_CC_CLANG
#    endif
#    define G_ASSUME_IMPL(expr)  __assume(expr)
#    define G_UNREACHABLE_IMPL() __builtin_unreachable()
#    if __INTEL_COMPILER >= 1300 && !defined(__APPLE__)
#      define G_DECL_DEPRECATED_X(text) __attribute__ ((__deprecated__(text)))
#    endif
#  elif defined(__clang__)
/* Clang also masquerades as GCC */
#    define G_CC_CLANG
#  else
/* Plain GCC */
#    if G_CC_GNU >= 405
#      define G_DECL_DEPRECATED_X(text) __attribute__ ((__deprecated__(text)))
#    endif
#  endif
#  define G_DECL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#if defined(G_CC_CLANG) && !defined(G_CC_INTEL) && !defined(G_CC_MSVC)
/* General C++ features */
#  if __has_feature(attribute_deprecated_with_message)
#    define G_DECL_DEPRECATED_X(text) __attribute__ ((__deprecated__(text)))
#  endif
#endif // G_CC_CLANG

#if defined(__cpp_enumerator_attributes) && __cpp_enumerator_attributes >= 201411
#if defined(G_CC_MSVC)
// Can't mark enum values as __declspec(deprecated) with MSVC, also can't move
// everything to [[deprecated]] because MSVC gives a compilation error when marking
// friend methods of a class as [[deprecated("text")]], breaking qstring.h
#  define G_DECL_ENUMERATOR_DEPRECATED [[deprecated]]
#  define G_DECL_ENUMERATOR_DEPRECATED_X(x) [[deprecated(x)]]
#else
#  define G_DECL_ENUMERATOR_DEPRECATED G_DECL_DEPRECATED
#  define G_DECL_ENUMERATOR_DEPRECATED_X(x) G_DECL_DEPRECATED_X(x)
#endif
#endif

/*
 * Fallback macros to certain compiler features
 */
#ifndef G_DECL_DEPRECATED
#  define G_DECL_DEPRECATED
#endif
#ifndef G_DECL_VARIABLE_DEPRECATED
#  define G_DECL_VARIABLE_DEPRECATED G_DECL_DEPRECATED
#endif
#ifndef G_DECL_DEPRECATED_X
#  define G_DECL_DEPRECATED_X(text) G_DECL_DEPRECATED
#endif
/** \endcond */ /* Internal macros */

/*---------------------------------------------------------------------------*/

/**
 * \name Basic deprecation macros
 *
 * These macros issues warnings, irrespective of deprecation version.
 *
 * They may be embedded in a \ref GEOS_DEPRECATED_SINCE conditional clause for
 * version checking.
 * \{
 */
#if !defined(GEOS_NO_DEPRECATED_WARNINGS)
#  undef GEOS_DEPRECATED
/**
 * \def GEOS_DEPRECATED
 * \brief Issues a simple function/member deprecation warning.
 *
 * Examples:
 * ```
 * GEOS_DEPRECATED void function();
 *
 * enum {x, y, z} GEOS_DEPRECATED;
 * ```
 */
#  define GEOS_DEPRECATED G_DECL_DEPRECATED
#  undef GEOS_DEPRECATED_X
/**
 * \def GEOS_DEPRECATED_X(text)
 * \brief Issues a function/member deprecation warning with a message.
 * \param text warning message
 */
#  define GEOS_DEPRECATED_X(text) G_DECL_DEPRECATED_X(text)
#  undef GEOS_DEPRECATED_VARIABLE
/**
 * \def GEOS_DEPRECATED_VARIABLE
 * \brief Issues a variable deprecation warning.
 *
 * Examples:
 * ```
 * enum {x, y, z, m GEOS_DEPRECATED_VARIABLE};
 *
 * struct {void a(int) GEOS_DEPRECATED_VARIABLE, void a_new(int), int b};
 * ```
 */
#  define GEOS_DEPRECATED_VARIABLE G_DECL_VARIABLE_DEPRECATED
#else
#  undef GEOS_DEPRECATED
#  define GEOS_DEPRECATED
#  undef GEOS_DEPRECATED_X
#  define GEOS_DEPRECATED_X(text)
#  undef GEOS_DEPRECATED_VARIABLE
#  define GEOS_DEPRECATED_VARIABLE
#endif
/**\}*/

/**
 * \name Version checking
 *
 * Facilitates deprecation warning version checking.
 * \{
 */
/**
 * \def GEOS_DEPRECATED_WARNINGS_SINCE
 * \brief Enables deprecated warnings from specified version of GEOS or later.
 *
 * Default value is present GEOS version (`GEOS_VERSION_HEX`); or if
 * specifically set, equal to `GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO`.
 */
#ifndef GEOS_DEPRECATED_WARNINGS_SINCE
# ifdef GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO
#  define GEOS_DEPRECATED_WARNINGS_SINCE GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO
# else
#  define GEOS_DEPRECATED_WARNINGS_SINCE GEOS_VERSION_HEX
# endif
#endif

#ifndef GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO
/**
 * \def GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO
 * \brief This macro can be defined in the project to disable functions
 * deprecated in a specified version of GEOS or any earlier version.
 *
 * The default version number is GEOS 2.x, meaning that functions deprecated
 * before GEOS 3.0 will not be included (if embedded in a
 * \ref GEOS_DEPRECATED_SINCE(major, minor) condition clause) and use of them
 * will lead to build failure.
 *
 * For instance, when using a future release of GEOS 3, set
 * `GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO=0x030400` to disable functions deprecated in
 * GEOS 3.4 and earlier. In any release, set
 * `GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO=0x000000` to enable all functions, including
 * the ones deprecated before GEOS 3.0.
 */
#define GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO GEOS_VERSION_CHECK(2, 0, 0)
#endif
/**
 * \def GEOS_DEPRECATED_SINCE
 * \brief Evaluates as true if the GEOS version is greater than the deprecation
 * point specified (with `GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO`).
 *
 * Use it to specify from which version of GEOS a function or class has been
 * deprecated.
 *
 * Example:
 * ```
 * #if GEOS_DEPRECATED_SINCE(3,4)
 *     GEOS_DEPRECATED
 *     void deprecatedFunction(); //function deprecated since GEOS 3.4
 * #endif
 * ```
 * If \ref GEOS_DEPRECATED_SINCE(major, minor) in the example evaluates as
 * false, i.e. 3.4 is less than `GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO`,
 * the `deprecatedFunction()` will be excluded from compilation. Usage of
 * the function will consequently lead to build failure. This can be considered
 * a hard deprecation.
 *
 * \param major,minor version since the function is deprecated
 *
 * \return true or false
 */
#ifdef GEOS_DEPRECATED
#define GEOS_DEPRECATED_SINCE(major, minor) (GEOS_VERSION_CHECK(major, minor, 0) > GEOS_DISABLE_DEPRECATED_BEFORE_OR_EQUAL_TO)
#else
#define GEOS_DEPRECATED_SINCE(major, minor) 0
#endif
/**\}*/

/**
 * \name Deprecation macros with version checking
 *
 * The macros named after patterns `GEOS_DEPRECATED_VERSION_[major]_[minor]` and
 * `GEOS_DEPRECATED_VERSION_X_[major]_[minor]` outputs a deprecation warning if
 * `GEOS_DEPRECATED_WARNINGS_SINCE` is equal or greater than the version
 * specified as major, minor.
 *
 * This makes it possible to make a soft deprecation of a function without
 * annoying a user who needs to stick at a specified minimum version and
 * therefore can't use the new function.
 *
 * \note For GEOS version 1.x and 2.x deprecations no minor version checks are
 * enabled.
 * \{
 */
#if GEOS_DEPRECATED_WARNINGS_SINCE >= GEOS_VERSION_CHECK(1, 0, 0)
# define GEOS_DEPRECATED_VERSION_X_1_0(text) GEOS_DEPRECATED_X(text)
# define GEOS_DEPRECATED_VERSION_1_0         GEOS_DEPRECATED
#else
# define GEOS_DEPRECATED_VERSION_X_1_0(text)
# define GEOS_DEPRECATED_VERSION_1_0
#endif

#if GEOS_DEPRECATED_WARNINGS_SINCE >= GEOS_VERSION_CHECK(2, 0, 0)
# define GEOS_DEPRECATED_VERSION_X_2_0(text) GEOS_DEPRECATED_X(text)
# define GEOS_DEPRECATED_VERSION_2_0         GEOS_DEPRECATED
#else
# define GEOS_DEPRECATED_VERSION_X_2_0(text)
# define GEOS_DEPRECATED_VERSION_2_0
#endif

#if GEOS_DEPRECATED_WARNINGS_SINCE >= GEOS_VERSION_CHECK(3, 0, 0)
# define GEOS_DEPRECATED_VERSION_X_3_0(text) GEOS_DEPRECATED_X(text)
# define GEOS_DEPRECATED_VERSION_3_0         GEOS_DEPRECATED
#else
# define GEOS_DEPRECATED_VERSION_X_3_0(text)
# define GEOS_DEPRECATED_VERSION_3_0
#endif

#if GEOS_DEPRECATED_WARNINGS_SINCE >= GEOS_VERSION_CHECK(3, 1, 0)
# define GEOS_DEPRECATED_VERSION_X_3_1(text) GEOS_DEPRECATED_X(text)
# define GEOS_DEPRECATED_VERSION_3_1         GEOS_DEPRECATED
#else
# define GEOS_DEPRECATED_VERSION_X_3_1(text)
# define GEOS_DEPRECATED_VERSION_3_1
#endif

#if GEOS_DEPRECATED_WARNINGS_SINCE >= GEOS_VERSION_CHECK(3, 2, 0)
# define GEOS_DEPRECATED_VERSION_X_3_2(text) GEOS_DEPRECATED_X(text)
# define GEOS_DEPRECATED_VERSION_3_2         GEOS_DEPRECATED
#else
# define GEOS_DEPRECATED_VERSION_X_3_2(text)
# define GEOS_DEPRECATED_VERSION_3_2
#endif

#if GEOS_DEPRECATED_WARNINGS_SINCE >= GEOS_VERSION_CHECK(3, 3, 0)
# define GEOS_DEPRECATED_VERSION_X_3_3(text) GEOS_DEPRECATED_X(text)
# define GEOS_DEPRECATED_VERSION_3_3         GEOS_DEPRECATED
#else
# define GEOS_DEPRECATED_VERSION_X_3_3(text)
# define GEOS_DEPRECATED_VERSION_3_3
#endif

#if GEOS_DEPRECATED_WARNINGS_SINCE >= GEOS_VERSION_CHECK(3, 4, 0)
# define GEOS_DEPRECATED_VERSION_X_3_4(text) GEOS_DEPRECATED_X(text)
# define GEOS_DEPRECATED_VERSION_3_4         GEOS_DEPRECATED
#else
# define GEOS_DEPRECATED_VERSION_X_3_4(text)
# define GEOS_DEPRECATED_VERSION_3_4
#endif

#if GEOS_DEPRECATED_WARNINGS_SINCE >= GEOS_VERSION_CHECK(3, 5, 0)
# define GEOS_DEPRECATED_VERSION_X_3_5(text) GEOS_DEPRECATED_X(text)
# define GEOS_DEPRECATED_VERSION_3_5         GEOS_DEPRECATED
#else
# define GEOS_DEPRECATED_VERSION_X_3_5(text)
# define GEOS_DEPRECATED_VERSION_3_5
#endif

#if GEOS_DEPRECATED_WARNINGS_SINCE >= GEOS_VERSION_CHECK(3, 6, 0)
# define GEOS_DEPRECATED_VERSION_X_3_6(text) GEOS_DEPRECATED_X(text)
# define GEOS_DEPRECATED_VERSION_3_6         GEOS_DEPRECATED
#else
# define GEOS_DEPRECATED_VERSION_X_3_6(text)
# define GEOS_DEPRECATED_VERSION_3_6
#endif

#if GEOS_DEPRECATED_WARNINGS_SINCE >= GEOS_VERSION_CHECK(3, 7, 0)
# define GEOS_DEPRECATED_VERSION_X_3_7(text) GEOS_DEPRECATED_X(text)
# define GEOS_DEPRECATED_VERSION_3_7         GEOS_DEPRECATED
#else
# define GEOS_DEPRECATED_VERSION_X_3_7(text)
# define GEOS_DEPRECATED_VERSION_3_7
#endif

#if GEOS_DEPRECATED_WARNINGS_SINCE >= GEOS_VERSION_CHECK(3, 8, 0)
# define GEOS_DEPRECATED_VERSION_X_3_8(text) GEOS_DEPRECATED_X(text)
# define GEOS_DEPRECATED_VERSION_3_8         GEOS_DEPRECATED
#else
# define GEOS_DEPRECATED_VERSION_X_3_8(text)
# define GEOS_DEPRECATED_VERSION_3_8
#endif
/**\}*/

/**
 * \def GEOS_NO_DEPRECATED_WARNINGS
 * \brief Compilation with this definition set, disables all warnings
 * and enables all deprecated functions.
 */
#ifdef _DOXYGEN_
#  define GEOS_NO_DEPRECATED_WARNINGS
#endif

#endif // GEOS_DEPRECATED_H
