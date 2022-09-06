/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CoverageValidator.h>
#include <geos/coverage/CoveragePolygonValidator.h>

#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>


using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;


namespace geos {     // geos
namespace coverage { // geos.coverage

/* public static */
bool
CoverageValidator::isValid(std::vector<const Geometry*>& coverage)
{
    CoverageValidator v(coverage);
    return ! hasInvalidResult(v.validate());
}

/* public static */
bool
CoverageValidator::hasInvalidResult(const std::vector<std::unique_ptr<Geometry>>& validateResult)
{
    for (const auto& geom : validateResult) {
        if (geom != nullptr)
            return true;
    }
    return false;
}


/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageValidator::validate(std::vector<const Geometry*>& coverage)
{
    CoverageValidator v(coverage);
    return v.validate();
}


/* public static */
std::vector<std::unique_ptr<Geometry>>
CoverageValidator::validate(std::vector<const Geometry*>& coverage, double gapWidth)
{
    CoverageValidator v(coverage);
    v.setGapWidth(gapWidth);
    return v.validate();
}


/* public */
std::vector<std::unique_ptr<Geometry>>
CoverageValidator::validate()
{
    TemplateSTRtree<const Geometry*> index;
    std::vector<std::unique_ptr<Geometry>> invalidLines;
    for (auto* geom : m_coverage) {
        index.insert(geom);
        invalidLines.emplace_back(nullptr);
    }

    for (std::size_t i = 0; i < m_coverage.size(); i++) {
        const Geometry* geom = m_coverage[i];
        std::unique_ptr<Geometry> result = validate(geom, index);
        invalidLines[i].reset(result.release());
    }
    return invalidLines;
}

/* private */
std::unique_ptr<Geometry>
CoverageValidator::validate(const Geometry* targetGeom, TemplateSTRtree<const Geometry*>& index)
{
    Envelope queryEnv = *(targetGeom->getEnvelopeInternal());
    queryEnv.expandBy(m_gapWidth);

    // Query the index for nearby geometry and add to the list
    std::vector<const Geometry*> nearGeoms;
    auto visitor = [&nearGeoms](const Geometry* geom) {
        nearGeoms.push_back(geom);
    };
    index.query(queryEnv, visitor);
    //-- the target geometry is returned in the query, so must be removed from the set
    auto it = std::find(nearGeoms.begin(), nearGeoms.end(), targetGeom);
    if (it != nearGeoms.end()) {
        nearGeoms.erase(it);
    }

    // Geometry[] nearGeoms = GeometryFactory.toGeometryArray(nearGeoms);
    std::unique_ptr<Geometry> result = CoveragePolygonValidator::validate(targetGeom, nearGeoms, m_gapWidth);
    if (result->isEmpty())
        return nullptr;
    else
        return result;
}


} // namespace geos.coverage
} // namespace geos
