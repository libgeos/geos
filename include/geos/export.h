/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009   Ragi Y. Burhum <ragi@burhum.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/
#pragma once

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || \
    defined( __BCPLUSPLUS__)  || defined( __MWERKS__)

#  if defined(GEOS_DLL_EXPORT)
#    define GEOS_DLL   __declspec(dllexport)
#  elif defined(GEOS_DLL_IMPORT)
#    define GEOS_DLL   extern __declspec(dllimport)
#  else
#    define GEOS_DLL
#  endif
#else
#  define GEOS_DLL
#endif


#if defined(_MSC_VER)
#  pragma warning(disable: 4251) // identifier : class type needs to have dll-interface to be used by clients of class type2
#endif


/**********************************************************************
 * Portability macros
 **********************************************************************/

#ifdef _MSC_VER
#include <sal.h>
#define GEOS_PRINTF_FORMAT _Printf_format_string_
#define GEOS_PRINTF_FORMAT_ATTR(format_param, dots_param) /**/
#elif __GNUC__
#define GEOS_PRINTF_FORMAT /**/
#define GEOS_PRINTF_FORMAT_ATTR(format_param, dots_param) \
  __attribute__((format(printf, format_param, dots_param)))
#else
#define GEOS_PRINTF_FORMAT /**/
#define GEOS_PRINTF_FORMAT_ATTR(format_param, dots_param) /**/
#endif
