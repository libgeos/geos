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
 * Revision 1.8  2004/05/17 12:36:39  strk
 * ParseException message made more readable
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/io.h"
#include "stdio.h"

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
	string ntxt="";
	ntxt+=msg;
	ntxt+=": ";
	char buffer[255];
	sprintf(buffer,"%g",num);
	ntxt.append(buffer);
	setMessage(ntxt);
}

ParseException::~ParseException(){}
}

