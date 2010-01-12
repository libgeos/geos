/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2008 Sean Gillies
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: ORIGINAL WORK
 *
 **********************************************************************/

#include <geos/export.h>

#include <string>

namespace geos {
namespace io {

/**
 * \class CLocalizer io.h geos.h
 */
class GEOS_DLL CLocalizer
{
public:

    CLocalizer();
    ~CLocalizer();

private:

    std::string saved_locale;
};

} // namespace io
} // namespace geos

