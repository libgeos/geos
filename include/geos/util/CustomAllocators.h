/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2012 Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_UTIL_CUSTOMALLOCATORS_H
#define GEOS_UTIL_CUSTOMALLOCATORS_H

#include <geos/export.h>
#include <cstdlib> // size_t

namespace geos {
namespace util {
namespace CustomAllocators {

  typedef void* (*GEOSAllocator) (std::size_t size);
  GEOSAllocator GEOS_DLL setAllocator(GEOSAllocator nf);

  typedef void  (*GEOSDeallocator)(void *ptr);
  GEOSDeallocator GEOS_DLL setDeallocator(GEOSDeallocator nf);

}
}
}

#endif // GEOS_UTIL_CUSTOMALLOCATORS_H
