//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include <string>
#include <iostream>
#include <fstream>

#include "../headers/util.h"
#include "../headers/graph.h"
#include "../headers/io.h"
#include "../headers/opRelate.h"
#include "../io/markup/MarkupSTL.h"

//#include "util.h"
//#include "graph.h"
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

#ifdef _MSC_VER
	InitAllocCheck();
	{
#endif
//	_CrtSetBreakAlloc(1157);
	int out=TEST_DESCR+GEOM_A_IN+GEOM_A_OUT+GEOM_B_IN+GEOM_B_OUT+TEST_OP+TEST_RESULT;
//	int out=TEST_DESCR+GEOM_A_IN+GEOM_B_IN+TEST_OP+TEST_RESULT;
//	int out=GEOM_A_IN+GEOM_B_IN+TEST_OP+TEST_RESULT+PRED;
//	int out=TEST_DESCR+GEOM_A_IN+GEOM_B_IN+TEST_OP+TEST_RESULT;
//	int out=TEST_DESCR+TEST_RESULT;
//	int out=0;
	int failed=0;
	int succeeded=0;
	string source="d://test.xml";
//	string source="./test.xml";
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
	PrecisionModel *pm;


//	pm = new PrecisionModel(1, 0, 0);
//	GeometryFactory *fact = new GeometryFactory(pm, 0);
//	WKTReader *wktRdr = new WKTReader(fact);
//	WKTWriter *wktWriter = new WKTWriter();
////    string wktA = "POLYGON((50 50, 200 50, 200 200, 50 200, 50 50))";
//    string wktA = "POINT(50 50)";
////    string wktB = " MULTIPOLYGON(((100 20, 180 20, 180 100, 100 100, 100 20)),((20 100, 100 100, 100 180, 20 180, 20 100)),((100 180, 180 180, 180 260, 100 260, 100 180)),((180 100, 260 100, 260 180, 180 180, 180 100)))";
//    Geometry *ax=wktRdr->read(wktA);
////    Geometry b = wktRdr.read(wktB);
//    
//	Geometry *b=ax->buffer(5);
//	cout << b->toString() << endl; 
//	_CrtSetBreakAlloc(18);
	CMarkupSTL xml;
	bool a=xml.Load(source.c_str());

	xml.ResetPos();
	xml.FindElem("run");
	xml.FindChildElem("precisionModel");
	precisionModel=xml.GetChildAttrib("type");

	if (precisionModel=="FLOATING") {
		pm=new PrecisionModel();
		cout << "Precision Model: " << precisionModel << endl;
	} else {
		char* stopstring;
		string scaleStr=xml.GetChildAttrib("scale");
		string offsetXStr=xml.GetChildAttrib("offsetx");
		string offsetYStr=xml.GetChildAttrib("offsety");

		double scale=strtod(scaleStr.c_str(),&stopstring);
		double offsetX=strtod(offsetXStr.c_str(),&stopstring);
		double offsetY=strtod(offsetYStr.c_str(),&stopstring);
		pm=new PrecisionModel(scale,offsetX,offsetY);
		cout << "Precision Model: FIXED" << endl;
	}
	WKTReader *r=new WKTReader(new GeometryFactory(pm,10));
	WKTWriter *w=new WKTWriter();
	Geometry *gA=NULL;
	Geometry *gB=NULL;

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

		xml.FindChildElem("b");
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
			if (out & TEST_OP) {
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
					Geometry *gRes=r->read(opRes);
					gRes->normalize();
					cout << "\t\tOperation '" << opName << "[" << opSig <<"]' should be " << gRes->toString() << endl;
					Geometry *gRealRes=gA->intersection(gB);
					gRealRes->normalize();
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
					gRealRes->normalize();
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
					cout << "\t\tOperation '" << opName << "(10.0)'" << endl;
					Geometry *gRealRes=gA->buffer(10.0);
					gRealRes->normalize();
					if (out & TEST_RESULT) {
						cout << "\t\tResult: buffer='" << gRealRes->toString() << "'"  <<endl;
					}
					delete gRealRes;
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
				}
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
		delete gA;
		delete gB;
	}
	cout << "Failed: ";
	cout << failed << endl;
	cout << "Succeeded: ";
	cout << succeeded << endl;

//	_CrtDumpMemoryLeaks();

	cout << "End Test";
	delete pm;
	delete r;
	delete w;

#ifdef _MSC_VER
	}
	DeInitAllocCheck();
#endif
}
