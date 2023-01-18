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


constexpr int
type_pair(const CoordinateType& typ1, const CoordinateType& typ2) {
    return (static_cast<int>(typ1) << 4) | static_cast<int>(typ2);
}

template<typename F, class... Args>
void binaryDispatch(const CoordinateSequence& seq1, const CoordinateSequence& seq2, F& fun, Args... args)
{
    using CoordinateXYZ = Coordinate;

    auto typ1 = seq1.getCoordinateType();
    auto typ2 = seq2.getCoordinateType();

    switch(type_pair(typ1, typ2)) {
        case type_pair(CoordinateType::XY, CoordinateType::XY):     fun.template operator()<CoordinateXY, CoordinateXY>(args...);   break;
        case type_pair(CoordinateType::XY, CoordinateType::XYZ):    fun.template operator()<CoordinateXY, CoordinateXYZ>(args...);  break;
        case type_pair(CoordinateType::XY, CoordinateType::XYM):    fun.template operator()<CoordinateXY, CoordinateXYM>(args...);  break;
        case type_pair(CoordinateType::XY, CoordinateType::XYZM):   fun.template operator()<CoordinateXY, CoordinateXYZM>(args...); break;

        case type_pair(CoordinateType::XYZ, CoordinateType::XY):    fun.template operator()<CoordinateXYZ, CoordinateXY>(args...);   break;
        case type_pair(CoordinateType::XYZ, CoordinateType::XYZ):   fun.template operator()<CoordinateXYZ, CoordinateXYZ>(args...);  break;
        case type_pair(CoordinateType::XYZ, CoordinateType::XYM):   fun.template operator()<CoordinateXYZ, CoordinateXYM>(args...);  break;
        case type_pair(CoordinateType::XYZ, CoordinateType::XYZM):  fun.template operator()<CoordinateXYZ, CoordinateXYZM>(args...); break;

        case type_pair(CoordinateType::XYM, CoordinateType::XY):    fun.template operator()<CoordinateXYM, CoordinateXY>(args...);   break;
        case type_pair(CoordinateType::XYM, CoordinateType::XYZ):   fun.template operator()<CoordinateXYM, CoordinateXYZ>(args...);  break;
        case type_pair(CoordinateType::XYM, CoordinateType::XYM):   fun.template operator()<CoordinateXYM, CoordinateXYM>(args...);  break;
        case type_pair(CoordinateType::XYM, CoordinateType::XYZM):  fun.template operator()<CoordinateXYM, CoordinateXYZM>(args...); break;

        case type_pair(CoordinateType::XYZM, CoordinateType::XY):   fun.template operator()<CoordinateXYZM, CoordinateXY>(args...);   break;
        case type_pair(CoordinateType::XYZM, CoordinateType::XYZ):  fun.template operator()<CoordinateXYZM, CoordinateXYZ>(args...);  break;
        case type_pair(CoordinateType::XYZM, CoordinateType::XYM):  fun.template operator()<CoordinateXYZM, CoordinateXYM>(args...);  break;
        case type_pair(CoordinateType::XYZM, CoordinateType::XYZM): fun.template operator()<CoordinateXYZM, CoordinateXYZM>(args...); break;
    }
}


}
}
