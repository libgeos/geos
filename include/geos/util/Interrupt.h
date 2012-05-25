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

#ifndef GEOS_UTIL_INTERRUPT_H
#define GEOS_UTIL_INTERRUPT_H

#include <geos/export.h>

namespace geos {
namespace util { // geos::util

#define GEOS_CHECK_FOR_INTERRUPTS() geos::util::Interrupt::process()

class GEOS_DLL Interrupt {

public:

  static void request() { requested = true; }

  static bool check() { return requested; }

  static void process() {
    if ( requested ) {
      requested = false;
      interrupt();
    }
  }

  static void interrupt();

private:

  static bool requested;

};


} // namespace geos::util
} // namespace geos


#endif // GEOS_UTIL_INTERRUPT_H
