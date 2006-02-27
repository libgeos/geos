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
#include <geos/opLinemerge.h>
#include <geos/profiler.h>
#include "MarkupSTL.h"
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
//using namespace geos::operation::polygonize;
//using namespace geos::operation::linemerge;

//using geos::Polygon; // for mingw providing a Polygon global function


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
		} catch (GEOSException* exc) {
			cerr<<exc->toString()<<endl;
			delete exc;
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
			cerr<<"HEX reading not supported"<<endl; // return br->readHEX(is);
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
	catch (GEOSException* ex) {
		thrownException = ex->toString();
		delete ex;
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

		else if (opName=="isValid")
		{
			Geometry *gT=gA;
			if ( opArg1 == "B" && gB ) {
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
			if ( opArg1 == "B" && gB ) gT=gB;

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
			if ( opArg1 == "B" && gB ) gT=gB;

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
			if ( opArg1 == "B" && gB ) gT=gB;

			if (gT->isSimple()) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;

		}

		else if (opName=="convexhull")
		{
			Geometry *gT=gA;
			if ( opArg1 == "B" && gB ) gT=gB;

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
			if ( opArg1 == "B" && gB ) gT=gB;

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

		else if (opName=="Linemerge")
		{
			Geometry *gRes=NULL;
			Geometry *gRealRes=NULL;
			gRes=r->read(opRes);
			gRes->normalize();

			Geometry *gT=gA;
			if ( opArg1 == "B" && gB ) gT=gB;

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
			if ( verbose )
			{
				cout << *curr_file <<":";
				cout << " case" << caseCount << ":";
				cout << " test" << testCount <<
					": " << opName << "(" << opSig <<")";
				cout<<": skipped (unrecognized)."<<endl;
			}
			return;
		}

	}
	catch (const std::exception &e)
	{
		string msg = e.what();
		cerr<<"EXCEPTION on case "<<caseCount
			<<" test "<<testCount<<": "<<msg<<endl;
		actual_result = msg;
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

		if ( gB ) {
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

#ifdef _MSC_VER
	}
	DeInitAllocCheck();
#endif

	Unload::Release();

	return tester.getFailuresCount();
}

/**********************************************************************
 * $Log$
 * Revision 1.48.2.4.2.1  2006/02/27 17:15:59  strk
 * Back ported changes in XMLTester and new buffer.xml 'pseudo-test'
 *
 **********************************************************************/
