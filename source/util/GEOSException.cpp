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
 * Revision 1.7  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.6  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.5  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.4  2003/10/16 17:41:59  strk
 * Fixed a bug in GEOSException that prevented print of the type of 
 * exception thrown.
 *
 **********************************************************************/


#include <geos/util.h>

namespace geos {

GEOSException::GEOSException(){
	setName("GEOSException");
	setMessage("unknown error");
}
GEOSException::GEOSException(string msg){
	setName("GEOSException");
	setMessage(msg);
}
GEOSException::GEOSException(string nname,string msg){
	setName(nname);
	setMessage(msg);
}

string GEOSException::toString() const
{
	string result(name);
	result+=": ";
	result+=txt;
	return result;
}

void GEOSException::setName(string nname){
	name=nname;
}

void GEOSException::setMessage(string msg){
	txt=msg;
}
}

