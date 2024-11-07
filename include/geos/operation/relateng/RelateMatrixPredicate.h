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


class GEOS_DLL RelateMatrixPredicate : public IMPredicate {
    using Envelope = geos::geom::Envelope;
    using Location = geos::geom::Location;
    using Dimension = geos::geom::Dimension;
    using IntersectionMatrix = geos::geom::IntersectionMatrix;

public:

    RelateMatrixPredicate() {};

    std::string name() const override {
        return "relateMatrix";
    };

    bool requireInteraction() const override {
        //-- ensure entire matrix is computed
        return false;
    };

    bool isDetermined() const override {
        //-- ensure entire matrix is computed
        return false;
    };

    bool valueIM() override {
        //-- indicates full matrix is being evaluated
        return false;
    };

    /**
    * Gets the current state of the IM matrix (which may only be partially complete).
    *
    * @return the IM matrix
    */
    std::unique_ptr<IntersectionMatrix> getIM() {
        return std::unique_ptr<IntersectionMatrix>(new IntersectionMatrix(intMatrix));
    }

};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

