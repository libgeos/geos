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

#include <geos/util/Interrupt.h>
#include <geos/util/GEOSException.h> // for inheritance

namespace geos {
namespace util { // geos::util

class GEOS_DLL InterruptedException: public GEOSException {
public:
	InterruptedException() :
		GEOSException("InterruptedException", "Interrupted!") {}
};

void
Interrupt::interrupt() {
  requested = false;
  throw InterruptedException();
}

bool Interrupt::requested = false;
Interrupt::Callback *Interrupt::callback = 0;

} // namespace geos::util
} // namespace geos

