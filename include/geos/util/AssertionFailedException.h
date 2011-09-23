/**********************************************************************
 * $Id: AssertionFailedException.h 2556 2009-06-06 22:22:28Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_UTIL_ASSERTIONFAILEDEXCEPTION_H
#define GEOS_UTIL_ASSERTIONFAILEDEXCEPTION_H

#include <geos/export.h>
#include <string>

#include <geos/util/GEOSException.h>

namespace geos {
namespace util { // geos.util

/** \class AssertionFailedException util.h geos.h
 * \brief Indicates a bug in GEOS code.
 */
class GEOS_DLL AssertionFailedException: public GEOSException {

public:

	AssertionFailedException()
		:
		GEOSException("AssertionFailedException", "")
	{}

	AssertionFailedException(const std::string& msg)
		:
		GEOSException("AssertionFailedException", msg)
	{}

	~AssertionFailedException() throw() {}
};

} // namespace geos.util
} // namespace geos


#endif // GEOS_UTIL_ASSERTIONFAILEDEXCEPTION_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
