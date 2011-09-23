/**********************************************************************
 * $Id: CLocalizer.h 2958 2010-03-29 11:29:40Z mloskot $
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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

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

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace io
} // namespace geos

