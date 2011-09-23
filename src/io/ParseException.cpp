/**********************************************************************
 * $Id: ParseException.cpp 2579 2009-06-15 14:03:52Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: io/ParseException.java rev. 1.13 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/io/ParseException.h>
#include <sstream>
#include <string>

using namespace std;

namespace geos {
namespace io { // geos.io

ParseException::ParseException()
	:
	GEOSException("ParseException", "")
{
}

ParseException::ParseException(const string& msg)
	:
	GEOSException("ParseException", msg)
{
}

ParseException::ParseException(const string& msg, const string& var)
	:
	GEOSException("ParseException", msg+": '"+var+"'")
{
}

ParseException::ParseException(const string& msg, double num)
	:
	GEOSException("ParseException", msg+": '"+stringify(num)+"'")
{
}

string
ParseException::stringify(double num)
{
	stringstream ss;
	ss<<num;
	return ss.str();
}

} // namespace geos.io
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2006/04/04 08:16:46  strk
 * Changed GEOSException hierarchy to be derived from std::runtime_exception.
 * Removed the GEOSException::toString redundant method (use ::what() instead)
 *
 * Revision 1.14  2006/03/20 18:18:15  strk
 * io.h header split
 *
 * Revision 1.13  2006/03/06 15:23:14  strk
 * geos::io namespace
 *
 * Revision 1.12  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.11  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.10  2005/01/28 10:01:09  strk
 * substituted sprintf calls with use of ostringstream
 *
 * Revision 1.9  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2004/05/17 12:36:39  strk
 * ParseException message made more readable
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/
