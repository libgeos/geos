/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
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

#pragma once

#include <geos/export.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryCollection.h>
#include <vector>

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

/**
 * Extracts all the components of a collection, or just echoes back a
 * pointers to singletons.
 */
class GEOS_DLL GeometryLister {

public:

    /**
     * Extracts the components from a {@link Geometry}
     * and adds them to the provided container.
     *
     * Useful for iterating over components of a collection.
     *
     * @param geom the geometry from which to extract
     * @param lst the list to add the extracted elements to
     */
    static void
    list(const Geometry* geom, std::vector<const Geometry*>& lst)
    {
        if(geom->isCollection()) {
            GeometryLister::Lister lister(lst);
            geom->apply_ro(&lister);
        }
        else {
            lst.push_back(geom);
        }
    }

private:

    struct Lister : public GeometryFilter {

        /**
         * Constructs a filter with a list in which to store the elements found.
         *
         * @param comps the container to extract into (will push_back to it)
         */
        Lister(std::vector<const Geometry*>& p_geoms) : geoms(p_geoms) {}

        std::vector<const Geometry*>& geoms;

        void
        filter_ro(const Geometry* geom) override
        {
            if(!geom->isCollection()) {
                geoms.push_back(geom);
            }
        }

        // // Declare type as noncopyable
        // Lister(const Lister& other);
        // Lister& operator=(const Lister& rhs);
    };
};


} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

