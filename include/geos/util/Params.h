/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <map>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

namespace geos {
namespace util { // geos::util


enum class ParamType {
    Double, Integer, String
};

/** \brief
 * Stores values that can be fed to the functions, for use in the
 * CAPI primarily to expose functionality with a lot of parametric
 * options.
 *
 */
class GEOS_DLL Params {

private:

    class ParamValue {

    private:

        union {
            double d;
            int i;
            std::string s;
        } m_val;

        ParamType m_type;

    public:

        void setDouble(double d) {
            m_val.d = d;
            m_type = ParamType::Double;
        };

        void setInteger(int i) {
            m_val.i = i;
            m_type = ParamType::Integer;
        };

        void setString(std::string& s) {
            m_val.s = s;
            m_type = ParamType::String;
        };

        void setString(const char* cstr) {
            std::string s(cstr);
            m_val.s = s;
            m_type = ParamType::String;
        };


    };



public:

    int get

};

} // namespace geos::util
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

