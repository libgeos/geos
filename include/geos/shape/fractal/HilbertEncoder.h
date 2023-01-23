/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/


#pragma once

#include <geos/export.h>
#include <geos/geom/Geometry.h>
#include <string>
#include <vector>
#include <cstdint>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
}

namespace geos {
namespace shape {   // geos.shape
namespace fractal { // geos.shape.fractal


class GEOS_DLL HilbertEncoder {

public:

    HilbertEncoder(uint32_t p_level, geom::Envelope& extent);
    uint32_t encode(const geom::Envelope* env);
    static void sort(std::vector<geom::Geometry*>& geoms);

    template<typename T>
    static geom::Envelope getEnvelope(T begin, T end) {
        geom::Envelope extent;
        for (auto it = begin; it != end; ++it) {
            const auto* g = *it;
            if (extent.isNull())
                extent = *(g->getEnvelopeInternal());
            else
                extent.expandToInclude(*(g->getEnvelopeInternal()));
        }

        return extent;
    }

    template<typename T>
    static void sort(T begin, T end) {
        auto extent = getEnvelope(begin, end);
        HilbertEncoder encoder(12, extent);
        HilbertComparator hilbertCompare(encoder);
        std::sort(begin, end, hilbertCompare);
    }

private:

    uint32_t level;
    double minx;
    double miny;
    double strideX;
    double strideY;

    struct HilbertComparator {

        HilbertEncoder& enc;

        HilbertComparator(HilbertEncoder& e)
            : enc(e) {};

        bool
        operator()(const geom::Geometry* a, const geom::Geometry* b)
        {
            return enc.encode(a->getEnvelopeInternal()) > enc.encode(b->getEnvelopeInternal());
        }
    };

};


} // namespace geos.shape.fractal
} // namespace geos.shape
} // namespace geos



