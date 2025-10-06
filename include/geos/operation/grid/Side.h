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

inline std::ostream&
operator<<(std::ostream& os, const Side& s)
{
    switch (s) {
        case Side::NONE:
            os << "none";
            return os;
        case Side::LEFT:
            os << "left";
            return os;
        case Side::RIGHT:
            os << "right";
            return os;
        case Side::TOP:
            os << "top";
            return os;
        case Side::BOTTOM:
            os << "bottom";
            return os;
    }

    return os; // unreachable statement needed for -Werror=return-type
}

}
