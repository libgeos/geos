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
 * Revision 1.7  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.6  2003/10/20 13:56:34  strk
 * fixed typo
 *
 **********************************************************************/


#include "../headers/util.h"

namespace geos {

AssertionFailedException::AssertionFailedException(){
	GEOSException();
	setName("AssertionFailedException");
}
AssertionFailedException::AssertionFailedException(string msg){
	setName("AssertionFailedException");
	setMessage(msg);
}

AssertionFailedException::~AssertionFailedException(){}
}

