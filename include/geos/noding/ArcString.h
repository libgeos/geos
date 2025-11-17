/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/CircularArc.h>
#include <geos/noding/PathString.h>

#include <vector>

namespace geos::noding {

/** \brief
 *  An interface for classes which represent a sequence of contiguous
 *  circular arcs, analogous to the SegmentString for contiguous line
 *  segments.
 */
class GEOS_DLL ArcString : public PathString {
public:
    explicit ArcString(std::vector<geom::CircularArc> arcs) : m_arcs(std::move(arcs)) {
    }

    ArcString(std::vector<geom::CircularArc> arcs, std::unique_ptr<geom::CoordinateSequence> seq, void* context)
        : m_arcs(std::move(arcs)),
          m_seq(std::move(seq)),
          m_context(context)
    {}

    std::size_t getSize() const override {
        return m_arcs.size();
    }

    double getLength() const override {
        double tot = 0;
        for (const auto &arc: m_arcs) {
            tot += arc.getLength();
        }
        return tot;
    }

    const geom::CircularArc &getArc(std::size_t i) const {
        return m_arcs[i];
    }

    auto begin() const {
        return m_arcs.begin();
    }

    auto end() const {
        return m_arcs.end();
    }

    std::unique_ptr<geom::CoordinateSequence> releaseCoordinates();

protected:
    std::vector<geom::CircularArc> m_arcs;
    std::unique_ptr<geom::CoordinateSequence> m_seq;
    void* m_context;
};

}