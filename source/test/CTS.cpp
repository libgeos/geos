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
 * Revision 1.8  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


// CTS.cpp : Testing class
//

#include <iostream>

#include <geos/io.h>
#include <geos/geom.h>

using namespace std;
using namespace geos;

int main(int argc, char** argv)
{
	try {
	cout << "Start:" << endl;
	

	cout << "End" << endl;
	} catch (GEOSException *ge) {
		cout << ge->toString() << endl;
	}

	return 0;
}

