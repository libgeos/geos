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

#include <geos/operation/relateng/RelatePredicate.h>
// #include <geos/constants.h>

#include <memory>


// using geos::geom::Envelope;
// using geos::geom::Location;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public static */
std::unique_ptr<BasicPredicate>
RelatePredicate::intersects()
{
    return std::unique_ptr<BasicPredicate>(new IntersectsPredicate());
}

/* public static */
std::unique_ptr<BasicPredicate>
RelatePredicate::disjoint()
{
    return std::unique_ptr<BasicPredicate>(new DisjointPredicate());
}

/* public static */
std::unique_ptr<IMPredicate>
RelatePredicate::contains()
{
    return std::unique_ptr<IMPredicate>(new ContainsPredicate());
}

/* public static */
std::unique_ptr<IMPredicate>
RelatePredicate::within()
{
    return std::unique_ptr<IMPredicate>(new WithinPredicate());
}

/* public static */
std::unique_ptr<IMPredicate>
RelatePredicate::covers()
{
    return std::unique_ptr<IMPredicate>(new CoversPredicate());
}

/* public static */
std::unique_ptr<IMPredicate>
RelatePredicate::coveredBy()
{
    return std::unique_ptr<IMPredicate>(new CoveredByPredicate());
}

/* public static */
std::unique_ptr<IMPredicate>
RelatePredicate::crosses()
{
    return std::unique_ptr<IMPredicate>(new CrossesPredicate());
}


/* public static */
std::unique_ptr<IMPredicate>
RelatePredicate::equalsTopo()
{
    return std::unique_ptr<IMPredicate>(new EqualsTopoPredicate());
}


/* public static */
std::unique_ptr<IMPredicate>
RelatePredicate::overlaps()
{
    return std::unique_ptr<IMPredicate>(new OverlapsPredicate());
}

/* public static */
std::unique_ptr<IMPredicate>
RelatePredicate::touches()
{
    return std::unique_ptr<IMPredicate>(new TouchesPredicate());
}

} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos




