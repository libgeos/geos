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
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <map>
#include <string>

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

        double m_d = 0.0;
        int m_i = 0;
        std::string m_s;
        ParamType m_type = ParamType::Double;

    public:

        ParamValue() {};
        ParamValue(double d);
        ParamValue(int i);
        ParamValue(std::string& s);
        ParamValue(const char* cstr);

        void setDouble(double d);
        void setInteger(int i);
        void setString(std::string& s);
        void setCString(const char* cstr);
        bool getDouble(double* d) const;
        bool getInteger(int* i) const;
        bool getCString(const char** str) const;

    };

    // where we actually store the parameters
    std::map<std::string, ParamValue> m_params;

    static void normalizeKey(std::string& str);
    bool haveKey(std::string& str) const;
    void clearEntry(std::string& key);
    const ParamValue* getValue(std::string& key) const;


public:

    Params() {};
    void setParam(const char* key, double d);
    void setParam(const char* key, int i);
    void setParam(const char* key, const char* s);
    bool getParamDouble(const char* key, double* d) const;
    bool getParamInteger(const char* key, int* i) const;
    bool getParamString(const char* key, const char** str) const;

};

} // namespace geos::util
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

