/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Jared Erickson
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#if defined(__GNUC__)
#pragma GCC system_header
#endif

// to avoid any clash if GEOS users have another version of nlohmann/json.hpp
#define nlohmann geos_nlohmann
#include "nlohmann/json.hpp"