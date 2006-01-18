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

#include <geos/util.h>
#include <geos/geomgraph.h>
#include <geos/io.h>
#include <geos/opRelate.h>
#include <geos/opPolygonize.h>
#include <geos/profiler.h>
#include "../io/markup/MarkupSTL.h"
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
//using geos::Polygon; // for mingw providing a Polygon global function


XMLTester::XMLTester()
{
	r=NULL;
	w=NULL;
	br=NULL;
	pm=NULL;
	factory=NULL;
	verbose=failed=succeeded=0;
	caseCount=testCount=testFileCount=totalTestCount=0;
	test_predicates=0;
	setVerbosityLevel(0);
}

int
XMLTester::setVerbosityLevel(int value)
{
	int old_value=verbose;

	verbose=value;
	//out = (int)pow(2.0, value+1)-1;
	//cout<<"OUT: "<<out<<endl;
	//if ( value > 0 ) out |= TEST_DESCR;
	//if ( value > 0 ) out |= GEOM_IN;
	//if ( value > 0 ) out |= GEOM_OUT;
	//if ( value > 2 ) out |= PRED;
	//default_verbosity=TEST_DESCR|TEST_OP|SHOW_RESULT;

	return old_value;
}


void
XMLTester::run(const string &source)
{
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
		} catch (GEOSException *exc) {
			cerr<<exc->toString()<<endl;
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

	if ( scaleStr == "" ) {
		pm=new PrecisionModel();
		if (verbose>=SHOW_RUN_INFO)
		{
			cout << "Precision Model: FLOATING" << endl;
		}
	} else {
		char* stopstring;
		//string scaleStr=xml.GetChildAttrib("scale");
		string offsetXStr=xml.GetChildAttrib("offsetx");
		string offsetYStr=xml.GetChildAttrib("offsety");

		double scale=strtod(scaleStr.c_str(),&stopstring);
		double offsetX=strtod(offsetXStr.c_str(),&stopstring);
		double offsetY=strtod(offsetYStr.c_str(),&stopstring);
		pm=new PrecisionModel(scale,offsetX,offsetY);
		if (verbose>=SHOW_RUN_INFO)
		{
		cout << "Precision Model: FIXED (scale: "<<scale<<", offsetX: "<<offsetX<<", offsetY: "<<offsetY<<")" << endl;
		}
	}
	factory = new GeometryFactory(pm);
	r=new WKTReader(factory);
	w=new WKTWriter();
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
	string desc;

	gA=NULL;
	gB=NULL;

	xml.IntoElem();
	++caseCount;
	xml.FindChildElem("desc");
	desc=trimBlanks(xml.GetChildData());
	if (verbose >= SHOW_CASE) {
		cout << "Case #" << caseCount << 
			" - " << desc << endl;
	}

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

	if (verbose>=SHOW_GEOMS)
	{
		string geomOut;

		if (gA) {
			//cout << "\tGeometry A (points: " << gA->getNumPoints() <<")" << endl;
			//cout << "\t\t In:" << geomAin << endl;
			geomOut=w->write(gA);
			cout << "\tGeometry A: " << geomOut << endl;
		}

		if (gB) {
			//cout << "\tGeometry B (points: " << gB->getNumPoints() <<")" << endl;
			//cout << "\t\tIn:" << geomBin << endl;
			geomOut=w->write(gB);
			cout << "\tGeometry B: " << geomOut << endl;
		}
	}


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


	string actual_result="NONE";

	// expected_result will be modified by specific tests
	// if needed (geometry normalization, for example)
	string expected_result=opRes;

	if (1) {

		if (verbose >= SHOW_TEST)
		{

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

			cout << "\tTest #" << testCount << 
				" -  " << opName <<
				"(" << opSig <<")"<<endl;
		}

		Profile *profile = new Profile("op");

		if (opName=="relate")
		{
			IntersectionMatrix *im=gA->relate(gB);

			if (im->matches(opArg3)) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;
			
			delete im;

		}

		else if (opName=="isValid")
		{
			Geometry *gT=gA;
			string gTname="A";
			if ( opArg1 == "B" && gB ) {
				gT=gB;
				gTname="B";
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
			} catch ( ... ) {
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
			string gTname="A";
			if ( opArg1 == "B" && gB ) {
				gT=gB;
				gTname="B";
			} 
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

		else if (opName=="getCentroid")
		{
			Geometry *gT=gA;
			string gTname="A";
			if ( opArg1 == "B" && gB ) {
				gT=gB;
				gTname="B";
			} 
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

		else if (opName=="isSimple")
		{
			Geometry *gT=gA;
			string gTname="A";
			if ( opArg1 == "B" && gB ) {
				gT=gB;
				gTname="B";
			} 
			if (gT->isSimple()) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;

		}

		else if (opName=="convexhull")
		{
			Geometry *gT=gA;
			string gTname="A";
			if ( opArg1 == "B" && gB ) {
				gT=gB;
				gTname="B";
			} 
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
			string gTname="A";
			if ( opArg1 == "B" && gB ) {
				gT=gB;
				gTname="B";
			} 
			Geometry *gRes=r->read(opRes);
			gRes->normalize();
			profile->start();
			Geometry *gRealRes;
			if ( opArg2 != "" ) {
				gRealRes=gT->buffer(atof(opArg3.c_str()), atoi(opArg2.c_str()));
			} else {
				gRealRes=gT->buffer(atof(opArg3.c_str()));
			}
			profile->stop();
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRealRes;
			delete gRes;
		}

		else if (opName=="getInteriorPoint")
		{
			Geometry *gRes=r->read(opRes);
			gRes->normalize();
			Geometry *gRealRes=gA->getInteriorPoint();
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes)==0) success=1;

			actual_result=gRealRes->toString();
			expected_result=gRes->toString();

			delete gRes;
			delete gRealRes;
		}

		else if (opName=="isWithinDistance")
		{
			float dist=atof(opArg3.c_str());
			if (gA->isWithinDistance(gB, dist)) {
				actual_result="true";
			} else {
				actual_result="false";
			}

			if (actual_result==opRes) success=1;

		}

		else if (opName=="Polygonize")
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
			} catch ( ... ) {
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
			cout<<"Something else\n";
		}

		delete profile;

		if ( success ) ++succeeded;
		else ++failed;

		if (verbose)
		{
			cout << "\t\tExpected: "<<expected_result<<endl;
			if ( ! success )
				cout << "\t\tObtained: "<<actual_result<<endl;
			cout << "\t\tResult: "<<(success?"true":"false")<<endl;
		}
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

	for (int i=1; i<argC; ++i)
	{
		// increment verbosity level
		if ( ! strcmp(argV[i], "-v" ) )
		{
			verbose=32;
			tester.setVerbosityLevel(verbose);
			continue;
		}

		string source = argV[i];
		tester.run(source);
	}
	tester.resultSummary(cout);

#ifdef _MSC_VER
	}
	DeInitAllocCheck();
#endif

	Unload::Release();
}

/**********************************************************************
 * $Log$
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

