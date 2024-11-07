/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Location.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/operation/relateng/IMPredicate.h>
#include <geos/export.h>

#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
    class Envelope;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation.
namespace relateng {  // geos.operation.relateng


class GEOS_DLL IMPatternMatcher : public IMPredicate {
    using Envelope = geos::geom::Envelope;
    using Location = geos::geom::Location;
    using Dimension = geos::geom::Dimension;
    using IntersectionMatrix = geos::geom::IntersectionMatrix;

private:

    std::string imPattern;
    IntersectionMatrix patternMatrix;

    static bool requireInteraction(const IntersectionMatrix& im);

    static bool isInteraction(int imDim);


public:

    IMPatternMatcher(std::string p_imPattern)
        : imPattern(p_imPattern)
        , patternMatrix(p_imPattern)
        {};

    std::string name() const override;

    using IMPredicate::init;
    void init(const Envelope& envA, const Envelope& envB) override;

    bool requireInteraction() const override;

    bool isDetermined() const override;

    bool valueIM() override;

    std::string toString() const;

    friend std::ostream& operator<<(std::ostream& os, const IMPatternMatcher& imp);

};


} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

