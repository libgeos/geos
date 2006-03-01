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

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <functional>

#include <geos/util.h>
#include <geos/geomgraph.h>
#include <geos/io.h>
#include <geos/opRelate.h>
#include <geos/opPolygonize.h>
#include <geos/opLinemerge.h>
#include <geos/profiler.h>
#include "markup/MarkupSTL.h"
#include <geos/unload.h>
#include "XMLTester.h"

//#include "util.h"
//#include "geomgraph.h"
//#include "io.h"
//#include "opRelate.h"
//#include "MarkupSTL.h"

#ifdef _MSC_VER
#include <windows.h>
#include "Stackwalker.h"
#endif

using namespace std;
using namespace geos;
using namespace geos::operation::polygonize;
using namespace geos::operation::linemerge;

//using geos::Polygon; // for mingw providing a Polygon global function

template <int (&F)(int)> unsigned char safe_ctype(unsigned char c) { return F(c); }

void
tolower(string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), safe_ctype<std::tolower>);
}

XMLTester::XMLTester()
	:
	gA(0),
	gB(0),
	gT(0),
	pm(0),
	factory(0),
	r(0),
	w(0),
	br(0),
	test_predicates(0),
	failed(0),
	succeeded(0),
	caseCount(0),
	testCount(0),
	testFileCount(0),
	totalTestCount(0),
	curr_file(NULL)
{
	setVerbosityLevel(0);
}

int
XMLTester::setVerbosityLevel(int value)
{
	int old_value=verbose;

	verbose=value;

	return old_value;
}


void
XMLTester::run(const string &source)
{
	curr_file=&source;

	++testFileCount;

	caseCount=0;

	xml.Load(source.c_str());

	xml.ResetPos();
	xml.FindElem("run");
	xml.FindChildElem("precisionModel");
	parsePrecisionModel();
	while (xml.FindChildElem("case")) {
		try {
			parseCase();
		} catch (const GEOSException& exc) {
			cerr<<exc.toString()<<endl;
		}
	}
}

void 
XMLTester::resultSummary(ostream &os) const
{
	os<<"Files: "<<testFileCount<<endl;
	os<<"Tests: "<<totalTestCount<<endl;
	os<<"Failed: "<<failed<<endl;
	os<<"Succeeded: "<<succeeded<<endl;
}

void 
XMLTester::resetCounters() 
{
	testFileCount=totalTestCount=failed=succeeded=0;
}


void
XMLTester::parsePrecisionModel()
{
	string precisionModel;

	/* This does not seem to work... */
	//precisionModel=xml.GetChildAttrib("type");
	string scaleStr=xml.GetChildAttrib("scale");

	if ( pm ) delete pm;

	if ( scaleStr == "" ) {
		pm=new PrecisionModel();
	} else {
		char* stopstring;
		//string scaleStr=xml.GetChildAttrib("scale");
		string offsetXStr=xml.GetChildAttrib("offsetx");
		string offsetYStr=xml.GetChildAttrib("offsety");

		double scale=strtod(scaleStr.c_str(),&stopstring);
		double offsetX=strtod(offsetXStr.c_str(),&stopstring);
		double offsetY=strtod(offsetYStr.c_str(),&stopstring);
		pm=new PrecisionModel(scale,offsetX,offsetY);
	}

	if (verbose)
	{
		cout << *curr_file <<": run: Precision Model: " << pm->toString() <<endl;
	}

	if ( factory ) delete factory;
	factory = new GeometryFactory(pm);

	if ( r ) delete r;
	r=new WKTReader(factory);

	if ( w ) delete w;
	w=new WKTWriter();

	if ( br ) delete br;
	br=new WKBReader(*factory);
}

/**
 * Parse WKT or HEXWKB
 */
Geometry *
XMLTester::parseGeometry(const string &in)
{
	stringstream is(in, ios_base::in);
	char first_char;

	// Remove leading spaces
	while (is.get(first_char) && std::isspace(first_char));
	is.unget();

	switch (first_char)
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			return br->readHEX(is);
		default:
			return r->read(in);
	}
}

string 
XMLTester::trimBlanks(const string &in)
{
	string out;
	string::size_type pos = in.find_first_not_of(" \t\n\r");
	if (pos!=string::npos) out=in.substr(pos);
	pos = out.find_last_not_of(" \t\n\r");
	if (pos!=string::npos) out=out.substr(0, pos+1);
	return out;
}

void
XMLTester::parseCase()
{
	string geomAin;
	string geomBin;
	string thrownException;

	gA=NULL;
	gB=NULL;

	xml.IntoElem();
	xml.FindChildElem("desc");
	curr_case_desc=trimBlanks(xml.GetChildData());

	try {
		xml.FindChildElem("a");
		geomAin=xml.GetChildData();
		geomAin=trimBlanks(geomAin);
		gA=parseGeometry(geomAin);

		if ( xml.FindChildElem("b") )
		{
			geomBin=xml.GetChildData();
			geomBin=trimBlanks(geomBin);
			gB=parseGeometry(geomBin);
		}
	}
	catch (const GEOSException& ex) {
		thrownException = ex.toString();
	}
	catch (const std::exception &e) {
		thrownException = e.what();
	}
	catch (...) {
		thrownException = "Unknown exception";
	}

	if ( thrownException != "" )
	{
		cout << *curr_file <<":";
		cout << " case" << caseCount << ":";
		cout << " skipped ("<<thrownException<<")."<<endl;
		return;
	}

	++caseCount;
	testCount=0;
	while(xml.FindChildElem("test")) {
		parseTest();
	}
	totalTestCount+=testCount;
		
	xml.OutOfElem();
	delete gA;
	delete gB;
}

void
XMLTester::parseTest()
{
	int success=0; // no success by default
	string opName;
	string opArg1;
	string opArg2;
	string opArg3;
	string opRes;
	//string opSig;

	++testCount;

	xml.IntoElem();
	xml.FindChildElem("op");
	opName=xml.GetChildAttrib("name");
	opArg1=xml.GetChildAttrib("arg1");
	opArg2=xml.GetChildAttrib("arg2");
	opArg3=xml.GetChildAttrib("arg3");
	//opSig=xml.GetChildAttrib("arg3");
	opRes=xml.GetChildData();

	// trim blanks
	opRes=trimBlanks(opRes);
	opName=trimBlanks(opName);
	tolower(opName);

	string opSig="";

	if ( opArg1 != "" ) opSig=opArg1;
	if ( opArg2 != "" ) {
		if ( opSig != "" ) opSig += ", ";
		opSig += opArg2;
	}
	if ( opArg3 != "" ) {
		if ( opSig != "" ) opSig += ", ";
		opSig += opArg3;
	}


	string actual_result="NONE";

	// expected_result will be modified by specific tests
	// if needed (geometry normalization, for example)
	string expected_result=opRes;

	try
	{

		Profile profile("op");

		if (opName=="relate")
		{
			IntersectionMatrix *im=gA->relate(gB);

			if (im->matches(opArg3)) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;
			
			delete im;

		}

		else if (opName=="isvalid")
		{
			Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) {
				gT=gB;
			} 

			if (gT->isValid()) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;

		}

		else if (opName=="intersection")
		{
			Geometry *gRes=NULL;
			Geometry *gRealRes=NULL;
			gRes=r->read(opRes);
			gRes->normalize();
			try {
				gRealRes=gA->intersection(gB);
				gRealRes->normalize();
			} catch (...) {
				delete gRealRes;
				delete gRes;
				throw;
			}

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRes;
			delete gRealRes;
		}

		else if (opName=="union")
		{
			Geometry *gRes=r->read(opRes);
			gRes->normalize();
			Geometry *gRealRes=gA->Union(gB);
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRes;
			delete gRealRes;
		}

		else if (opName=="difference")
		{
			Geometry *gRes=r->read(opRes);
			gRes->normalize();
			Geometry *gRealRes=gA->difference(gB);
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRes;
			delete gRealRes;
		}

		else if (opName=="symdifference")
		{
			Geometry *gRes=r->read(opRes);
			gRes->normalize();
			Geometry *gRealRes=gA->symDifference(gB);
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRes;
			delete gRealRes;
		}

		else if (opName=="intersects")
		{
			if (gA->intersects(gB)) actual_result="true";
			else actual_result="false";
			
			if (actual_result==opRes) success=1;
		}

		else if (opName=="getboundary")
		{
			Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			Geometry *gRes=r->read(opRes);
			gRes->normalize();
			Geometry *gRealRes=gT->getBoundary();
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRes;
			delete gRealRes;
		}

		else if (opName=="getcentroid")
		{
			Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			Geometry *gRes=r->read(opRes);
			gRes->normalize();

			Geometry *gRealRes=gT->getCentroid();
			if ( gRealRes ) gRealRes->normalize();
			else gRealRes = factory->createGeometryCollection();
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRes;
			delete gRealRes;
		}

		else if (opName=="issimple")
		{
			Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			if (gT->isSimple()) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;

		}

		else if (opName=="convexhull")
		{
			Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			Geometry *gRes=r->read(opRes);
			gRes->normalize();
			Geometry *gRealRes=gT->convexHull();
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRes;
			delete gRealRes;
		}

		else if (opName=="buffer")
		{
			Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			Geometry *gRes=r->read(opRes);
			gRes->normalize();
			profile.start();
			Geometry *gRealRes;
			if ( opArg2 != "" ) {
				gRealRes=gT->buffer(atof(opArg3.c_str()), atoi(opArg2.c_str()));
			} else {
				gRealRes=gT->buffer(atof(opArg3.c_str()));
			}
			profile.stop();
			gRealRes->normalize();

			/// Allow for slightly different representations
			if (gRes->equalsExact(gRealRes, 0.00000000001)==0) success=1;
			//if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRealRes;
			delete gRes;
		}

		else if (opName=="getinteriorpoint")
		{
			Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			Geometry *gRes=r->read(opRes);
			gRes->normalize();

			Geometry *gRealRes=gT->getInteriorPoint();
			if ( gRealRes ) gRealRes->normalize();
			else gRealRes = factory->createGeometryCollection();

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRes;
			delete gRealRes;
		}

		else if (opName=="iswithindistance")
		{
			float dist=atof(opArg3.c_str());
			if (gA->isWithinDistance(gB, dist)) {
				actual_result="true";
			} else {
				actual_result="false";
			}

			if (actual_result==opRes) success=1;

		}

		else if (opName=="polygonize")
		{
			Geometry *gRes=NULL;
			Geometry *gRealRes=NULL;
			gRes=r->read(opRes);
			gRes->normalize();
			try {
				Polygonizer plgnzr;
				plgnzr.add(gA);
				vector<geos::Polygon *>*polys = plgnzr.getPolygons();
				vector<Geometry *>*newgeoms = new vector<Geometry *>;
				for (unsigned int i=0; i<polys->size(); i++)
					newgeoms->push_back((*polys)[i]);
				delete polys;
				gRealRes=factory->createGeometryCollection(newgeoms);
				gRealRes->normalize();
			} catch (...) {
				delete gRealRes;
				delete gRes;
				throw;
			}

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRealRes;
			delete gRes;
		}

		else if (opName=="linemerge")
		{
			Geometry *gRes=NULL;
			Geometry *gRealRes=NULL;
			gRes=r->read(opRes);
			gRes->normalize();

			Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			try {
				LineMerger merger;
				merger.add(gT);
				vector<geos::LineString *>*lines = merger.getMergedLineStrings();
				vector<Geometry *>*newgeoms = new vector<Geometry *>(lines->begin(),
						lines->end());
				delete lines;
				gRealRes=factory->createGeometryCollection(newgeoms);
				gRealRes->normalize();
			} catch (...) {
				delete gRealRes;
				delete gRes;
				throw;
			}

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRealRes;
			delete gRes;
		}

		else
		{
			if ( 1 ) // verbose 
			{
				cerr << *curr_file <<":";
				cerr << " case" << caseCount << ":";
				cerr << " test" << testCount <<
					": " << opName << "(" << opSig <<")";
				cerr<<": skipped (unrecognized)."<<endl;
			}
			return;
		}

	}
	catch (const std::exception &e)
	{
		cerr<<"EXCEPTION on case "<<caseCount
			<<" test "<<testCount<<": "<<e.what()<<endl;
		actual_result = e.what();
	}
	catch (...)
	{
		cerr<<"EXEPTION"<<endl;
		actual_result = "Unknown exception thrown";
	}

	if ( success ) ++succeeded;
	else ++failed;

	if ((!success && verbose) || verbose > 1)
	{

		cout << *curr_file <<":";
		cout << " case" << caseCount << ":";
		cout << " test" << testCount << 
			": " << opName << "(" << opSig <<")";
		cout << ": " << (success?"ok.":"failed.")<<endl;

		cout << "\tDescription: " << curr_case_desc << endl;

		string geomOut;

		if ( gA ) {
			geomOut=w->write(gA);
			cout << "\tGeometry A: " << geomOut << endl;
		}

		if ( gB ) {
			geomOut=w->write(gB);
			cout << "\tGeometry B: " << geomOut << endl;
		}

		cout << "\tExpected result: "<<expected_result<<endl;
		cout << "\tObtained result: "<<actual_result<<endl;
		cout <<endl;


	}

	if (test_predicates && gB && gA) {
		runPredicates(gA, gB);
	}

	xml.OutOfElem();
}

void
XMLTester::runPredicates(const Geometry *gA, const Geometry *gB)
{
	cout << "\t    Equals:\tAB=" << (gA->equals(gB)?"T":"F") << ", BA=" << (gB->equals(gA)?"T":"F") << endl;
	cout << "\t  Disjoint:\tAB=" << (gA->disjoint(gB)?"T":"F") << ", BA=" << (gB->disjoint(gA)?"T":"F") << endl;
	cout << "\tIntersects:\tAB=" << (gA->intersects(gB)?"T":"F") << ", BA=" << (gB->intersects(gA)?"T":"F") << endl;
	cout << "\t   Touches:\tAB=" << (gA->touches(gB)?"T":"F") << ", BA=" << (gB->touches(gA)?"T":"F") << endl;
	cout << "\t   Crosses:\tAB=" << (gA->crosses(gB)?"T":"F") << ", BA=" << (gB->crosses(gA)?"T":"F") << endl;
	cout << "\t    Within:\tAB=" << (gA->within(gB)?"T":"F") << ", BA=" << (gB->within(gA)?"T":"F") << endl;
	cout << "\t  Contains:\tAB=" << (gA->contains(gB)?"T":"F") << ", BA=" << (gB->contains(gA)?"T":"F") << endl;
	cout << "\t  Overlaps:\tAB=" << (gA->overlaps(gB)?"T":"F") << ", BA=" << (gB->overlaps(gA)?"T":"F") << endl;
}

XMLTester::~XMLTester()
{
	delete pm;
	delete factory;
	delete r; r=NULL;
	delete w; w=NULL;
	delete br; br=NULL;
}


static void
usage(char *me, int exitcode, ostream &os)
{
	os<<"Usage: "<<me<<" [-v] <test> [<test> ...]"<<endl;
	os<<" Multiple -v increments verbosity"<<endl;
	exit(exitcode);
}

int
main(int argC, char* argV[])
{
	int verbose=0;

#ifdef _MSC_VER
	InitAllocCheck();
	{
#endif

	if ( argC < 2 ) usage(argV[0], 1, cerr);

	XMLTester tester;
	tester.setVerbosityLevel(verbose);

	for (int i=1; i<argC; ++i)
	{
		// increment verbosity level
		if ( ! strcmp(argV[i], "-v" ) )
		{
			++verbose;
			tester.setVerbosityLevel(verbose);
			continue;
		}

		string source = argV[i];
		tester.run(source);
	}
	tester.resultSummary(cout);

	Unload::Release();

	return tester.getFailuresCount();

#ifdef _MSC_VER
	}
	DeInitAllocCheck();
#endif

}

/**********************************************************************
 * $Log$
 * Revision 1.13  2006/03/01 18:14:07  strk
 * Handled NULL return from Geometry::getInteriorPoint()
 *
 * Revision 1.12  2006/03/01 13:06:41  strk
 * Used FLOATING precision model in buffer.xml test, added expected results,
 * changed XMLTester.cpp to use a tolerance when comparing expected and obtained
 * results from buffer operations.
 *
 * Revision 1.11  2006/03/01 12:12:03  strk
 * Fixed a bug in verbose output preventing geometry arg 'A' from being properly printed
 *
 * Revision 1.10  2006/03/01 09:56:32  strk
 * Case insensitive operation names and geometry arguments names (a/b)
 *
 * Revision 1.9  2006/03/01 09:43:44  strk
 * Unrecognized tests always printed (was only printed when verbose before)
 *
 * Revision 1.8  2006/02/28 19:18:04  strk
 * Added cctype include (bug #34)
 *
 * Revision 1.7  2006/02/28 15:34:44  strk
 * Fix for VC++ builds (Bug #32)
 *
 * Revision 1.6  2006/02/27 14:41:38  strk
 * More verbose handling of exceptions
 *
 * Revision 1.5  2006/02/23 20:32:55  strk
 * Added support for LineMerge tests. Exception printed on stderr.
 *
 * Revision 1.4  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.3  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
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
 * Revision 1.60  2006/01/18 17:49:58  strk
 * Reworked XMLTester to be quiet by default. Use -v switch to make it verbose.
 *
 * Revision 1.59  2006/01/18 12:54:48  strk
 * Added HEXWKB support in XMLTester. Added a simple test in HEXWKB form
 * and a 'test' rule running the locally-available tests and showing
 * result summay.
 *
 * Revision 1.58  2005/11/25 12:22:42  strk
 * Made XMLTester able to run multiple test files and keep overall
 * counters.
 *
 * Revision 1.57  2005/06/28 16:52:09  strk
 * Added number of points count as a debugging aid
 *
 * Revision 1.56  2005/06/21 12:22:19  strk
 * XMLTester code cleanups
 *
 * Revision 1.55  2005/06/14 11:57:03  strk
 * Added workaround for mingw Polygon name clash
 *
 * Revision 1.54  2005/06/10 13:24:50  strk
 * Added use declaration to make MingW build work
 *
 * Revision 1.53  2005/02/18 08:20:24  strk
 * Added support for point-per-quadrant argument in buffer tests (using arg2).
 *
 * Revision 1.52  2005/01/03 16:06:27  strk
 * Changed polygonize op to return a GeometryCollection
 *
 * Revision 1.51  2005/01/03 15:56:21  strk
 * Fixed memory leaks just introduced for Polygonizer test case.
 *
 * Revision 1.50  2005/01/03 15:49:30  strk
 * Added Polygonize test handling
 *
 * Revision 1.49  2004/12/30 13:32:30  strk
 * Handled NULL result from getCentroid()
 *
 * Revision 1.48  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.47  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.46  2004/11/02 09:38:33  strk
 * Added timer for buffer test.
 *
 * Revision 1.45  2004/09/13 12:39:49  strk
 * Added missing newline at end of output
 *
 * Revision 1.44  2004/07/07 09:38:12  strk
 * Dropped WKTWriter::stringOfChars (implemented by std::string).
 * Dropped WKTWriter default constructor (internally created GeometryFactory).
 * Updated XMLTester to respect the changes.
 * Main documentation page made nicer.
 *
 * Revision 1.43  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.42  2004/05/27 08:40:13  strk
 * Fixed a memleak in buffer test.
 *
 * Revision 1.41  2004/05/18 13:49:18  strk
 * Output made more neat (geometry B is not printed if not existent).
 * Added support for buffer tests.
 *
 * Revision 1.40  2004/05/17 12:53:52  strk
 * Expected result string trimmed for blanks
 *
 * Revision 1.39  2004/05/14 07:19:59  strk
 * Changed the algorythm for finding precisionModel type (current way did
 * not work): now if you specify a scale precisionModel will be FIXED,
 * otherwise it will be FLOATING.
 *
 * Revision 1.38  2004/05/07 13:23:51  strk
 * Memory leaks fixed.
 *
 * Revision 1.37  2004/03/19 09:48:46  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.36  2003/11/12 17:10:01  strk
 * added missing initialization
 *
 * Revision 1.35  2003/11/12 15:02:12  strk
 * more cleanup on exception
 *
 * Revision 1.34  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.33  2003/10/17 05:51:21  ybychkov
 * Fixed a small memory leak.
 *
 * Revision 1.32  2003/10/16 13:01:31  strk
 * Added call to Unload::Release()
 *
 * Revision 1.31  2003/10/16 12:09:48  strk
 * bug fixed in exception handling
 *
 * Revision 1.30  2003/10/16 08:48:06  strk
 * Exceptions handled
 *
 **********************************************************************/

