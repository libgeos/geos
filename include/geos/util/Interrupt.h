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

/** Used to manage interruption requests and callbacks */
class GEOS_DLL Interrupt {

public:

  typedef void (Callback)(void *userdata);

  /** 
   * Request interruption of operations
   *
   * Operations will be terminated by a GEOSInterrupt
   * exception at first occasion.
   */
  static void request() { requested = true; }

  /** Cancel a pending interruption request */
  static void cancel() { requested = false; }

  /** Check if an interruption request is pending */
  static bool check() { return requested; }

  /** \brief
   * Register a callback that will be invoked
   * before checking for interruption requests.
   *
   * NOTE that interruption request checking may happen
   * frequently so any callback would better be quick.
   *
   * The callback can be used to call Interrupt::request()
   *
   */
  static void registerCallback(Callback *cb, void *arg) { callback = cb; callback_arg = arg; }

  /**
   * Invoke the callback, if any. Process pending interruption, if any.
   *
   */
  static void process() {
    if ( callback ) (*callback)(callback_arg);
    if ( requested ) {
      requested = false;
      interrupt();
    }
  }

  /* Perform the actual interruption (simply throw an exception) */
  static void interrupt();

private:

  /* Could these be portably stored in thread-specific space ? */

  static bool requested;

  static Callback *callback;

  static void *callback_arg;

};


} // namespace geos::util
} // namespace geos


#endif // GEOS_UTIL_INTERRUPT_H
