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
 * Revision 1.10  2004/07/03 12:51:37  strk
 * Documentation cleanups for DoxyGen.
 *
 * Revision 1.9  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geosAlgorithm.h>

namespace geos {

/**
 * Indicates that a {@link HCoordinate} has been computed which is
 * not representable on the Cartesian plane.
 *
 * @version 1.4
 * @see HCoordinate
 */
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

