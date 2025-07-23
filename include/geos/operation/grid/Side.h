/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018-2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <iostream>

namespace geos::operation::grid {

enum class Side
{
    NONE,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

std::ostream&
operator<<(std::ostream& os, const Side& s);

}
