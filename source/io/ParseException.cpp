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
 **********************************************************************
 * $Log$
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

#include <geos/io.h>
#include <sstream>
#include <string>

using namespace std;

namespace geos {

ParseException::ParseException(){
	GEOSException();
	setName("ParseException");
}

ParseException::ParseException(string msg){
	setName("ParseException");
	setMessage(msg);
}
ParseException::ParseException(string msg, string var){
	setName("ParseException");
	setMessage(msg+": '"+var+"'");
}
ParseException::ParseException(string msg, double num){
	setName("ParseException");
	ostringstream s;
	s<<msg<<": "<<num;
	setMessage(s.str());
}

} // namespace geos

