/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/simplify/RingHull.h>


namespace geos {
namespace geom {
class Envelope;
}
}

namespace geos {
namespace simplify { // geos::simplify


class RingHullIndex
{
    private:

        std::vector<const RingHull*> hulls;
        bool m_enabled;

    public:

        RingHullIndex()
        : m_enabled(true)
        {};

        bool enabled(void) const { return m_enabled; };
        void enabled(bool p_enabled) { m_enabled = p_enabled; };
        void add(const RingHull* ringHull);
        std::vector<const RingHull*> query(const geos::geom::Envelope& queryEnv) const;
        std::size_t size() const;


}; // RingHullIndex


} // geos::simplify
} // geos

