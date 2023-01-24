/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>

namespace geos {
namespace geom {

template<typename C1, typename C2>
struct CommonCoordinateType {};

template<> struct CommonCoordinateType<CoordinateXY, CoordinateXY>     { using type = CoordinateXY;   };
template<> struct CommonCoordinateType<CoordinateXY, Coordinate>       { using type = Coordinate;     };
template<> struct CommonCoordinateType<CoordinateXY, CoordinateXYM>    { using type = CoordinateXYM;  };
template<> struct CommonCoordinateType<CoordinateXY, CoordinateXYZM>   { using type = CoordinateXYZM; };

template<> struct CommonCoordinateType<Coordinate, CoordinateXY>       { using type = Coordinate;     };
template<> struct CommonCoordinateType<Coordinate, Coordinate>         { using type = Coordinate;     };
template<> struct CommonCoordinateType<Coordinate, CoordinateXYM>      { using type = CoordinateXYZM; };
template<> struct CommonCoordinateType<Coordinate, CoordinateXYZM>     { using type = CoordinateXYZM; };

template<> struct CommonCoordinateType<CoordinateXYM, CoordinateXY>    { using type = CoordinateXYM;  };
template<> struct CommonCoordinateType<CoordinateXYM, Coordinate>      { using type = CoordinateXYZM; };
template<> struct CommonCoordinateType<CoordinateXYM, CoordinateXYM>   { using type = CoordinateXYM;  };
template<> struct CommonCoordinateType<CoordinateXYM, CoordinateXYZM>  { using type = CoordinateXYZM; };

template<> struct CommonCoordinateType<CoordinateXYZM, CoordinateXY>   { using type = CoordinateXYZM; };
template<> struct CommonCoordinateType<CoordinateXYZM, Coordinate>     { using type = CoordinateXYZM; };
template<> struct CommonCoordinateType<CoordinateXYZM, CoordinateXYM>  { using type = CoordinateXYZM; };
template<> struct CommonCoordinateType<CoordinateXYZM, CoordinateXYZM> { using type = CoordinateXYZM; };





}
}
