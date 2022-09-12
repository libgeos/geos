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

#pragma once

#include <string>

namespace geos {
namespace util {

bool endsWith(const std::string & s, const std::string & suffix);
bool endsWith(const std::string & s, char suffix);

bool startsWith(const std::string & s, const std::string & prefix);
bool startsWith(const std::string & s, char prefix);

}
}