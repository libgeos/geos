/**********************************************************************
 * $Id: CLocalizer.cpp 2579 2009-06-15 14:03:52Z strk $
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

#include <geos/io/CLocalizer.h>

#include <string>
#include <clocale>

using namespace std;

namespace geos {
namespace io {

CLocalizer::CLocalizer()
{
    char* p = std::setlocale(LC_NUMERIC, NULL);
    if (0 != p)
    {
        saved_locale = p;
    }
    std::setlocale(LC_NUMERIC, "C");
}

CLocalizer::~CLocalizer()
{
    std::setlocale(LC_NUMERIC, saved_locale.c_str());
}

} // namespace geos.io
} // namespace geos

