/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
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
