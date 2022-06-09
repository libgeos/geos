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
 **********************************************************************/

#include <geos/util/Params.h>

#include <sstream>

namespace geos {
namespace util { // geos.util

Params::ParamValue::ParamValue(double d)
    : m_d(d)
    , m_type(ParamType::Double) {}

Params::ParamValue::ParamValue(int i)
    : m_i(i)
    , m_type(ParamType::Integer) {}

Params::ParamValue::ParamValue(std::string& s)
    : m_s(s)
    , m_type(ParamType::String) {}

Params::ParamValue::ParamValue(const char* cstr)
    : m_s(cstr)
    , m_type(ParamType::String) {}


void
Params::ParamValue::setDouble(double d)
{
    m_d = d;
    m_type = ParamType::Double;
}

void
Params::ParamValue::setInteger(int i)
{
    m_i = i;
    m_type = ParamType::Integer;
}

void
Params::ParamValue::setString(std::string& s)
{
    m_s = s;
    m_type = ParamType::String;
}

void
Params::ParamValue::setCString(const char* cstr)
{
    m_s.assign(cstr);
    m_type = ParamType::String;
}

bool
Params::ParamValue::getDouble(double* d) const
{
    if (!d || m_type != ParamType::Double)
        return false;
    *d = m_d;
    return true;
}

bool
Params::ParamValue::getInteger(int* i) const
{
    if (!i || m_type != ParamType::Integer)
        return false;
    *i = m_i;
    return true;
}

bool
Params::ParamValue::getCString(const char** str) const
{
    if (!str || m_type != ParamType::String)
        return false;
    *str = m_s.c_str();
    return true;
}

void
Params::normalizeKey(std::string& str)
{
    std::stringstream ss;
    // lowercase and only alphanumeric
    for (char const &c: str) {
        if (std::isalnum(c)) {
            ss << std::tolower(c);
        }
    }
    str = ss.str();
}

bool
Params::haveKey(std::string& str) const
{
    if(m_params.find(str) == m_params.end())
        return false;
    else
        return true;
}

void
Params::clearEntry(std::string& key)
{
    auto search = m_params.find(key);
    if(search != m_params.end()) {
        m_params.erase(search);
    }
}

const Params::ParamValue*
Params::getValue(std::string& key) const
{
    auto search = m_params.find(key);
    if(search == m_params.end()) {
        return nullptr;
    }
    return &(search->second);
}

void
Params::setParam(const char* key, double d)
{
    std::string k(key);
    normalizeKey(k);
    clearEntry(k);
    m_params.emplace(k, d);
}

void
Params::setParam(const char* key, int i)
{
    std::string k(key);
    normalizeKey(k);
    clearEntry(k);
    m_params.emplace(k, i);
}

void
Params::setParam(const char* key, const char* s)
{
    std::string k(key);
    normalizeKey(k);
    clearEntry(k);
    m_params.emplace(k, s);
}

bool
Params::getParamDouble(const char* key, double* d) const
{
    std::string k(key);
    normalizeKey(k);
    const ParamValue* val = getValue(k);
    if (!val) return false;
    return val->getDouble(d);
}

bool
Params::getParamInteger(const char* key, int* i) const
{
    std::string k(key);
    normalizeKey(k);
    const ParamValue* val = getValue(k);
    if (!val) return false;
    return val->getInteger(i);
}

bool
Params::getParamString(const char* key, const char** str) const
{
    std::string k(key);
    normalizeKey(k);
    const ParamValue* val = getValue(k);
    if (!val) return false;
    return val->getCString(str);
}


} // namespace geos.util
} // namespace geos

