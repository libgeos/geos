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
 **********************************************************************/

#include <string>

//#include <geos/geosAlgorithm.h>
#include <geos/algorithm/NotRepresentableException.h>

namespace geos {
namespace algorithm { // geos.algorithm

NotRepresentableException::NotRepresentableException(){
	setName("NotRepresentableException");
	setMessage("Projective point not representable on the Cartesian plane.");
}
NotRepresentableException::NotRepresentableException(std::string msg){
	setName("NotRepresentableException");
	setMessage(msg);
}

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2006/03/09 16:46:45  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.14  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.13  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.12  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.11  2004/07/07 10:29:54  strk
 * Adjusted exceptions documentation.
 *
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

