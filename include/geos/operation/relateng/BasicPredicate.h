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
#include <geos/operation/relateng/TopologyPredicate.h>
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


class GEOS_DLL BasicPredicate : public TopologyPredicate {
protected:
    using Envelope = geos::geom::Envelope;
    using Location = geos::geom::Location;

private:

    static constexpr int UNKNOWN = -1;
    static constexpr int FALSE = 0;
    static constexpr int TRUE = 1;

    int m_value = UNKNOWN;

    static bool isKnown(int val);

    static bool toBoolean(int val);

    static int toValue(bool val);


protected:

    /**
    * Updates the predicate value to the given state
    * if it is currently unknown.
    *
    * @param val the predicate value to update
    */
    void setValue(bool val);

    void setValue(int val);

    void setValueIf(bool val, bool cond);

    void require(bool cond);

    using TopologyPredicate::requireCovers;
    void requireCovers(const Envelope& a, const Envelope& b);


public:

    /**
    * Tests if two geometries intersect
    * based on an interaction at given locations.
    *
    * @param locA the location on geometry A
    * @param locB the location on geometry B
    * @return true if the geometries intersect
    */
    static bool isIntersection(Location locA, Location locB);

    std::string name() const override = 0;

    void finish() override = 0;

    bool isKnown() const override;

    bool value() const override;


};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

