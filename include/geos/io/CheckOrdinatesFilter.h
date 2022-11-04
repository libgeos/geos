/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 ISciences LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/io/OrdinateSet.h>

namespace geos {
namespace io {

class CheckOrdinatesFilter : public geom::CoordinateSequenceFilter {
public:
    CheckOrdinatesFilter(OrdinateSet checkOrdinateFlags) :
        m_checkFlags(checkOrdinateFlags),
        m_foundFlags(OrdinateSet::createXY())
    {}

    void filter_ro(const geom::CoordinateSequence& seq, std::size_t i) override {
        bool checkZ = m_checkFlags.hasZ() && !m_foundFlags.hasZ();
        bool checkM = m_checkFlags.hasM() && !m_foundFlags.hasM();

        if (checkZ || checkM) {
            seq.getAt(i, m_coord);

            if (checkZ && !std::isnan(m_coord.z)) {
                m_foundFlags.setZ(true);
            }

            if (checkM && !std::isnan(m_coord.m)) {
                m_foundFlags.setM(true);
            }
        }
    }

    bool isGeometryChanged() const override {
        return false;
    }

    bool isDone() const override {
        return m_checkFlags == m_foundFlags;
    }

    OrdinateSet getFoundOrdinates() const {
        return m_foundFlags;
    }

private:
    OrdinateSet m_checkFlags;
    OrdinateSet m_foundFlags;
    geom::CoordinateXYZM m_coord;
};

}
}
