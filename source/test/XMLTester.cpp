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

#include <geos/util.h>
#include <geos/geomgraph.h>
#include <geos/io.h>
#include <geos/opRelate.h>
#include <geos/profiler.h>
#include "../io/markup/MarkupSTL.h"
#include <geos/unload.h>

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

#define TEST_DESCR 1
#define GEOM_A_IN 2
#define GEOM_A_OUT 4
#define GEOM_B_IN 8
#define GEOM_B_OUT 16
#define TEST_OP 32
#define TEST_RESULT 64
#define PRED 128

int main(int argC, char* argV[]) {
	PrecisionModel *pm=NULL;
	GeometryFactory *factory=NULL;
	WKTReader *r=NULL;
	WKTWriter *w=NULL;
	Geometry *gA=NULL;
	Geometry *gB=NULL;

try{

#ifdef _MSC_VER
	InitAllocCheck();
	{
#endif
//	_CrtSetBreakAlloc(837964);
	int out=TEST_DESCR+GEOM_A_IN+GEOM_A_OUT+GEOM_B_IN+GEOM_B_OUT+TEST_OP+TEST_RESULT;
//	int out=TEST_DESCR+GEOM_A_IN+GEOM_B_IN+TEST_OP+TEST_RESULT;
//	int out=GEOM_A_IN+GEOM_B_IN+TEST_OP+TEST_RESULT+PRED;
//	int out=TEST_DESCR+GEOM_A_IN+GEOM_B_IN+TEST_OP+TEST_RESULT;
//	int out=TEST_DESCR+TEST_RESULT;
//	int out=0;
	int failed=0;
	int succeeded=0;
	string source="./test.xml";
//	string source="d:\\test.xml";
	if ( argC > 1 ) source = argV[1];
	string precisionModel="";
	string desc="";
	string geomAin="";
	string geomBin="";
	string geomAout="";
	string geomBout="";
	string opName="";
	string opSig="";
	string opRes="";
	int caseCount=0;
	int testCount=0;
	//	_CrtSetBreakAlloc(18);
	CMarkupSTL xml;
	//bool a=
	xml.Load(source.c_str());

	xml.ResetPos();
	xml.FindElem("run");
	xml.FindChildElem("precisionModel");
	/* This does not seem to work... */
	//precisionModel=xml.GetChildAttrib("type");
	string scaleStr=xml.GetChildAttrib("scale");

	if ( scaleStr == "" ) {
		pm=new PrecisionModel();
		cout << "Precision Model: FLOATING" << endl;
	} else {
		char* stopstring;
		//string scaleStr=xml.GetChildAttrib("scale");
		string offsetXStr=xml.GetChildAttrib("offsetx");
		string offsetYStr=xml.GetChildAttrib("offsety");

		double scale=strtod(scaleStr.c_str(),&stopstring);
		double offsetX=strtod(offsetXStr.c_str(),&stopstring);
		double offsetY=strtod(offsetYStr.c_str(),&stopstring);
		pm=new PrecisionModel(scale,offsetX,offsetY);
		cout << "Precision Model: FIXED (scale: "<<scale<<", offsetX: "<<offsetX<<", offsetY: "<<offsetY<<")" << endl;
	}
	factory = new GeometryFactory(pm);
	r=new WKTReader(factory);
	w=new WKTWriter();
	gA=NULL;
	gB=NULL;

	while (xml.FindChildElem("case")) {
		xml.IntoElem();
		caseCount++;
		xml.FindChildElem("desc");
		desc=xml.GetChildData();
		if (out & TEST_DESCR) {
			cout << "Case #" << caseCount << endl;
			cout << "\t" << desc << endl;
		}

		xml.FindChildElem("a");
		geomAin=xml.GetChildData();
		gA=r->read(geomAin);
		geomAout=w->write(gA);
		if (out &(GEOM_A_IN | GEOM_A_OUT)) {
			cout << "\tGeometry A" << endl;
			if (out & GEOM_A_IN)
				cout << "\t\tIn:" << geomAin << endl;
			if (out & GEOM_A_OUT)
				cout << "\t\tOut:" << geomAout << endl;
		}

		if ( xml.FindChildElem("b") )
		{
			geomBin=xml.GetChildData();
			gB=r->read(geomBin);
			geomBout=w->write(gB);
			if (out &(GEOM_B_IN | GEOM_B_OUT)) {
				cout << "\tGeometry B" << endl;
				if (out & GEOM_B_IN)
					cout << "\t\tIn:" << geomBin << endl;
				if (out & GEOM_B_OUT)
					cout << "\t\tOut:" << geomBout << endl;
			}
		}

		testCount=0;
		while(xml.FindChildElem("test")) {
			testCount++;
			if (out & TEST_DESCR) {
				cout << "\tTest #" << testCount << endl;
			}
			xml.IntoElem();
			xml.FindChildElem("op");
			opName=xml.GetChildAttrib("name");
			opSig=xml.GetChildAttrib("arg3");
			opRes=xml.GetChildData();
		// trim blanks
		string::size_type pos = opRes.find_first_not_of(" \t\n\r");
		if (pos!=string::npos) opRes=opRes.substr(pos);
		pos = opRes.find_last_not_of(" \t\n\r");
		if (pos!=string::npos) opRes=opRes.substr(0, pos+1);

			if (out & TEST_OP) {
				Profile *profile = new Profile("op");
				if (opName=="relate") {
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << opRes << endl;
					IntersectionMatrix *im=gA->relate(gB);
					if (out & TEST_RESULT)
						cout << "\t\tResult: matrix='" << im->toString() << "' result=" << (im->matches(opSig)?"true":"false") <<endl;
					if (!im->matches(opSig)) {
						failed++;
					} else {
						succeeded++;
					}
					delete im;
				} else if (opName=="isValid") {
					cout << "\t\tOperation '" << opName << " should be " << opRes << endl;
					string result;
					if (gA->isValid()) {
						result="true";
					} else {
						result="false";
					}
					if (out & TEST_RESULT) {
						if (result==opRes) {
							cout << "\t\tResult: isValid='" << result << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: isValid='" << result << "' result=false"  <<endl;
							failed++;
						}
					}
				} else if (opName=="intersection") {
					Geometry *gRes=NULL;
					Geometry *gRealRes=NULL;
					gRes=r->read(opRes);
					gRes->normalize();
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << gRes->toString() << endl;
					try {
						gRealRes=gA->intersection(gB);
						gRealRes->normalize();
					} catch ( ... ) {
						delete gRealRes;
						delete gRes;
						throw;
					}
					if (out & TEST_RESULT) {
						if (gRes->compareTo(gRealRes)==0) {
							cout << "\t\tResult: intersection='" << gRealRes->toString() << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: intersection='" << gRealRes->toString() << "' result=false"  <<endl;
							failed++;
						}
					}
					delete gRes;
					delete gRealRes;
				} else if (opName=="union") {
					Geometry *gRes=r->read(opRes);
					gRes->normalize();
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << gRes->toString() << endl;
					Geometry *gRealRes=gA->Union(gB);
					gRealRes->normalize();
					if (out & TEST_RESULT) {
						if (gRes->compareTo(gRealRes)==0) {
							cout << "\t\tResult: union='" << gRealRes->toString() << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: union='" << gRealRes->toString() << "' result=false"  <<endl;
							failed++;
						}
					}
					delete gRes;
					delete gRealRes;
				} else if (opName=="difference") {
					Geometry *gRes=r->read(opRes);
					gRes->normalize();
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << gRes->toString() << endl;
					Geometry *gRealRes=gA->difference(gB);
					gRealRes->normalize();
					if (out & TEST_RESULT) {
						if (gRes->compareTo(gRealRes)==0) {
							cout << "\t\tResult: difference='" << gRealRes->toString() << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: difference='" << gRealRes->toString() << "' result=false"  <<endl;
							failed++;
						}
					}
					delete gRes;
					delete gRealRes;
				} else if (opName=="symdifference") {
					Geometry *gRes=r->read(opRes);
					gRes->normalize();
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << gRes->toString() << endl;
					Geometry *gRealRes=gA->symDifference(gB);
					gRealRes->normalize();
					if (out & TEST_RESULT) {
						if (gRes->compareTo(gRealRes)==0) {
							cout << "\t\tResult: symdifference='" << gRealRes->toString() << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: symdifference='" << gRealRes->toString() << "' result=false"  <<endl;
							failed++;
						}
					}
					delete gRes;
					delete gRealRes;
				} else if (opName=="intersects") {
					cout << "\t\tOperation '" << opName << " should be " << opRes << endl;
					string result;
					if (gA->intersects(gB)) {
						result="true";
					} else {
						result="false";
					}
					if (out & TEST_RESULT) {
						if (result==opRes) {
							cout << "\t\tResult: intersects='" << result << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: intersects='" << result << "' result=false"  <<endl;
							failed++;
						}
					}
				} else if (opName=="getboundary") {
					Geometry *gRes=r->read(opRes);
					gRes->normalize();
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << gRes->toString() << endl;
					Geometry *gRealRes=gA->getBoundary();
					gRealRes->normalize();
					if (out & TEST_RESULT) {
						if (gRes->compareTo(gRealRes)==0) {
							cout << "\t\tResult: getboundary='" << gRealRes->toString() << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: getboundary='" << gRealRes->toString() << "' result=false"  <<endl;
							failed++;
						}
					}
					delete gRes;
					delete gRealRes;
				} else if (opName=="getCentroid") {
					Geometry *gRes=r->read(opRes);
					gRes->normalize();
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << gRes->toString() << endl;
					Geometry *gRealRes=gA->getCentroid();
					if ( gRealRes ) {
						gRealRes->normalize();
					} else {
						gRealRes = factory->createGeometryCollection();
					}
					if (out & TEST_RESULT) {
						if (gRes->compareTo(gRealRes)==0) {
							cout << "\t\tResult: getCentroid='" << gRealRes->toString() << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: getCentroid='" << gRealRes->toString() << "' result=false"  <<endl;
							failed++;
						}
					}
					delete gRes;
					delete gRealRes;
				} else if (opName=="isSimple") {
					cout << "\t\tOperation '" << opName << " should be " << opRes << endl;
					string result;
					if (gA->isSimple()) {
						result="true";
					} else {
						result="false";
					}
					if (out & TEST_RESULT) {
						if (result==opRes) {
							cout << "\t\tResult: isSimple='" << result << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: isSimple='" << result << "' result=false"  <<endl;
							failed++;
						}
					}
				} else if (opName=="convexhull") {
					Geometry *gRes=r->read(opRes);
					gRes->normalize();
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << gRes->toString() << endl;
					Geometry *gRealRes=gA->convexHull();
					gRealRes->normalize();
					if (out & TEST_RESULT) {
						if (gRes->compareTo(gRealRes)==0) {
							cout << "\t\tResult: convexHull='" << gRealRes->toString() << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: convexHull='" << gRealRes->toString() << "' result=false"  <<endl;
							failed++;
						}
					}
					delete gRes;
					delete gRealRes;
				} else if (opName=="buffer") {
					Geometry *gRes=r->read(opRes);
					gRes->normalize();
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << gRes->toString() << endl;
					profile->start();
					Geometry *gRealRes=gA->buffer(atof(opSig.c_str()));
					profile->stop();
					gRealRes->normalize();
					if (out & TEST_RESULT) {
						cout << "\t\tResult: buffer='" << gRealRes->toString() <<"'"<<endl;
						cout << "Test result=";
						if (gRes->compareTo(gRealRes)==0) {
							cout << "true";
							succeeded++;
						} else {
							cout << "false";
							failed++;
						}
						cout << " Test time:" << profile->getTot()<<endl;
					}
					delete gRealRes;
					delete gRes;
				} else if (opName=="getInteriorPoint") {
					Geometry *gRes=r->read(opRes);
					gRes->normalize();
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << gRes->toString() << endl;
					Geometry *gRealRes=gA->getInteriorPoint();
					gRealRes->normalize();
					if (out & TEST_RESULT) {
						if (gRes->compareTo(gRealRes)==0) {
							cout << "\t\tResult: getInteriorPoint='" << gRealRes->toString() << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: getInteriorPoint='" << gRealRes->toString() << "' result=false"  <<endl;
							failed++;
						}
					}
					delete gRes;
					delete gRealRes;
				} else if (opName=="isWithinDistance") {
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << opRes << endl;
					string result;
					if (gA->isWithinDistance(gB,atof(opSig.c_str()))) {
						result="true";
					} else {
						result="false";
					}
					if (out & TEST_RESULT) {
						if (result==opRes) {
							cout << "\t\tResult: isWithinDistance='" << result << "' result=true"  <<endl;
							succeeded++;
						} else {
							cout << "\t\tResult: isWithinDistance='" << result << "' result=false"  <<endl;
							failed++;
						}
					}
				} else {
					cout<<"Something else\n";
					//GeometryFactory *gf=new GeometryFactory(pm,0);
					//Geometry *g1=gf->createPoint(*(new Coordinate(10,10)));
					//Geometry *g2=gf->createPoint(*(new Coordinate(20,20)));
					//delete g1;
					//cout << endl;
  				}
				delete profile;
			}
			if (out & PRED) {
				cout << "\tEquals:\t\tAB=" << (gA->equals(gB)?"T":"F") << ", BA=" << (gB->equals(gA)?"T":"F") << endl;
				cout << "\tDisjoint:\tAB=" << (gA->disjoint(gB)?"T":"F") << ", BA=" << (gB->disjoint(gA)?"T":"F") << endl;
				cout << "\tIntersects:\tAB=" << (gA->intersects(gB)?"T":"F") << ", BA=" << (gB->intersects(gA)?"T":"F") << endl;
				cout << "\tTouches:\tAB=" << (gA->touches(gB)?"T":"F") << ", BA=" << (gB->touches(gA)?"T":"F") << endl;
				cout << "\tCrosses:\tAB=" << (gA->crosses(gB)?"T":"F") << ", BA=" << (gB->crosses(gA)?"T":"F") << endl;
				cout << "\tWithin:\t\tAB=" << (gA->within(gB)?"T":"F") << ", BA=" << (gB->within(gA)?"T":"F") << endl;
				cout << "\tContains:\tAB=" << (gA->contains(gB)?"T":"F") << ", BA=" << (gB->contains(gA)?"T":"F") << endl;
				cout << "\tOverlaps:\tAB=" << (gA->overlaps(gB)?"T":"F") << ", BA=" << (gB->overlaps(gA)?"T":"F") << endl;
			}

			xml.OutOfElem();
		}
			
		xml.OutOfElem();
		delete gA; gA=NULL;
		delete gB; gB=NULL;
	}
	cout << "Failed: ";
	cout << failed << endl;
	cout << "Succeeded: ";
	cout << succeeded << endl;

//	_CrtDumpMemoryLeaks();

	cout << "End Test"<<endl;
	delete pm;
	delete factory;
	delete r; r=NULL;
	delete w; w=NULL;

#ifdef _MSC_VER
	}
	DeInitAllocCheck();
#endif

} catch (GEOSException *exc) {
	cerr<<"Exception: "<<exc->toString()<<endl;
	delete pm;
	delete factory;
	delete r;
	delete w;
	delete gA;
	delete gB;
	delete exc;
} catch (exception e) {
	cerr<<"Exception: "<<e.what()<<endl;
	delete pm;
	delete factory;
	delete r;
	delete w;
	delete gA;
	delete gB;
} catch (...) {
	cerr<<"unexpected exception: "<<endl;
	delete pm;
	delete factory;
	delete r;
	delete w;
	delete gA;
	delete gB;
}

	Unload::Release();
}

/**********************************************************************
 * $Log$
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

