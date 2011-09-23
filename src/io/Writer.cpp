/**********************************************************************
 * $Id: Writer.cpp 2579 2009-06-15 14:03:52Z strk $
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
 * Last port: ORIGINAL WORK to be used like java.io.Writer
 *
 **********************************************************************/

#include <geos/io/Writer.h>
#include <string>

using namespace std;

namespace geos {
namespace io { // geos.io

Writer::Writer()
{
	str="";
}

Writer::~Writer()
{
}

void
Writer::write(string txt)
{
	str+=txt;
}

string
Writer::toString()
{
	return str;
}

} // namespace geos.io
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.12  2006/06/01 11:49:36  strk
 * Reduced installed headers form geomgraph namespace
 *
 * Revision 1.11  2006/03/20 18:18:15  strk
 * io.h header split
 *
 * Revision 1.10  2006/03/06 15:23:14  strk
 * geos::io namespace
 *
 * Revision 1.9  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.8  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/
