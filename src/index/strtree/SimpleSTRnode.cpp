/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/index/strtree/SimpleSTRnode.h>
#include <geos/geom/Envelope.h>


using namespace geos::geom;

namespace geos {
namespace index { // geos.index
namespace strtree { // geos.index.strtree


void
SimpleSTRnode::computeBounds()
{
    for (auto* node: childNodes) {
        bounds.expandToInclude(node->getEnvelope());
    }
}

/*public*/
void
SimpleSTRnode::toString(std::ostream& os, int indentLevel) const
{
    for (int i = 0; i < indentLevel; i++) {
        os << "  ";
    }
    os << bounds << std::endl;
    for (auto* node: childNodes) {
        node->toString(os, indentLevel+1);
    }
}


} // namespace geos.index.strtree
} // namespace geos.index
} // namespace geos


