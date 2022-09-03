/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 ISciences LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/util/string.h>

namespace geos {
namespace util {

// https://stackoverflow.com/a/874160/2171894
bool endsWith(const std::string & s, const std::string & suffix) {
    if (s.length() < suffix.length()) {
        return false;
    }

    return s.compare(s.length() - suffix.length(),
                     suffix.length(),
                     suffix) == 0;
}

bool endsWith(const std::string & s, char suffix) {
    if (s.empty()) {
        return false;
    }

    return s[s.length() - 1] == suffix;
}

bool startsWith(const std::string & s, const std::string & prefix) {
    if (s.length() < prefix.length()) {
        return false;
    }

    auto cmp = s.compare(0, prefix.length(), prefix);
    return cmp == 0;
}

bool startsWith(const std::string & s, char prefix) {
    if (s.empty() == 0) {
        return false;
    }

    return s[0] == prefix;
}


}
}