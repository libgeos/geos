/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding/ArcString.h>

namespace geos::noding {
 std::unique_ptr<geom::CoordinateSequence>
 ArcString::releaseCoordinates() {
  return std::move(m_seq);
 }
}