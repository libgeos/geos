/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/util/GEOSException.h>
#include <string>

namespace geos {
namespace util { // geos.util

#if 0
std::string GEOSException::toString() const
{
	std::string result(name);
	result+=": ";
	result+=txt;
	return result;
}
#endif

} // namespace geos.util
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2006/04/04 08:16:46  strk
 * Changed GEOSException hierarchy to be derived from std::runtime_exception.
 * Removed the GEOSException::toString redundant method (use ::what() instead)
 *
 **********************************************************************/

