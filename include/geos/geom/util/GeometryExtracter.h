/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2011 Sandro Santilli <strk@keybit.net>
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ********************************************************************** 
 *
 * Last port: geom/util/GeometryExtracter.java r320 (JTS-1.12)
 *
 **********************************************************************/

#ifndef GEOS_GEOM_UTIL_POINTEXTRACTER_H
#define GEOS_GEOM_UTIL_POINTEXTRACTER_H

#include <geos/export.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/platform.h>
#include <vector>

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

/**
 * Extracts the components of a given type from a {@link Geometry}.
 */
class GeometryExtracter {

public:

  /**
   * Extracts the components of type <tt>clz</tt> from a {@link Geometry}
   * and adds them to the provided container.
   *
   * @param geom the geometry from which to extract
   * @param list the list to add the extracted elements to
   */
  template <class to, class container>
  static void extract(const Geometry& geom, container& lst)
  {
    if ( const to* c = dynamic_cast<const to*>(&geom) )
    {
      lst.push_back(c);
    }
    else if ( const GeometryCollection* c =
                   dynamic_cast<const GeometryCollection*>(&geom) )
    {
      GeometryExtracter::Extracter<to, container> extracter(lst);
      c->apply_ro(&extracter);
    }
  }

private:

  template <class TO, class CO>
  struct Extracter: public GeometryFilter {

    /**
     * Constructs a filter with a list in which to store the elements found.
     *
     * @param comps the container to extract into (will push_back to it)
     */
    Extracter(CO& comps) : _comps(comps) {}

    CO& _comps;

    void filter_ro(const Geometry* geom)
    {
      if ( const TO* c = dynamic_cast<const TO*>(geom) ) {
        _comps.push_back(c);
      }
    }

  };

  // Declare type as noncopyable
  GeometryExtracter(const GeometryExtracter& other);
  GeometryExtracter& operator=(const GeometryExtracter& rhs);
};

} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

#endif
