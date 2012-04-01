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
 * Revision 1.3  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.2  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


/*////////////////////////////////////////////////////////////////////////////
 *  Project:
 *    Memory_and_Exception_Trace
 *
 * ///////////////////////////////////////////////////////////////////////////
 *  File:
 *    Stackwalker.h
 *
 *  Remarks:
 *
 *
 *  Note:
 *
 *
 *  Author:
 *    Jochen Kalmbach
 *
 *////////////////////////////////////////////////////////////////////////////

#ifndef __STACKWALKER_H__
#define __STACKWALKER_H__

// Only valid in the following environment: Intel platform, MS VC++ 5/6/7
#ifndef _X86_
#error Only INTEL envirnoments are supported!
#endif

// Only MS VC++ 5 to 7
#if (_MSC_VER < 1100) || (_MSC_VER > 1300)
//#warning Only MS VC++ 5/6/7 supported. Check if the '_CrtMemBlockHeader' has not changed with this compiler!
#endif

typedef enum eAllocCheckOutput
{
  ACOutput_Simple,
  ACOutput_Advanced,
  ACOutput_XML
};

// Make extern "C", so it will also work with normal C-Programs
#ifdef __cplusplus
extern "C" {
#endif
extern int InitAllocCheckWN(eAllocCheckOutput eOutput, LPCTSTR pszFilename, ULONG ulShowStackAtAlloc = 0);
extern int InitAllocCheck(eAllocCheckOutput eOutput = ACOutput_Simple, BOOL bSetUnhandledExeptionFilter = TRUE, ULONG ulShowStackAtAlloc = 0);  // will create the filename by itself

extern ULONG DeInitAllocCheck();

extern DWORD StackwalkFilter( EXCEPTION_POINTERS *ep, DWORD status, LPCTSTR pszLogFile);

#ifdef __cplusplus
}
#endif

#endif  // __STACKWALKER_H__
