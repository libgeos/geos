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
 *
 **********************************************************************/


// CTS.cpp : Testing class
//

#include <iostream>

#include "../headers/io.h"
#include "../headers/geom.h"

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

