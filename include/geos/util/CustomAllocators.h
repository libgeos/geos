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

#include <cstdlib> // size_t

namespace geos {
namespace util {
namespace CustomAllocators {

  typedef void * (*GEOSAllocator) (std::size_t size);
  GEOSAllocator setAllocator(GEOSAllocator nf);

  typedef void   (*GEOSFreer)    (void *ptr);
  GEOSFreer setFreer(GEOSFreer nf);

}
}
}

