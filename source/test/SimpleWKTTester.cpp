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
 * Revision 1.9  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


// CTS.cpp : Testing class
//

#include <iostream>
#include <fstream>

#include <geos/io.h>

using namespace std;
using namespace geos;

int main(int argc, char** argv)
{
    
	try {
		ofstream out("WKTOut");
		ifstream in("WKTIn");
		string instr;
		string outstr;
		WKTReader *r = new WKTReader(new GeometryFactory(new PrecisionModel(),10));
		WKTWriter *w=new WKTWriter();
		Geometry *g;

		cout << "Start Testing:" << endl;
		while(!in.eof()) {
			&getline(in,instr);
			if (instr!="") {
				g=r->read(instr);
				outstr=w->write(g);
				out << "----------" << endl;
				out << instr << endl;
				out << outstr << endl;
				out << "----------" << endl << endl;
			}
		}
		out.flush();
		out.close();
		cout << "End of Testing" << endl;

	} catch (GEOSException *ge) {
		cout << ge->toString() << endl;
	}

	return 0;
}

