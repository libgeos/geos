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

///
/// \brief The CoordinateSequences class provides utility methods to operate
///        on CoordinateSequences. Methods that do not benefit from access to the
/// 	   CoordinateSequence internals can be placed here.
///
class CoordinateSequences {

private:
    template<typename CoordinateType>
    static constexpr int
    type_pair(const CoordinateType& typ1, const CoordinateType& typ2) {
        return (static_cast<int>(typ1) << 4) | static_cast<int>(typ2);
    }

public:
    ///
    /// \brief binaryDispatch calls a functor template, explicitly providing the backing types of two CoordinateSequences. The
    /// CoordinateSequences are not provided to the functor as arguments but can be provided along with any other arugments
    /// through the `args` argument.
    template<typename F, class... Args>
    static void binaryDispatch(const CoordinateSequence& seq1, const CoordinateSequence& seq2, F& fun, Args... args)
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

};

}
}
