/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: index/strtree/STRtree.java rev. 1.11
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
        bounds.expandToInclude(node->getBounds());
    }
}


/*public*/
void
SimpleSTRnode::toString(std::ostream& os, int level) const
{
    for (int i = 0; i < level; i++) {
        os << "  ";
    }
    os << bounds << std::endl;
    for (auto* node: childNodes) {
        node->toString(os, level+1);
    }
}


} // namespace geos.index.strtree
} // namespace geos.index
} // namespace geos


