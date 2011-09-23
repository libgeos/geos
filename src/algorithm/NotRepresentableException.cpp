/**********************************************************************
 * $Id: NotRepresentableException.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

#include <geos/algorithm/NotRepresentableException.h>

#include <string>

namespace geos {
namespace algorithm { // geos.algorithm

NotRepresentableException::NotRepresentableException()
	:
	GEOSException(
		"NotRepresentableException",
		"Projective point not representable on the Cartesian plane.")
		
{
}

NotRepresentableException::NotRepresentableException(std::string msg)
	:
	GEOSException(
		"NotRepresentableException", msg)
{
}

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.17  2006/04/04 08:16:46  strk
 * Changed GEOSException hierarchy to be derived from std::runtime_exception.
 * Removed the GEOSException::toString redundant method (use ::what() instead)
 *
 * Revision 1.16  2006/03/21 11:12:23  strk
 * Cleanups: headers inclusion and Log section
 *
 * Revision 1.15  2006/03/09 16:46:45  strk
 * geos::geom namespace definition, first pass at headers split
 **********************************************************************/

