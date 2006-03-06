/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
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
		io::WKTReader *r = new io::WKTReader(new GeometryFactory(new PrecisionModel(),10));
		io::WKTWriter *w = new io::WKTWriter();
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

	} catch (const util::GEOSException& ge) {
		cout << ge.what() << endl;
	}

	return 0;
}

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/03/06 19:40:48  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.3  2006/03/06 15:23:14  strk
 * geos::io namespace
 *
 * Revision 1.2  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.1  2006/01/31 19:07:35  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
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

