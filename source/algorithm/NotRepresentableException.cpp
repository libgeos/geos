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
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/geosAlgorithm.h"

namespace geos {

NotRepresentableException::NotRepresentableException(){
	setName("NotRepresentableException");
	setMessage("Projective point not representable on the Cartesian plane.");
}
NotRepresentableException::NotRepresentableException(string msg){
	setName("NotRepresentableException");
	setMessage(msg);
}
NotRepresentableException::~NotRepresentableException(){}

}

