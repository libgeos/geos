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
#include <geos/noding/Noder.h>

#include <memory> // for unique_ptr

// Forward declarations
namespace geos {
namespace algorithm {
class LineIntersector;
}
namespace geom {
class Geometry;
}
}

namespace geos {
namespace noding { // geos.noding

class GEOS_DLL ValidatingNoder : public Noder {

private:

    std::vector<SegmentString*>* nodedSS;
    noding::Noder& noder;


public:

    ValidatingNoder(Noder& noderArg)
        : noder(noderArg)
        {}

    void computeNodes(std::vector<SegmentString*>* segStrings);

    void validate();

    std::vector<SegmentString*>* getNodedSubstrings();

};

} // namespace geos.noding
} // namespace geos

