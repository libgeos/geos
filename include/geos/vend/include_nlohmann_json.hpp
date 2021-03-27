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
#ifndef GEOS_VEND_JSON
#define GEOS_VEND_JSON

#ifdef nlohmann
    #error "GEOS modifies the nlohmann define "
#endif    
#define nlohmann geos_nlohmann
#include "geos/vend/json.hpp"
#undef nlohmann

#endif