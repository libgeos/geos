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
 * Revision 1.5  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.4  2003/10/16 17:41:59  strk
 * Fixed a bug in GEOSException that prevented print of the 
 * type of exception thrown.
 *
 **********************************************************************/


#include "../headers/util.h"

namespace geos {

UnsupportedOperationException::UnsupportedOperationException(): GEOSException() {
	setName("UnsupportedOperationException");
}
UnsupportedOperationException::UnsupportedOperationException(string msg): GEOSException("UnsupportedOperationException", msg) {
	//setName("UnsupportedOperationException");
	//setMessage(msg);
}

UnsupportedOperationException::~UnsupportedOperationException(){}
}

