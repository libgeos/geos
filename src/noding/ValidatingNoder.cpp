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

#include <geos/noding/FastNodingValidator.h>
#include <geos/noding/ValidatingNoder.h>
#include <geos/noding/SegmentString.h>


#include <memory> // for unique_ptr
#include <iostream>

namespace geos {
namespace noding { // geos.noding

ValidatingNoder::~ValidatingNoder() = default;

void
ValidatingNoder::computeNodes(const std::vector<SegmentString*>& segStrings)
{
    noder.computeNodes(segStrings);
    nodedSS = noder.getNodedSubstrings();
    validate();
}

void
ValidatingNoder::validate() const
{
    FastNodingValidator nv(nodedSS);
    try {
        nv.checkValid();
    }
    catch (const std::exception &) {
        throw;
    }
}

std::vector<std::unique_ptr<SegmentString>>
ValidatingNoder::getNodedSubstrings()
{
    return std::move(nodedSS);
}



} // namespace geos.noding
} // namespace geos
