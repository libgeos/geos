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

#include <geos/util/CustomAllocators.h>

#include <new>
#include <iostream>
#include <cstdlib>

namespace geos {
namespace util {
namespace CustomAllocators {

GEOSAllocator geos_alloc = std::malloc;
GEOSFreer geos_free = std::free;

GEOSAllocator
setAllocator(GEOSAllocator nf)
{
  GEOSAllocator of = geos_alloc;
  geos_alloc = nf;
  return of;
}

GEOSFreer
setFreer(GEOSFreer nf)
{
  GEOSFreer of = geos_free;
  geos_free = nf;
  return of;
}

}
}
}

void*
operator new (std::size_t size, const std::nothrow_t&) throw () {
        //std::cout << "...new(" << size << ") called" << std::endl;
        return geos::util::CustomAllocators::geos_alloc(size);
}

void operator delete (void *ptr) throw () {
        //std::cout << "...delete(" << ptr << ") called" << std::endl;
        if ( ptr ) geos::util::CustomAllocators::geos_free(ptr);
}

