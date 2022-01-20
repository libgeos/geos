/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

//xie add for realse static memory 2003,10,06
namespace geos {
namespace io {

class GEOS_DLL Unload {
private:
    Unload(void) {}
    ~Unload(void) {}
public:
    static void Release();
};

}
}

